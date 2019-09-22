#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/network.h"
#include "cengine/cerver/packets.h"
#include "cengine/cerver/events.h"
#include "cengine/cerver/errors.h"
#include "cengine/cerver/client.h"
#include "cengine/cerver/handler.h"
#include "cengine/cerver/cerver.h"
#include "cengine/cerver/connection.h"
#include "cengine/cerver/game.h"

#include "cengine/collections/dlist.h"

#include "cengine/threads/thread.h"

#include "cengine/utils/log.h"
#include "cengine/utils/utils.h"

int client_connection_end (Client *client, Connection *connection);
Connection *client_connection_get_by_socket (Client *client, i32 sock_fd);

static ClientStats *client_stats_new (void) {

    ClientStats *client_stats = (ClientStats *) malloc (sizeof (ClientStats));
    if (client_stats) {
        memset (client_stats, 0, sizeof (ClientStats));
        client_stats->received_packets = packets_per_type_new ();
        client_stats->sent_packets = packets_per_type_new ();
    } 

    return client_stats;

}

static inline void client_stats_delete (ClientStats *client_stats) { 
    
    if (client_stats) {
        packets_per_type_delete (client_stats->received_packets);
        packets_per_type_delete (client_stats->sent_packets);

        free (client_stats); 
    } 
    
}

void client_stats_print (Client *client) {

    if (client) {
        if (client->stats) {
            printf ("\nClient's stats: ");
            printf ("\nThreshold time:            %ld\n", client->stats->threshold_time);
            printf ("N packets received:        %ld\n", client->stats->n_packets_received);
            printf ("N receives done:           %ld\n", client->stats->n_receives_done);
            printf ("Total bytes received:      %ld\n", client->stats->total_bytes_received);
            printf ("N packets sent:            %ld\n", client->stats->n_packets_sent);
            printf ("Total bytes sent:          %ld\n", client->stats->total_bytes_sent);

            printf ("\nReceived packets:\n");
            packets_per_type_print (client->stats->received_packets);

            printf ("\nSent packets:\n");
            packets_per_type_print (client->stats->sent_packets);
        }

        else {
            cengine_log_msg (stderr, LOG_ERROR, LOG_CLIENT, 
                "Client does not have a reference to a client stats!");
        }
    }

    else {
        cengine_log_msg (stderr, LOG_WARNING, LOG_CLIENT, 
            "Can't get stats of a NULL client!");
    }

}

static Client *client_new (void) {

    Client *client = (Client *) malloc (sizeof (Client));
    if (client) {
        memset (client, 0, sizeof (Client));

        client->running = false;
        client->connections = NULL;
        client->registered_actions = NULL;

        client->app_packet_handler = NULL;
        client->app_error_packet_handler = NULL;
        client->custom_packet_handler = NULL;

        client->stats = NULL;
    }

    return client;

}

static void client_delete (Client *client) {

    if (client) {
        dlist_delete (client->connections);

        client_events_end (client);

        client_stats_delete (client->stats);

        free (client);
    }

}

// sets a cutom app packet hanlder and a custom app error packet handler
void client_set_app_handlers (Client *client, Action app_handler, Action app_error_handler) {

    if (client) {
        client->app_packet_handler = app_handler;
        client->app_error_packet_handler = app_error_handler;
    }

}

// sets a custom packet handler
void client_set_custom_handler (Client *client, Action custom_handler) {

    if (client) client->custom_packet_handler = custom_handler;

}

// inits client with default values
static u8 client_init (Client *client) {

    u8 retval = 1;

    if (client) {
        client->connections = dlist_init (connection_delete, NULL);
        client_events_init (client);
        client->stats = client_stats_new ();

        client->running = false;
    }

    return retval;

}

Client *client_create (void) {

    Client *client = client_new ();
    if (client) client_init (client);

    return client;

}

// start the client thpool and adds client_poll () to it
static u8 client_start (Client *client) {

    u8 retval = 1;

    if (client) {
        // check if we walready have the client poll running
        if (!client->running) {
            time (&client->time_started);
            client->running = true;
            retval = 0;
        }
    }

    return retval;

}

