#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

#include "cengine/cerver/network.h"
#include "cengine/cerver/packets.h"
#include "cengine/cerver/events.h"
#include "cengine/cerver/errors.h"
#include "cengine/cerver/client.h"
#include "cengine/cerver/cerver.h"
#include "cengine/cerver/connection.h"

#include "cengine/collections/dlist.h"

#include "cengine/utils/log.h"
#include "cengine/utils/utils.h"

static Client *client_new (void) {

    Client *client = (Client *) malloc (sizeof (Client));
    if (client) {
        memset (client, 0, sizeof (Client));

        client->connections = NULL;

        memset (client->fds, 0, sizeof (client->fds));
        for (u8 i = 0; i < DEFAULT_MAX_CONNECTIONS; i++)    
            client->fds[i].fd = -1;

        client->thpool = NULL;

        client->registered_actions = NULL;
    }

    return client;

}

static client_delete (Client *client) {

    if (client) {
        dlist_destroy (client->connections);
        // TODO: check with new thpool code --- 14/06/2019
        if (client->thpool) thpool_destroy (client->thpool);

        client_events_end (client);

        free (client);
    }

}

Client *client_create (void) {

    Client *client = client_new ();
    if (client) {
        client->connections = dlist_init (connection_delete, NULL);
        client->poll_timeout = DEFAULT_POLL_TIMEOUT;
        client->running = false;
        client->in_lobby = false;
        client->owner = false;
        client_events_init (client->registered_actions);
        client->thpool = thpool_init (DEFAULT_THPOOL_INIT);
        if (!client->thpool) {
            #ifdef CLIENT_DEBUG
            cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to init client thpool!");
            #endif
        }
    }

    return client;

}

// stop any on going process and destroys the client
u8 client_teardown (Client *client) {

    u8 retval = 1;

    if (client) {
        client->running = false;

        // end any ongoing connection
        for (ListElement *le = dlist_start (client->connections); le; le = le->next) {
            connection_end ((Connection *) le->data);
            le->data = NULL;
        }

        if (client->thpool) {
            #ifdef CLIENT_DEBUG
                cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE,
                    createString ("Active threads in thpool: %i", 
                    thpool_num_threads_working (client->thpool)));
            #endif

            thpool_destroy (client->thpool);
            #ifdef CLIENT_DEBUG
                cengine_log_msg (stdout, SUCCESS, NO_TYPE, "Client thpool got destroyed!");
            #endif

            client->thpool = NULL;
        } 

        client_delete (client);

        retval = 0;
    }

    return retval;

}

// FIXME: create a way to register to different events such as success auth, to avoid creating
// an specific placeholder for every action we might wanna need to trigger when something happens
static void client_auth_packet_handler (Packet *packet) {

    if (packet) {
        if (packet->packet_size >= (sizeof (PacketHeader) + sizeof (RequestData))) {
            char *end = packet->packet;
            RequestData *req = (RequestData *) (end += sizeof (PacketHeader));

            switch (req->type) {
                // we recieve a token from the cerver to use in sessions
                case CLIENT_AUTH_DATA:
                    break;

                // we have successfully authenticated with the server
                case SUCCESS_AUTH:
                    break;

                default: 
                    cengine_log_msg (stderr, WARNING, NO_TYPE, "Unknown authentication request.");
                    break;
            }
        }
    }

}

// FIXME: 
void auth_packet_handler (Packet *packet) {

    // char *end = pack_info->packetData;
    // RequestData *reqdata = (RequestData *) (end += sizeof (PacketHeader));

    // switch (reqdata->type) {
    //     case CLIENT_AUTH_DATA: {
    //         Token *tokenData = (Token *) (end += sizeof (RequestData));
    //         #ifdef CLIENT_DEBUG 
    //         logMsg (stdout, DEBUG_MSG, CLIENT,
    //             createString ("Token recieved from server: %s", tokenData->token));
    //         #endif  
    //         Token *token_data = (Token *) malloc (sizeof (Token));
    //         memcpy (token_data->token, tokenData->token, sizeof (token_data->token));
    //         pack_info->connection->server->token_data = token_data;
    //     } break;
    //     case SUCCESS_AUTH: {
    //         logMsg (stdout, SUCCESS, CLIENT, "Client authenticated successfully to server!");
    //         if (pack_info->connection->successAuthAction)
    //             pack_info->connection->successAuthAction (pack_info->connection->successAuthArgs);  
    //     } break;
    //     default: break;
    // }
    // break;

}

