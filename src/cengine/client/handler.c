#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "client/types/types.h"

#include "client/collections/dlist.h"

#include "client/network.h"
#include "client/packets.h"
#include "client/events.h"
#include "client/errors.h"
#include "client/client.h"
#include "client/cerver.h"
#include "client/connection.h"
#include "client/handler.h"
#include "client/game.h"

#include "client/threads/thread.h"

#include "client/utils/log.h"
#include "client/utils/utils.h"

#pragma region auxiliary

SockReceive *sock_receive_new (void) {

    SockReceive *sr = (SockReceive *) malloc (sizeof (SockReceive));
    if (sr) {
        sr->spare_packet = NULL;
        sr->missing_packet = 0;

        sr->header = NULL;
        sr->header_end = NULL;
        sr->remaining_header = 0;
        sr->complete_header = false;
    } 

    return sr;

}

void sock_receive_delete (void *sock_receive_ptr) {

    if (sock_receive_ptr) {
        packet_delete (((SockReceive *) sock_receive_ptr)->spare_packet);
        free (sock_receive_ptr);
    }

}

#pragma endregion

#pragma region handlers

// handles a client type packet
static void client_client_packet_handler (Packet *packet) {

    if (packet) {
        if (packet->header) {
            switch (packet->header->request_type) {
                // the cerver close our connection
                case CLIENT_CLOSE_CONNECTION:
                    client_connection_end (packet->client, packet->connection);
                    break;

                // the cerver has disconneted us
                case CLIENT_DISCONNET:
                    client_got_disconnected (packet->client);
                    client_event_trigger (CLIENT_EVENT_DISCONNECTED, packet->client, NULL);
                    break;

                default: 
                    client_log_msg (stderr, LOG_WARNING, LOG_NO_TYPE, "Unknown client packet type.");
                    break;
            }
        }
    }

}

// get the token from the packet data
// returns 0 on succes, 1 on error
static u8 auth_strip_token (Packet *packet, Client *client) {

    u8 retval = 1;

    if (packet) {
        // check we have a big enough packet
        if (packet->data_size > 0) {
            char *end = (char *) packet->data;

            // check if we have a token
            if (packet->data_size == (sizeof (SToken))) {
                SToken *s_token = (SToken *) (end);
                retval = client_set_session_id (client, s_token->token);
            }
        }
    }

    return retval;

}

static void client_auth_success_handler (Packet *packet) {

    if (packet) {
        packet->connection->authenticated = true;

        if (packet->connection->cerver) {
            if (packet->connection->cerver->uses_sessions) {
                if (!auth_strip_token (packet, packet->client)) {
                    #ifdef AUTH_DEBUG
                    char *status = c_string_create ("Got client's <%s> session id <%s>",
                        packet->client->name->str, packet->client->session_id->str);
                    if (status) {
                        client_log_debug (status);
                        free (status);
                    }
                    #endif
                }
            }
        }

        client_event_trigger (CLIENT_EVENT_SUCCESS_AUTH, packet->client, packet->connection);
    }

}

static void client_auth_packet_handler (Packet *packet) {

    if (packet) {
        if (packet->header) {
            switch (packet->header->request_type) {
                // 24/01/2020 -- cerver requested authentication, if not, we will be disconnected
                case AUTH_PACKET_TYPE_REQUEST_AUTH:
                    break;

                // we recieve a token from the cerver to use in sessions
                case AUTH_PACKET_TYPE_CLIENT_AUTH:
                    break;

                // we have successfully authenticated with the server
                case AUTH_PACKET_TYPE_SUCCESS:
                    client_auth_success_handler (packet);
                    break;

                default: 
                    client_log_msg (stderr, LOG_WARNING, LOG_NO_TYPE, "Unknown auth packet type.");
                    break;
            }
        }
    }

}

// handles a request made from the cerver
static void client_request_packet_handler (Packet *packet) {

    if (packet) {
        if (packet->header) {
            switch (packet->header->request_type) {
                default: 
                    client_log_msg (stderr, LOG_WARNING, LOG_NO_TYPE, "Unknown request from cerver");
                    break;
            }
        }
    }

}

