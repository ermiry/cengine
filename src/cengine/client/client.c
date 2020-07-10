#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "client/types/types.h"
#include "client/types/string.h"

#include "client/collections/dlist.h"

#include "client/network.h"
#include "client/packets.h"
#include "client/events.h"
#include "client/errors.h"
#include "client/client.h"
#include "client/handler.h"
#include "client/cerver.h"
#include "client/connection.h"
#include "client/game.h"

#include "client/threads/thread.h"

#include "client/utils/log.h"
#include "client/utils/utils.h"

int client_disconnect (Client *client);
int client_connection_end (Client *client, Connection *connection);
Connection *client_connection_get_by_socket (Client *client, i32 sock_fd);

#pragma region stats

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
            printf ("\nClient's stats:\n");
            printf ("Threshold time:            %ld\n", client->stats->threshold_time);

            printf ("N receives done:           %ld\n", client->stats->n_receives_done);

            printf ("Total bytes received:      %ld\n", client->stats->total_bytes_received);
            printf ("Total bytes sent:          %ld\n", client->stats->total_bytes_sent);

            printf ("N packets received:        %ld\n", client->stats->n_packets_received);
            printf ("N packets sent:            %ld\n", client->stats->n_packets_sent);

            printf ("\nReceived packets:\n");
            packets_per_type_print (client->stats->received_packets);

            printf ("\nSent packets:\n");
            packets_per_type_print (client->stats->sent_packets);
        }

        else {
            client_log_msg (stderr, LOG_ERROR, LOG_CLIENT, 
                "Client does not have a reference to a client stats!");
        }
    }

    else {
        client_log_msg (stderr, LOG_WARNING, LOG_CLIENT, 
            "Can't get stats of a NULL client!");
    }

}

#pragma endregion

#pragma region main

static Client *client_new (void) {

    Client *client = (Client *) malloc (sizeof (Client));
    if (client) {
        client->name = NULL;

        client->connections = NULL;

        client->running = false;

        client->registered_events = NULL;
        client->registered_errors = NULL;

        client->app_packet_handler = NULL;
        client->app_error_packet_handler = NULL;
        client->custom_packet_handler = NULL;

        client->check_packets = false;

        client->time_started = 0;
        client->uptime = 0;

        client->session_id = NULL;

        client->stats = NULL;
    }

    return client;

}

static void client_delete (Client *client) {

    if (client) {
        dlist_delete (client->connections);

        client_events_end (client);

        client_errors_end (client);

        str_delete (client->session_id);

        client_stats_delete (client->stats);

        free (client);
    }

}

// sets the client's name
void client_set_name (Client *client, const char *name) {

    if (client) {
        if (client->name) str_delete (client->name);
        client->name = name ? str_new (name) : NULL;
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

// set whether to check or not incoming packets
// check packet's header protocol id & version compatibility
// if packets do not pass the checks, won't be handled and will be inmediately destroyed
// packets size must be cheked in individual methods (handlers)
// by default, this option is turned off
void client_set_check_packets (Client *client, bool check_packets) {

    if (client) {
        client->check_packets = check_packets;
    }

}

// sets the client's session id
// returns 0 on succes, 1 on error
u8 client_set_session_id (Client *client, const char *session_id) {

    u8 retval = 1;

    if (client) {
        str_delete (client->session_id);
        client->session_id = session_id ? str_new (session_id) : NULL;

        retval = 0;
    }

    return retval;

}

// inits client with default values
static u8 client_init (Client *client) {

    u8 retval = 1;

    if (client) {
        client->connections = dlist_init (connection_delete, connection_comparator_by_sock_fd);

        client_events_init (client);
        client_errors_init (client);

        client->stats = client_stats_new ();

        client->running = false;

        retval = 0;
    }

    return retval;

}

Client *client_create (void) {

    Client *client = client_new ();
    if (client) {
        client->name = str_new ("no-name");
        client_init (client);
    } 

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

        else {
            // client is already running because of an active connection
            retval = 0;
        }
    }

    return retval;

}

// stop any on going process and destroys the client
u8 client_teardown (Client *client) {

    u8 retval = 1;

    if (client) {
        client_disconnect (client);

        client_delete (client);

        retval = 0;
    }

    return retval;

}

#pragma endregion

#pragma region connections

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
            if (connection->socket->sock_fd == sock_fd) {
                retval = connection;
                break;
            }
        }
    }

    return retval;

}