// the client handles a packet based on its type
static void client_packet_handler (void *data) {

    if (data) {
        Packet *packet = (Packet *) data;

        if (!packet_check (packet)) {
            switch (packet->header->packet_type) {
                // handles cerver type packets
                case SERVER_PACKET: cerver_packet_handler (packet); break;

                // handles an error from the server
                case ERROR_PACKET: error_packet_handler (packet); break;

                // handles authentication packets
                case AUTH_PACKET: auth_packet_handler (packet); break;

                // handles a request made from the server
                case REQUEST_PACKET: break;

                // handle a game packet sent from the server
                case GAME_PACKET: break;

                // FIXME:
                // user set handler to handle app specific errors
                case APP_ERROR_PACKET: 
                    // if (pack_info->client->appErrorHandler)  
                    //     pack_info->client->appErrorHandler (pack_info);
                    break;

                // FIXME:
                // user set handler to handler app specific packets
                case APP_PACKET:
                    // if (pack_info->client->appPacketHandler)
                    //     pack_info->client->appPacketHandler (pack_info);
                    break;

                // custom packet hanlder
                case CUSTOM_PACKET: break;

                // FIXME:
                // handles test packets
                case TEST_PACKET: break;

                default:
                    #ifdef CLIENT_DEBUG
                    cengine_log_msg (stdout, WARNING, NO_TYPE, "Got a packet of unknown type.");
                    #endif
                    break;
            }
        }

        packet_delete (packet);
    }

}

// splits the entry buffer in packets of the correct size
static void client_handle_recieve_buffer (Client *client, i32 socket_fd, 
    char *buffer, size_t total_size) {

    if (buffer && (total_size > 0)) {
        u32 buffer_idx = 0;
        char *end = buffer;

        PacketHeader *header = NULL;
        u32 packet_size;
        char *packet_data = NULL;

        while (buffer_idx < total_size) {
            header = (PacketHeader *) end;

            // check the packet size
            packet_size = header->packet_size;
            if (packet_size > 0) {
                // copy the content of the packet from the buffer
                packet_data = (char *) calloc (packet_size, sizeof (char));
                for (u32 i = 0; i < packet_size; i++, buffer_idx++) 
                    packet_data[i] = buffer[buffer_idx];

                Packet *packet = packet_new ();
                if (packet) {
                    packet->connection = client_connection_get_by_socket (client, socket_fd);
                    packet->header = header;
                    packet->packet_size = packet_size;
                    packet->packet = packet_data;

                    thpool_add_work (client->thpool, (void *) client_packet_handler, packet);
                }

                end += packet_size;
            }

            else break;
        }
    }

}

// FIXME: correctly end server connection
// TODO: add support for handling large files transmissions
static void client_recieve (Client *client, i32 fd) {

    ssize_t rc;
    char packet_buffer[MAX_UDP_PACKET_SIZE];
    memset (packet_buffer, 0, MAX_UDP_PACKET_SIZE);

    // do {
        rc = recv (fd, packet_buffer, sizeof (packet_buffer), 0);
        
        if (rc < 0) {
            if (errno != EWOULDBLOCK) {     // no more data to read 
                #ifdef CLIENT_DEBUG 
                cengine_log_msg (stderr, ERROR, NO_TYPE, "Client recv failed!");
                perror ("Error ");
                #endif
            }

            // /break;
        }

        if (rc == 0) {
            // man recv -> steam socket perfomed an orderly shutdown
            // but in dgram it might mean something?
            #ifdef CLIENT_DEBUG
            cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "client_recieve () - rc == 0");
            #endif
            // break;
        }

        char *buffer_data = (char *) calloc (MAX_UDP_PACKET_SIZE, sizeof (char));
        if (buffer_data) {
            memcpy (buffer_data, packet_buffer, rc);
            client_handle_recieve_buffer (client, fd, buffer_data, rc);
        }
        
    // } while (true);

}

void client_set_poll_timeout (Client *client, unsigned int timeout) { 
    
    if (client) client->poll_timeout = timeout; 
    
}

static u8 client_get_free_poll_idx (Client *client) {

    for (u8 i = 0; i < DEFAULT_MAX_CONNECTIONS; i++)
        if (client->fds[i].fd == -1)
            return i;

    return -1;

}