// the client handles a packet based on its type
static void client_packet_handler (void *data) {

    if (data) {
        Packet *packet = (Packet *) data;
        packet->client->stats->n_packets_received += 1;

        bool good = true;
        if (packet->client->check_packets) {
            good = packet_check (packet);
        }

        if (good) {
            switch (packet->header->packet_type) {
                // handles cerver type packets
                case CERVER_PACKET:
                    packet->client->stats->received_packets->n_cerver_packets += 1;
                    packet->connection->stats->received_packets->n_cerver_packets += 1;
                    cerver_packet_handler (packet); 
                    break;

                // handles a client type packet
                case CLIENT_PACKET:
                    client_client_packet_handler (packet);
                    break;

                // handles an error from the server
                case ERROR_PACKET: 
                    packet->client->stats->received_packets->n_error_packets += 1;
                    packet->connection->stats->received_packets->n_error_packets += 1;
                    error_packet_handler (packet); 
                    break;

                // handles authentication packets
                case AUTH_PACKET: 
                    packet->client->stats->received_packets->n_auth_packets += 1;
                    packet->connection->stats->received_packets->n_auth_packets += 1;
                    client_auth_packet_handler (packet); 
                    break;

                // handles a request made from the server
                case REQUEST_PACKET: 
                    packet->client->stats->received_packets->n_request_packets += 1; 
                    packet->connection->stats->received_packets->n_request_packets += 1; 
                    client_request_packet_handler (packet);
                    break;

                // handles a game packet sent from the server
                case GAME_PACKET: 
                    packet->client->stats->received_packets->n_game_packets += 1;
                    packet->connection->stats->received_packets->n_game_packets += 1;
                    client_game_packet_handler (packet);
                    break;

                // user set handler to handle app specific errors
                case APP_ERROR_PACKET: 
                    packet->client->stats->received_packets->n_app_error_packets += 1;
                    packet->connection->stats->received_packets->n_app_error_packets += 1;
                    if (packet->client->app_error_packet_handler)
                        packet->client->app_error_packet_handler (packet);
                    break;

                // user set handler to handler app specific packets
                case APP_PACKET:
                    packet->client->stats->received_packets->n_app_packets += 1;
                    packet->connection->stats->received_packets->n_app_packets += 1;
                    if (packet->client->app_packet_handler)
                        packet->client->app_packet_handler (packet);
                    break;

                // custom packet hanlder
                case CUSTOM_PACKET: 
                    packet->client->stats->received_packets->n_custom_packets += 1;
                    packet->connection->stats->received_packets->n_custom_packets += 1;
                    if (packet->client->custom_packet_handler)
                        packet->client->custom_packet_handler (packet);
                    break;

                // handles a test packet form the cerver
                case TEST_PACKET: 
                    packet->client->stats->received_packets->n_test_packets += 1;
                    packet->connection->stats->received_packets->n_test_packets += 1;
                    client_log_msg (stdout, LOG_TEST, LOG_NO_TYPE, "Got a test packet from cerver.");
                    break;

                default:
                    packet->client->stats->received_packets->n_bad_packets += 1;
                    packet->connection->stats->received_packets->n_bad_packets += 1;
                    #ifdef CLIENT_DEBUG
                    client_log_msg (stdout, LOG_WARNING, LOG_NO_TYPE, "Got a packet of unknown type.");
                    #endif
                    break;
            }
        }

        packet_delete (packet);
    }

}

#pragma endregion

#pragma region receive

static void client_receive_handle_spare_packet (Client *client, Connection *connection, 
    size_t buffer_size, char **end, size_t *buffer_pos) {

    if (connection->sock_receive->header) {
        // copy the remaining header size
        memcpy (connection->sock_receive->header_end, (void *) *end, connection->sock_receive->remaining_header);

        connection->sock_receive->complete_header = true;
    }

    else if (connection->sock_receive->spare_packet) {
        size_t copy_to_spare = 0;
        if (connection->sock_receive->missing_packet < buffer_size) 
            copy_to_spare = connection->sock_receive->missing_packet;

        else copy_to_spare = buffer_size;

        // append new data from buffer to the spare packet
        if (copy_to_spare > 0) {
            packet_append_data (connection->sock_receive->spare_packet, *end, copy_to_spare);

            // check if we can handler the packet 
            size_t curr_packet_size = connection->sock_receive->spare_packet->data_size + sizeof (PacketHeader);
            if (connection->sock_receive->spare_packet->header->packet_size == curr_packet_size) {
                connection->sock_receive->spare_packet->client = client;
                connection->sock_receive->spare_packet->connection = connection;

                connection->full_packet = true;
                client_packet_handler (connection->sock_receive->spare_packet);

                connection->sock_receive->spare_packet = NULL;
                connection->sock_receive->missing_packet = 0;
            }

            else connection->sock_receive->missing_packet -= copy_to_spare;

            // offset for the buffer
            if (copy_to_spare < buffer_size) *end += copy_to_spare;
            *buffer_pos += copy_to_spare;
        }
    }

}