// creates a new connection and registers it to the specified client
// the connection should be ready to be started
// returns a new connection on success, NULL on error
Connection *client_connection_create (Client *client,
    const char *ip_address, u16 port, Protocol protocol, bool use_ipv6) {

    Connection *connection = NULL;

    if (client) {
        if (ip_address) {
            connection = connection_create (ip_address, port, protocol, use_ipv6);
            if (connection) {
                dlist_insert_after (client->connections, dlist_end (client->connections), connection);
            }

            else client_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to create new connection!");
        }

        else {
            client_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, 
                "Failed to create new connection, no ip provided!");
        }
    }

    return connection;

}

// registers an existing connection to a client
// retuns 0 on success, 1 on error
int client_connection_register (Client *client, Connection *connection) {

    int retval = 1;

    if (client && connection) {
        retval =  dlist_insert_after (
            client->connections, 
            dlist_end (client->connections), 
            connection
        );
    }

    return retval;

}

// unregister an exitsing connection from the client
// returns 0 on success, 1 on error or if the connection does not belong to the client
int client_connection_unregister (Client *client, Connection *connection) {

    int retval = 1;
    if (client && connection) {
        if (dlist_remove (client->connections, connection, NULL)) {
            retval = 0;
        }
    }

    return retval;

}

#pragma endregion

#pragma region connect

// connects a client to the host with the specified values in the connection
// it can be a cerver or not
// this is a blocking method, as it will wait until the connection has been successfull or a timeout
// user must manually handle how he wants to receive / handle incomming packets and also send requests
// returns 0 when the connection has been established, 1 on error or failed to connect
unsigned int client_connect (Client *client, Connection *connection) {

    unsigned int retval = 1;

    if (client && connection) {
        if (!connection_start (connection)) {
            client_event_trigger (client, connection, CLIENT_EVENT_CONNECTED);
            connection->connected = true;
            time (&connection->connected_timestamp);
            
            client_start (client);

            retval = 0;     // success - connected to cerver
        }
    }

    return retval;

}

// connects a client to the host with the specified values in the connection
// performs a first read to get the cerver info packet 
// this is a blocking method, and works exactly the same as if only calling client_connect ()
// returns 0 when the connection has been established, 1 on error or failed to connect
unsigned int client_connect_to_cerver (Client *client, Connection *connection) {

    unsigned int retval = 1;

    if (!client_connect (client, connection)) {
        client_receive (client, connection);

        retval = 0;
    }

    return retval;

}

static void *client_connect_thread (void *client_connection_ptr) {

    if (client_connection_ptr) {
        ClientConnection *cc = (ClientConnection *) client_connection_ptr;

        if (!connection_start (cc->connection)) {
            client_event_trigger (cc->client, cc->connection, CLIENT_EVENT_CONNECTED);
            cc->connection->connected = true;
            time (&cc->connection->connected_timestamp);
            
            client_start (cc->client);
        }

        client_connection_aux_delete (cc);
    }

    return NULL;

}

// connects a client to the host with the specified values in the connection
// it can be a cerver or not
// this is NOT a blocking method, a new thread will be created to wait for a connection to be established
// open a success connection, EVENT_CONNECTED will be triggered, otherwise, EVENT_CONNECTION_FAILED will be triggered
// user must manually handle how he wants to receive / handle incomming packets and also send requests
// returns 0 on success connection thread creation, 1 on error
unsigned int client_connect_async (Client *client, Connection *connection) {

    unsigned int retval = 1;

    if (client && connection) {
        ClientConnection *cc = client_connection_aux_new (client, connection);
        if (cc) {
            pthread_t thread_id = 0;
            if (!thread_create_detachable (&thread_id, client_connect_thread, cc)) {
                retval = 0;         // success
            }

            else {
                #ifdef CLIENT_DEBUG
                client_log_error ("Failed to create client_connect_thread () detachable thread!");
                #endif
            }
        }
    }

    return retval;

}