static u8 client_poll (void *data) {

    if (!data) {
        #ifdef CLIENT_DEBUG
        cengine_log_msg (stderr, ERROR, NO_TYPE, "Can't poll on a NULL client!");
        #endif
        return 1;
    }

    Client *client = (Client *) data;

    int poll_retval;    

    #ifdef CLIENT_DEBUG
        logMsg (stdout, DEBUG_MSG, NO_TYPE, "Client poll has started!");
    #endif

    while (client->running) {
        poll_retval = poll (client->fds, client->n_fds, client->poll_timeout);

        // poll failed
        if (poll_retval < 0) {
            #ifdef CLIENT_DEBUG
            logMsg (stderr, ERROR, NO_TYPE, "Client poll failed!");
            perror ("Error");
            #endif
            // FIXME: close all of our active connections...
            break;
        }

        // if poll has timed out, just continue to the next loop... 
        if (poll_retval == 0) {
            // #ifdef CLIENT_DEBUG
            //     cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Poll timeout.");
            // #endif
            continue;
        }

        // one or more fd(s) are readable, need to determine which ones they are
        for (u8 i = 0; i < client->n_fds; i++) {
            if (client->fds[i].revents == 0) continue;
            if (client->fds[i].revents != POLLIN) continue;

            client_recieve (client, client->fds[i].fd);
        }
    }

    #ifdef CLIENT_DEBUG
        cengine_log_msg (stdout, DEBUG_MSG, CLIENT, "Client poll has ended!");
    #endif

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
static Connection *client_connection_get_by_socket (Client *client, i32 sock_fd) {

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
    const char *ip_address, u16 port, u8 protocol, bool use_ipv6, bool async) {

    int retval = 1;

    if (client) {
        if (ip_address) {
            Connection *connection = connection_create (name, ip_address, port, protocol, use_ipv6, async);
            if (connection) {
                dlist_insert_after (client->connections, dlist_end (client->connections), connection);
                retval = 0;
            }

            else cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to create new connection!");
        }

        else 
            cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to create new connection, no ip provided!");
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

// FIXME: end connections on errors!!
// starts a client connection
// returns 0 on success, 1 on error
int client_connection_start (Client *client, Connection *connection) {

    int retval = 1;

    if (client && connection) {
        if (!connection_start (connection)) {
            if (connection->async) {
                // add the new socket to the poll structure
                u8 idx = client_get_free_poll_idx (client);

                if (idx >= 0) {
                    client->fds[idx].fd = connection->sock_fd;
                    client->fds[idx].events = POLLIN;
                    client->n_fds++;

                    // check if we walready have the client poll running
                    if (!client->running) {
                        if (!thpool_add_work (client->thpool, (void *) client_poll, client)) {
                            client->running = true;
                            retval = 0;
                        }

                        else {
                            cengine_log_msg (stderr, ERROR, NO_TYPE, 
                                "Failed to add client_poll () to client thpool!");
                        } 
                    }
                }     

                else cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to get free client poll idx!");
            }

            else retval = 1;
        }
    }

    return retval;

}

// terminates and destroy a connection registered to a client
// returns 0 on success, 1 on error
int client_connection_end (Client *client, Connection *connection) {

    int retval = 1;

    if (client && connection) {
        // end the connection
        if (!connection_end (connection)) {
            // remove the connection from the client
            for (u8 i = 0; i < client->n_fds; i++) 
                if (client->fds[i].fd == connection->sock_fd)
                    client->fds[i].fd = -1;

            connection_delete (dlist_remove_element (client->connections, 
                dlist_get_element (client->connections, connection)));

            retval = 0;
        }
    }

    return retval;

}

/*** REQUESTS ***/

// These are the requests that we send to the server and we expect a response 

void *generateRequest (PacketType packetType, RequestType reqType) {

    size_t packetSize = sizeof (PacketHeader) + sizeof (RequestData);
    void *begin = client_generatePacket (packetType, packetSize);
    char *end = begin;

    RequestData *reqdata = (RequestData *) (end += sizeof (PacketHeader));
    reqdata->type = reqType;

    return begin;

}

u8 client_makeTestRequest (Client *client, Connection *connection) {

    // if (client && connection) {
    //     size_t packetSize = sizeof (PacketHeader);
    //     void *req = client_generatePacket (TEST_PACKET, packetSize);
    //     if (req) {
    //         if (client_sendPacket (connection, req, packetSize) < 0) 
    //             logMsg (stderr, ERROR, PACKET, "Failed to send test packet!");

    //         else logMsg (stdout, TEST, PACKET, "Sent test packet to server.");

    //         free (req);

    //         return 0;
    //     }
    // }

    // return 1;

}

#pragma endregion

/*** FILE SERVER ***/

#pragma region FILE SERVER

// TODO:
// request a file from the server
i8 client_file_get (Client *client, Connection *connection, const char *filename) {

    if (client && connection) {}

}

// TODO:
// send a file to the server
i8 client_file_send (Client *client, Connection *connection, const char *filename) {

    if (client && connection) {}

}

#pragma endregion

/*** GAME SERVER ***/

#pragma region GAME SERVER

// request to create a new multiplayer game
void *client_game_createLobby (Client *owner, Connection *connection, GameType gameType) {

    Lobby *new_lobby = NULL;

    // create a new connection
    Connection *new_con = client_make_new_connection (owner, connection->cerver->ip, 
        connection->cerver->port, false);

    if (new_con) {
        char buffer[1024];
        memset (buffer, 0, 1024);
        int rc = read (new_con->sock_fd, buffer, 1024);

        if (rc > 0) {
            char *end = buffer;
            PacketHeader *header = (PacketHeader *) end;
            #ifdef CLIENT_DEBUG
                if (header->packetType == SERVER_PACKET)
                    logMsg (stdout, DEBUG_MSG, NO_TYPE, "New connection - got a server packet.");
            #endif

            // authenticate using our server token
            size_t token_packet_size = sizeof (PacketHeader) + sizeof (RequestData) + sizeof (Token);
            void *token_packet = client_generatePacket (AUTHENTICATION, token_packet_size);
            if (token_packet) {
                char *end = token_packet;
                RequestData *req = (RequestData *) (end += sizeof (PacketHeader));
                req->type = CLIENT_AUTH_DATA;

                Token *tok = (Token *) (end += sizeof (RequestData));
                memcpy (tok->token, connection->server->token_data->token, sizeof (tok->token));

                client_sendPacket (new_con, token_packet, token_packet_size);
                free (token_packet);
            }

            else {
                // logMsg (stderr, ERROR, CLIENT, "New connection - failed to create auth packet!");
                client_end_connection (owner, new_con);
                return NULL;
            }

            memset (buffer, 0, 1024);
            rc = read (new_con->sock_fd, buffer, 1024);

            if (rc > 0) {
                end = buffer;
                RequestData *reqdata = (RequestData *) (end + sizeof (PacketHeader));
                if (reqdata->type == SUCCESS_AUTH) {
                    #ifdef CLIENT_DEBUG
                        logMsg (stdout, DEBUG_MSG, NO_TYPE, 
                            "New connection - authenticated to server.");
                    #endif

                    sleep (1);

                    // make the create lobby request
                    size_t create_packet_size = sizeof (PacketHeader) + sizeof (RequestData);
                    void *lobby_req = generateRequest (GAME_PACKET, LOBBY_CREATE);
                    if (lobby_req) {
                        client_sendPacket (new_con, lobby_req, create_packet_size);
                        free (lobby_req);
                    }

                    else {
                        logMsg (stderr, ERROR, CLIENT, 
                            "New connection - failed to create lobby packet!");
                        client_end_connection (owner, new_con);
                        return NULL;
                    }
                    
                    memset (buffer, 0, 1024);
                    rc = read (new_con->sock_fd, buffer, 1024);

                    if (rc > 0) {
                        end = buffer;
                        RequestData *reqdata = (RequestData *) (end += sizeof (PacketHeader));
                        if (reqdata->type == LOBBY_UPDATE) {
                            SLobby *got_lobby = (SLobby *) (end += sizeof (RequestData));
                            new_lobby = (Lobby *) malloc (sizeof (SLobby));
                            memcpy (new_lobby, got_lobby, sizeof (SLobby));
                        }
                    }
                }
                   
            }
        }

        client_end_connection (owner, new_con);
    }

    return new_lobby;

}

// FIXME: send game type to server
// request to join an on going game
void *client_game_joinLobby (Client *client, Connection *connection, GameType gameType) {

    if (client && connection) {
        // create & send a join lobby req packet to the server
        size_t packetSize = sizeof (PacketHeader) + sizeof (RequestData);
        void *req = generateRequest (GAME_PACKET, LOBBY_JOIN);

        if (req) {
            i8 retval = client_sendPacket (connection, req, packetSize);
            free (req);
            // return retval;
        }
    }

    return NULL;

}

// request the server to leave the lobby
i8 client_game_leaveLobby (Client *client, Connection *connection) {

    if (client && connection) {
        if (client->in_lobby) {
            // create & send a leave lobby req packet to the server
            size_t packetSize = sizeof (PacketHeader) + sizeof (RequestData);
            void *req = generateRequest (GAME_PACKET, LOBBY_LEAVE);

            if (req) {
                i8 retval = client_sendPacket (connection, req, packetSize);
                free (req);
                return retval;
            }
        }
    }

    return -1;

}

// request to destroy the current lobby, only if the client is the owner
i8 client_game_destroyLobby (Client *client, Connection *connection) {

    if (client && connection) {
        if (client->in_lobby) {
            // create & send a leave lobby req packet to the server
            size_t packetSize = sizeof (PacketHeader) + sizeof (RequestData);
            void *req = generateRequest (GAME_PACKET, LOBBY_DESTROY);

            if (req) {
                i8 retval = client_sendPacket (connection, req, packetSize);
                free (req);
                return retval;
            }
        }
    }

    return -1;

}

// the owner of the lobby can request to init the game
i8 client_game_startGame (Client *client, Connection *connection) {

    if (client && connection) {
        if (client->in_lobby) {
            // create & send a leave lobby req packet to the server
            size_t packetSize = sizeof (PacketHeader) + sizeof (RequestData);
            void *req = generateRequest (GAME_PACKET, GAME_INIT);

            if (req) {
                i8 retval = client_sendPacket (connection, req, packetSize);
                free (req);
                return retval;
            }
        }
    }

    return -1;

}