// splits the entry buffer in packets of the correct size
static void client_receive_handle_buffer (Client *client, Connection *connection, 
    char *buffer, size_t buffer_size) {

    if (buffer && (buffer_size > 0)) {
        char *end = buffer;
        size_t buffer_pos = 0;

        SockReceive *sock_receive = connection->sock_receive;

        client_receive_handle_spare_packet (
            client, connection, 
            buffer_size, &end, 
            &buffer_pos
        );

        PacketHeader *header = NULL;
        size_t packet_size = 0;
        // char *packet_data = NULL;

        size_t remaining_buffer_size = 0;
        size_t packet_real_size = 0;
        size_t to_copy_size = 0;

        bool spare_header = false;

        while (buffer_pos < buffer_size) {
            remaining_buffer_size = buffer_size - buffer_pos;

            if (sock_receive->complete_header) {
                packet_header_copy (&header, (PacketHeader *) sock_receive->header);
                // header = ((PacketHeader *) sock_receive->header);
                // packet_header_print (header);

                end += sock_receive->remaining_header;
                buffer_pos += sock_receive->remaining_header;
                // printf ("buffer pos after copy to header: %ld\n", buffer_pos);

                // reset sock header values
                free (sock_receive->header);
                sock_receive->header = NULL;
                sock_receive->header_end = NULL;
                // sock_receive->curr_header_pos = 0;
                // sock_receive->remaining_header = 0;
                sock_receive->complete_header = false;

                spare_header = true;
            }

            else if (remaining_buffer_size >= sizeof (PacketHeader)) {
                header = (PacketHeader *) end;
                end += sizeof (PacketHeader);
                buffer_pos += sizeof (PacketHeader);

                // packet_header_print (header);

                spare_header = false;
            }

            if (header) {
                // check the packet size
                packet_size = header->packet_size;
                if ((packet_size > 0) /* && (packet_size < 65536) */) {
                    // printf ("packet_size: %ld\n", packet_size);
                    // end += sizeof (PacketHeader);
                    // buffer_pos += sizeof (PacketHeader);
                    // printf ("first buffer pos: %ld\n", buffer_pos);

                    Packet *packet = packet_new ();
                    if (packet) {
                        packet_header_copy (&packet->header, header);
                        packet->packet_size = header->packet_size;
                        // packet->cerver = cerver;
                        // packet->lobby = lobby;
                        packet->client = client;
                        packet->connection = connection;

                        if (spare_header) {
                            free (header);
                            header = NULL;
                        }

                        // check for packet size and only copy what is in the current buffer
                        packet_real_size = packet->header->packet_size - sizeof (PacketHeader);
                        to_copy_size = 0;
                        if ((remaining_buffer_size - sizeof (PacketHeader)) < packet_real_size) {
                            sock_receive->spare_packet = packet;

                            if (spare_header) to_copy_size = buffer_size - sock_receive->remaining_header;
                            else to_copy_size = remaining_buffer_size - sizeof (PacketHeader);
                            
                            sock_receive->missing_packet = packet_real_size - to_copy_size;
                        }

                        else {
                            to_copy_size = packet_real_size;
                            packet_delete (sock_receive->spare_packet);
                            sock_receive->spare_packet = NULL;
                        } 

                        // printf ("to copy size: %ld\n", to_copy_size);
                        packet_set_data (packet, (void *) end, to_copy_size);

                        end += to_copy_size;
                        buffer_pos += to_copy_size;
                        // printf ("second buffer pos: %ld\n", buffer_pos);

                        if (!sock_receive->spare_packet) {
                            connection->full_packet = true;
                            client_packet_handler (packet);
                        }
                            
                    }

                    else {
                        client_log_msg (stderr, LOG_ERROR, LOG_CLIENT, 
                            "Failed to create a new packet in cerver_handle_receive_buffer ()");
                    }
                }

                else {
                    char *status = c_string_create ("Got a packet of invalid size: %ld", packet_size);
                    if (status) {
                        client_log_msg (stderr, LOG_WARNING, LOG_CLIENT, status); 
                        free (status);
                    }
                    
                    break;
                }
            }

            else {
                if (sock_receive->spare_packet) packet_append_data (sock_receive->spare_packet, (void *) end, remaining_buffer_size);

                else {
                    // handle part of a new header
                    // #ifdef CERVER_DEBUG
                    // client_log_debug ("Handle part of a new header...");
                    // #endif

                    // printf ("buffer size: %ld\n", buffer_size);
                    // printf ("buffer pos: %ld\n", buffer_pos);
                    // printf ("remaining_buffer_size: %ld\n", remaining_buffer_size);

                    // copy the piece of possible header that was cut of between recv ()
                    sock_receive->header = malloc (sizeof (PacketHeader));
                    memcpy (sock_receive->header, (void *) end, remaining_buffer_size);

                    sock_receive->header_end = (char *) sock_receive->header;
                    sock_receive->header_end += remaining_buffer_size;

                    // sock_receive->curr_header_pos = remaining_buffer_size;
                    sock_receive->remaining_header = sizeof (PacketHeader) - remaining_buffer_size;

                    // printf ("curr header pos: %d\n", sock_receive->curr_header_pos);
                    // printf ("remaining header: %d\n", sock_receive->remaining_header);

                    buffer_pos += remaining_buffer_size;
                }
            }
        }
    }

}