#pragma endregion

#pragma region requests

// when a client is already connected to the cerver, a request can be made to the cerver
// and the result will be returned
// this is a blocking method, as it will wait until a complete cerver response has been received
// the response will be handled using the client's packet handler
// this method only works if your response consists only of one packet
// neither client nor the connection will be stopped after the request has ended, the request packet won't be deleted
// retruns 0 when the response has been handled, 1 on error
unsigned int client_request_to_cerver (Client *client, Connection *connection, Packet *request) {

    unsigned int retval = 1;

    if (client && connection && request) {
        // send the request to the cerver
        packet_set_network_values (request, client, connection);

        size_t sent = 0;
        if (!packet_send (request, 0, &sent, false)) {
            // printf ("Request to cerver: %ld\n", sent);

            // receive the data directly
            connection->full_packet = false;
            while (!connection->full_packet) {
                client_receive (client, connection);
            }

            retval = 0;
        }

        else {
            #ifdef CLIENT_DEBUG
            client_log_error ("client_request_to_cerver () - failed to send request packet!");
            #endif
        }
    }

    return retval;

}

static void *client_request_to_cerver_thread (void *cc_ptr) {

    if (cc_ptr) {
        ClientConnection *cc = (ClientConnection *) cc_ptr;

        cc->connection->full_packet = false;
        while (!cc->connection->full_packet) {
            client_receive (cc->client, cc->connection);
        }

        client_connection_aux_delete (cc);
    }

    return NULL;

}

// when a client is already connected to the cerver, a request can be made to the cerver
// the result will be placed inside the connection
// this method will NOT block and the response will be handled using the client's packet handler
// this method only works if your response consists only of one packet
// neither client nor the connection will be stopped after the request has ended, the request packet won't be deleted
// returns 0 on success request, 1 on error
unsigned int client_request_to_cerver_async (Client *client, Connection *connection, Packet *request) {

    unsigned int retval = 1;

    if (client && connection && request) {
        // send the request to the cerver
        packet_set_network_values (request, client, connection);
        if (!packet_send (request, 0, NULL, false)) {
            ClientConnection *cc = client_connection_aux_new (client, connection);
            if (cc) {
                // create a new thread to receive & handle the response
                pthread_t thread_id = 0;
                if (!thread_create_detachable (&thread_id, client_request_to_cerver_thread, cc)) {
                    retval = 0;         // success
                }

                else {
                    #ifdef CLIENT_DEBUG
                    client_log_error ("Failed to create client_request_to_cerver_thread () detachable thread!");
                    #endif
                }
            }
        }

        else {
            #ifdef CLIENT_DEBUG
            client_log_error ("client_request_to_cerver_async () - failed to send request packet!");
            #endif
        }
    }

    return retval;

}

#pragma endregion

#pragma region start

// after a client connection successfully connects to a server, 
// it will start the connection's update thread to enable the connection to
// receive & handle packets in a dedicated thread
// returns 0 on success, 1 on error
int client_connection_start (Client *client, Connection *connection) {

    int retval = 1;

    if (client && connection) {
        if (connection->connected) {
            if (!client_start (client)) {
                pthread_t thread_id = 0;
                if (!thread_create_detachable (
                    &thread_id,
                    (void *(*)(void *)) connection_update,
                    client_connection_aux_new (client, connection)
                )) {
                    retval = 0;         // success
                }

                else {
                    char *s = c_string_create ("client_connection_start () - Failed to create update thread for client %s", 
                        client->name->str);
                    if (s) {
                        client_log_error (s);
                        free (s);
                    }
                }
            }

            else {
                char *s = c_string_create ("client_connection_start () - Failed to start client %s", 
                    client->name->str);
                if (s) {
                    client_log_error (s);
                    free (s);
                }
            }
        }
    }

    return retval;

}