// stop any on going process and destroys the client
u8 client_teardown (Client *client) {

    u8 retval = 1;

    if (client) {
        // end any ongoing connection
        for (ListElement *le = dlist_start (client->connections); le; le = le->next) {
            connection_end (client, (Connection *) le->data);
            le->data = NULL;
        }

        client->running = false;

        client_delete (client);

        retval = 0;
    }

    return retval;

}

// returns a connection (registered to a client) by its name
Connection *client_connection_get_by_name (Client *client, const char *name) {

    Connection *retval = NULL;

    if (client && name) {
        Connection *connection = NULL;
        for (ListElement *le = dlist_start (client->connections); le; le = le->next) {
            connection = (Connection *) le->data;
            if (!strcmp (connection->name->str, name)) {
                retval = connection;
                break;
            }
        }
    }

    return retval;

}

// returns a connection assocaited with a socket
Connection *client_connection_get_by_socket (Client *client, i32 sock_fd) {

    Connection *retval = NULL;

    if (client) {
        Connection *connection = NULL;
        for (ListElement *le = dlist_start (client->connections); le; le = le->next) {
            connection = (Connection *) le->data;
            if (connection->sock_fd == sock_fd) {
                retval = connection;
                break;
            }
        }
    }

    return retval;

}

// creates a new connection and registers it to the specified client;
// the connection should be ready to be started
// returns 0 on success, 1 on error
int client_connection_create (Client *client, const char *name,
    const char *ip_address, u16 port, Protocol protocol, bool use_ipv6) {

    int retval = 1;

    if (client) {
        if (ip_address) {
            Connection *connection = connection_create (name, ip_address, port, protocol, use_ipv6);
            if (connection) {
                dlist_insert_after (client->connections, dlist_end (client->connections), connection);
                retval = 0;
            }

            else cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to create new connection!");
        }

        else 
            cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to create new connection, no ip provided!");
    }

    return retval;

}

// registers an existing connection to a client
// retuns 0 on success, 1 on error
int client_connection_register (Client *client, Connection *connection) {

    int retval = 1;

    if (client && connection) {
        dlist_insert_after (client->connections, dlist_end (client->connections), connection);
        retval = 0;
    }

    return retval;

}

// starts a client connection
// returns 0 on success, 1 on error
int client_connection_start (Client *client, Connection *connection) {

    int retval = 1;

    if (client && connection) {
        if (!connection_start (connection)) {
            client_event_trigger (client, EVENT_CONNECTED);
            connection->connected = true;
            thread_create_detachable ((void *(*)(void *)) client_connection_update, 
                client_connection_aux_new (client, connection));
            client_start (client);
            retval = 0;
        }

        else {
            client_event_trigger (client, EVENT_CONNECTION_FAILED);
        } 
    }

    return retval;

}

// terminates and destroy a connection registered to a client
// returns 0 on success, 1 on error
int client_connection_end (Client *client, Connection *connection) {

    int retval = 1;

    if (client && connection) {
        connection_end (client, connection);

        connection_delete (dlist_remove_element (client->connections, 
            dlist_get_element (client->connections, connection)));

        retval = 0;
    }

    return retval;

}

/*** Files ***/

#pragma region Files

// requests a file from the server
// filename: the name of the file to request
// file complete event will be sent when the file is finished
// appropiate error is set on bad filename or error in file transmission
// returns 0 on success sending request, 1 on failed to send request
u8 client_file_get (Client *client, Connection *connection, const char *filename) {

    u8 retval = 1;

    if (client && connection) {
        // request the file from the cerver
        // set our file tray for incoming files
    }

    return retval;

}