// handles a failed recive from a connection associatd with a client
// end sthe connection to prevent seg faults or signals for bad sock fd
static void client_receive_handle_failed (Client *client, Connection *connection) {

    if (client && connection) {
        if (!client_connection_end (client, connection)) {
            // check if the client has any other active connection
            if (client->connections->size <= 0) {
                client->running = false;
            }
        }
    }

}

// receives incoming data from the socket
void client_receive (Client *client, Connection *connection) {

    if (client && connection) {
        char *packet_buffer = (char *) calloc (connection->receive_packet_buffer_size, sizeof (char));
        if (packet_buffer) {
            ssize_t rc = recv (connection->socket->sock_fd, packet_buffer, connection->receive_packet_buffer_size, 0);

            switch (rc) {
                case -1: {
                    if (errno != EWOULDBLOCK) {
                        #ifdef CLIENT_DEBUG 
                        char *s = c_string_create ("client_receive () - rc < 0 - sock fd: %d", connection->socket->sock_fd);
                        if (s) {
                            client_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, s);
                            free (s);
                        }
                        perror ("Error");
                        #endif

                        client_receive_handle_failed (client, connection);
                    }
                } break;

                case 0: {
                    // man recv -> steam socket perfomed an orderly shutdown
                    // but in dgram it might mean something?
                    #ifdef CLIENT_DEBUG
                    char *s = c_string_create ("client_receive () - rc == 0 - sock fd: %d",
                        connection->socket->sock_fd);
                    if (s) {
                        client_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, s);
                        free (s);
                    }
                    // perror ("Error");
                    #endif
                    
                    client_receive_handle_failed (client, connection);
                } break;

                default: {
                    // char *s = c_string_create ("Connection %s rc: %ld",
                    //     connection->name->str, rc);
                    // if (s) {
                    //     client_log_msg (stdout, LOG_DEBUG, LOG_CLIENT, s);
                    //     free (s);
                    // }

                    client->stats->n_receives_done += 1;
                    client->stats->total_bytes_received += rc;

                    connection->stats->n_receives_done += 1;
                    connection->stats->total_bytes_received += rc;

                    // handle the recived packet buffer -> split them in packets of the correct size
                    client_receive_handle_buffer (
                        client, 
                        connection, 
                        packet_buffer, 
                        rc
                    );
                } break;
            }

            free (packet_buffer);
        }

        else {
            #ifdef CLIENT_DEBUG
            client_log_msg (stderr, LOG_ERROR, LOG_CLIENT, 
                "Failed to allocate a new packet buffer!");
            #endif
        }
    }

}

#pragma endregion