// connects a client connection to a server
// and after a success connection, it will start the connection (create update thread for receiving messages)
// this is a blocking method, returns only after a success or failed connection
// returns 0 on success, 1 on error
int client_connect_and_start (Client *client, Connection *connection) {

    int retval = 1;

    if (client && connection) {
        if (!client_connect (client, connection)) {
            if (!client_connection_start (client, connection)) {
                retval = 0;
            }
        }

        else {
            char *s = c_string_create ("client_connect_and_start () - Client %s failed to connect", 
                client->name->str);
            if (s) {
                client_log_error (s);
                free (s);
            }
        }
    }

    return retval;

}

static void client_connection_start_wrapper (void *data_ptr) {

    if (data_ptr) {
        ClientConnection *cc = (ClientConnection *) data_ptr;
        client_connect_and_start (cc->client, cc->connection);
        client_connection_aux_delete (cc);
    }

}

// connects a client connection to a server in a new thread to avoid blocking the calling thread,
// and after a success connection, it will start the connection (create update thread for receiving messages)
// returns 0 on success creating connection thread, 1 on error
u8 client_connect_and_start_async (Client *client, Connection *connection) {

    pthread_t thread_id = 0;
    return (client && connection) ? thread_create_detachable (
        &thread_id,
        (void *(*)(void *)) client_connection_start_wrapper,
        client_connection_aux_new (client, connection)
    ) : 1;

}

#pragma endregion

#pragma region end

// ends a connection with a cerver by sending a disconnect packet and the closing the connection
static void client_connection_terminate (Client *client, Connection *connection) {

    if (connection) {
        if (connection->connected) {
            if (connection->cerver) {
                // send a close connection packet
                Packet *packet = packet_generate_request (REQUEST_PACKET, CLIENT_CLOSE_CONNECTION, NULL, 0);
                if (packet) {
                    packet_set_network_values (packet, client, connection);
                    if (packet_send (packet, 0, NULL, false)) {
                        client_log_error ("Failed to send CLIENT_CLOSE_CONNECTION!");
                    }
                    packet_delete (packet);
                }
            }

            connection_close (connection);
        } 
    }

}

// terminates and destroy a connection registered to a client
// returns 0 on success, 1 on error
int client_connection_end (Client *client, Connection *connection) {

    int retval = 1;

    if (client && connection) {
        if (connection->connected) {
            client_connection_terminate (client, connection);
            client_event_trigger (client, connection, CLIENT_EVENT_CONNECTION_CLOSE);

            retval = 0;
        }
    }

    return retval;

}

// terminates all of the client connections and deletes them
// return 0 on success, 1 on error
int client_disconnect (Client *client) {

    int retval = 1;

    if (client) {
        // end any ongoing connection
        for (ListElement *le = dlist_start (client->connections); le; le = le->next) {
            client_connection_terminate (client, (Connection *) le->data);
        }

        dlist_reset (client->connections);

        // reset client
        client->running = false;
        client->time_started = 0;

        retval = 0;
    }

    return retval;

}

// the client got disconnected from the cerver, so correctly clear our data
void client_got_disconnected (Client *client) {

    if (client) {
        // close any ongoing connection
        for (ListElement *le = dlist_start (client->connections); le; le = le->next) {
            connection_close ((Connection *) le->data);
        }

        // dlist_reset (client->connections);

        // reset client
        client->running = false;
        client->time_started = 0;
    }

}

#pragma endregion

#pragma region files

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

#pragma region game

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
            retval = packet_send (packet, 0, NULL, false);
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
            retval = packet_send (packet, 0, NULL, false);
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
            retval = packet_send (packet, 0, NULL, false);
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
            retval = packet_send (packet, 0, NULL, false);
            packet_delete (packet);
        }
    }

    return retval;

}

#pragma endregion

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