// sends a file to the server
// filename: the name of the file the cerver will receive
// file is opened using the filename
// when file is completly sent, event is set appropriately
// appropiate error is sent on cerver error or on bad file transmission
// returns 0 on success sending request, 1 on failed to send request
u8 client_file_send (Client *client, Connection *connection, const char *filename) {

    u8 retval = 1;

    if (client && connection) {
        // request the cerver for a file transmission
        // open the file
        // send file header
        // send file in packets
    }

    return retval;

}

#pragma endregion

/*** Game ***/

#pragma region Game

// requets the cerver to create a new lobby
// game type: is the type of game to create the lobby, the configuration must exist in the cerver
// returns 0 on success sending request, 1 on failed to send request
u8 client_game_create_lobby (Client *owner, Connection *connection,
    const char *game_type) {

    u8 retval = 1;

    if (owner && connection && game_type) {
        String *type = str_new (game_type);
        void *stype = str_serialize (type, SS_SMALL);

        Packet *packet = packet_generate_request (GAME_PACKET, GAME_LOBBY_CREATE, 
            stype, sizeof (SStringS));
        if (packet) {
            packet_set_network_values (packet, owner, connection);
            retval = packet_send (packet, 0, NULL);
            packet_delete (packet);
        }

        str_delete (type);
        free (stype);
    }

    return retval;

}

// requests the cerver to join a lobby
// game type: is the type of game to create the lobby, the configuration must exist in the cerver
// lobby id: if you know the id of the lobby to join to, if not, the cerver witll search one for you
// returns 0 on success sending request, 1 on failed to send request
u8 client_game_join_lobby (Client *client, Connection *connection,
    const char *game_type, const char *lobby_id) {

    u8 retval = 1;

    if (client && connection) {
        LobbyJoin lobby_join = { 0 };
        if (game_type) {
            lobby_join.game_type.len = strlen (game_type);
            strcpy (lobby_join.game_type.string, game_type);
        }

        if (lobby_id) {
            lobby_join.lobby_id.len = strlen (lobby_id);
            strcpy (lobby_join.lobby_id.string, lobby_id);
        }

        Packet *packet = packet_generate_request (GAME_PACKET, GAME_LOBBY_JOIN,
            &lobby_join, sizeof (LobbyJoin));
        if (packet) {
            packet_set_network_values (packet, client, connection);
            retval = packet_send (packet, 0, NULL);
            packet_delete (packet);
        }
    }

    return retval;

}

// request the cerver to leave the current lobby
// returns 0 on success sending request, 1 on failed to send request
u8 client_game_leave_lobby (Client *client, Connection *connection,
    const char *lobby_id) {

    u8 retval = 1;

    if (client && connection && lobby_id) {
        SStringS id = { 0 };
        id.len = strlen (lobby_id);
        strcpy (id.string, lobby_id);

        Packet *packet = packet_generate_request (GAME_PACKET, GAME_LOBBY_LEAVE, 
            &id, sizeof (SStringS));
        if (packet) {
            packet_set_network_values (packet, client, connection);
            retval = packet_send (packet, 0, NULL);
            packet_delete (packet);
        }
    }

    return retval;

}

// requests the cerver to start the game in the current lobby
// returns 0 on success sending request, 1 on failed to send request
u8 client_game_start_lobby (Client *client, Connection *connection,
    const char *lobby_id) {

    u8 retval = 1;

    if (client && connection && lobby_id) {
        SStringS id = { 0 };
        id.len = strlen (lobby_id);
        strcpy (id.string, lobby_id);

        Packet *packet = packet_generate_request (GAME_PACKET, GAME_START,
            &id, sizeof (SStringS));
        if (packet) {
            packet_set_network_values (packet, client, connection);
            retval = packet_send (packet, 0, NULL);
            packet_delete (packet);
        }
    }

    return retval;

}

/*** aux ***/

ClientConnection *client_connection_aux_new (Client *client, Connection *connection) {

    ClientConnection *cc = (ClientConnection *) malloc (sizeof (ClientConnection));
    if (cc) {
        cc->client = client;
        cc->connection = connection;
    }

    return cc;

}

void client_connection_aux_delete (void *ptr) { if (ptr) free (ptr); }