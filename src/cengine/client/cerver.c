#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/client/cerver.h"
#include "cengine/client/connection.h"
#include "cengine/client/packets.h"

#include "cengine/utils/utils.h"
#include "cengine/utils/log.h"

static Cerver *cerver_deserialize (SCerver *scerver);

#pragma region stats

static CerverStats *cerver_stats_new (void) {

    CerverStats *cerver_stats = (CerverStats *) malloc (sizeof (CerverStats));
    if (cerver_stats) {
        memset (cerver_stats, 0, sizeof (CerverStats));
        cerver_stats->received_packets = packets_per_type_new ();
        cerver_stats->sent_packets = packets_per_type_new ();
    } 

    return cerver_stats;

}

static void cerver_stats_delete (CerverStats *cerver_stats) {

    if (cerver_stats) {
        packets_per_type_delete (cerver_stats->received_packets);
        packets_per_type_delete (cerver_stats->sent_packets);
        
        free (cerver_stats);
    } 

}

void cerver_stats_print (Cerver *cerver) {

    if (cerver) {
        if (cerver->stats) {
            printf ("\nCerver's %s stats: \n", cerver->name->str);
            printf ("Threshold time:                %ld\n", cerver->stats->threshold_time);

            if (cerver->auth_required) {
                printf ("Client packets received:       %ld\n", cerver->stats->client_n_packets_received);
                printf ("Client receives done:          %ld\n", cerver->stats->client_receives_done);
                printf ("Client bytes received:         %ld\n\n", cerver->stats->client_bytes_received);

                printf ("On hold packets received:       %ld\n", cerver->stats->on_hold_n_packets_received);
                printf ("On hold receives done:          %ld\n", cerver->stats->on_hold_receives_done);
                printf ("On hold bytes received:         %ld\n\n", cerver->stats->on_hold_bytes_received);
            }

            printf ("\n");
            printf ("Total packets received:        %ld\n", cerver->stats->total_n_packets_received);
            printf ("Total receives done:           %ld\n", cerver->stats->total_n_receives_done);
            printf ("Total bytes received:          %ld\n\n", cerver->stats->total_bytes_received);

            printf ("\n");
            printf ("N packets sent:                %ld\n", cerver->stats->n_packets_sent);
            printf ("Total bytes sent:              %ld\n", cerver->stats->total_bytes_sent);

            printf ("\n");
            printf ("Current active client connections:         %ld\n", cerver->stats->current_active_client_connections);
            printf ("Current connected clients:                 %ld\n", cerver->stats->current_n_connected_clients);
            printf ("Current on hold connections:               %ld\n", cerver->stats->current_n_hold_connections);
            printf ("Total on hold connections:                 %ld\n", cerver->stats->total_on_hold_connections);
            printf ("Total clients:                             %ld\n", cerver->stats->total_n_clients);
            printf ("Unique clients:                            %ld\n", cerver->stats->unique_clients);
            printf ("Total client connections:                  %ld\n", cerver->stats->total_client_connections);

            printf ("\nReceived packets:\n");
            packets_per_type_print (cerver->stats->received_packets);

            printf ("\nSent packets:\n");
            packets_per_type_print (cerver->stats->sent_packets);
        }

        else {
            char *status = c_string_create ("Cerver %s does not have a reference to cerver stats!",
                cerver->name->str);
            if (status) {
                cengine_log_msg (stderr, LOG_ERROR, LOG_CERVER, status);
                free (status);
            }
        }
    }

    else {
        cengine_log_msg (stderr, LOG_WARNING, LOG_CERVER, 
            "Cant print stats of a NULL cerver!");
    }

}

#pragma endregion

#pragma region cerver

Cerver *cerver_new (void) {

    Cerver *cerver = (Cerver *) malloc (sizeof (Cerver));
    if (cerver) {
        memset (cerver, 0, sizeof (Cerver));

        cerver->name = NULL;
        cerver->welcome = NULL;

        cerver->stats = NULL;
    }

    return cerver;

}

void cerver_delete (void *ptr) {

    if (ptr) {
        Cerver *cerver = (Cerver *) ptr;

        str_delete (cerver->name);
        str_delete (cerver->welcome);

        cerver_stats_delete (cerver->stats);

        free (cerver);
    }

}

static void cerver_check_info_handle_auth (Cerver *cerver, Client *client, Connection *connection) {

    if (cerver && connection) {
        if (cerver->auth_required) {
            // #ifdef CLIENT_DEBUG
            cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver requires authentication.");
            // #endif
            if (connection->auth_data) {
                #ifdef CLIENT_DEBUG
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Sending auth data to cerver...");
                #endif

                if (!connection->auth_packet) {
                    if (!connection_generate_auth_packet (connection)) {
                        char *status = c_string_create ("cerver_check_info () - Generated connection %s auth packet!",
                            connection->name->str);
                        if (status) {
                            cengine_log_success (status);
                            free (status);
                        }
                    }

                    else {
                        char *status = c_string_create ("cerver_check_info () - Failed to generate connection %s auth packet!",
                            connection->name->str);
                        if (status) {
                            cengine_log_error (status);
                            free (status);
                        }
                    }
                }

                if (connection->auth_packet) {
                    packet_set_network_values (connection->auth_packet, NULL, connection);

                    if (!packet_send (connection->auth_packet, 0, NULL, false)) {
                        char *s = c_string_create ("cerver_check_info () - Sent connection %s auth packet!",
                            connection->name->str);
                        if (s) {
                            cengine_log_success (s);
                            free (s);
                        }

                        client_event_trigger (CLIENT_EVENT_AUTH_SENT, client, connection);
                    }

                    else {
                        char *s = c_string_create ("cerver_check_info () - Failed to send connection %s auth packet!",
                            connection->name->str);
                        if (s) {
                            cengine_log_error (s);
                            free (s);
                        }
                    }
                }

                if (cerver->uses_sessions) {
                    cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver supports sessions.");
                }
            }

            else {
                char *s = c_string_create ("Connection %s does NOT have an auth packet!",
                    connection->name->str);
                if (s) {
                    cengine_log_error (s);
                    free (s);
                }
            }
        }

        else {
            #ifdef CLIENT_DEBUG
            cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver does NOT require authentication.");
            #endif
        }
    }

}

// compare the info the server sent us with the one we expected 
// and ajust our connection values if necessary
static u8 cerver_check_info (Cerver *cerver, Client *client, Connection *connection) {

    u8 retval = 1;

    if (cerver && connection) {
        connection->cerver = cerver;

        #ifdef CLIENT_DEBUG
        char *s = c_string_create ("Connected to cerver %s.", cerver->name->str);
        if (s) {
            cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, s);
            free (s);
        }

        if (cerver->welcome) {
            printf ("%s\n", cerver->welcome->str);
        }
        
        switch (cerver->protocol) {
            case PROTOCOL_TCP: 
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver using TCP protocol."); 
                break;
            case PROTOCOL_UDP: 
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver using UDP protocol.");
                break;

            default: 
                cengine_log_msg (stdout, LOG_WARNING, LOG_NO_TYPE, "Cerver using unknown protocol."); 
                break;
        }
        #endif

        if (cerver->use_ipv6) {
            #ifdef CLIENT_DEBUG
            cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver is configured to use ipv6");
            #endif
        }

        #ifdef CLIENT_DEBUG
        switch (cerver->type) {
            case CUSTOM_CERVER:
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver type: CUSTOM");
                break;
            case FILE_CERVER:
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver type: FILE");
                break;
            case WEB_CERVER:
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver type: WEB");
                break;
            case GAME_CERVER:
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver type: GAME");
                break;

            default: 
                cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Cerver type: UNKNOWN"); 
                break;
        }
        #endif

        cerver_check_info_handle_auth (cerver, client, connection);

        retval = 0;
    }

    return retval;

}

#pragma endregion

#pragma region handler

static void cerver_packet_handle_info (Packet *packet) {

    if (packet->data && (packet->data_size > 0)) {
        char *end = (char *) packet->data;

        #ifdef CLIENT_DEBUG
        cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Received a cerver info packet.");
        #endif
        Cerver *cerver = cerver_deserialize ((SCerver *) end);
        if (cerver_check_info (cerver, packet->client, packet->connection))
            cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to correctly check cerver info!");
    }

}

// handles cerver type packets
void cerver_packet_handler (Packet *packet) {

    if (packet) {
        if (packet->header) {
            switch (packet->header->request_type) {
                case CERVER_INFO: {
                   cerver_packet_handle_info (packet);
                } break;

                // the cerves is going to be teardown, we have to disconnect
                case CERVER_TEARDOWN:
                    #ifdef CLIENT_DEBUG
                    cengine_log_msg (stdout, LOG_WARNING, LOG_NO_TYPE, "---> Server teardown! <---");
                    #endif
                    client_got_disconnected (packet->client);
                    client_event_trigger (CLIENT_EVENT_DISCONNECTED, packet->client, NULL);
                    break;

                case CERVER_INFO_STATS:
                    // #ifdef CLIENT_DEBUG
                    // cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Received a cerver stats packet.");
                    // #endif
                    break;

                case CERVER_GAME_STATS:
                    // #ifdef CLIENT_DEBUG
                    // cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Received a cerver game stats packet.");
                    // #endif
                    break;

                default: 
                    cengine_log_msg (stderr, LOG_WARNING, LOG_NO_TYPE, "Unknown cerver type packet."); 
                    break;
            }
        }
    }

}

#pragma endregion

#pragma region serialization

static Cerver *cerver_deserialize (SCerver *scerver) {
    
    Cerver *cerver = NULL;

    if (scerver) {
        cerver = cerver_new ();
        if (cerver) {
            cerver->type = scerver->type;

            cerver->name = str_new (scerver->name);
            if (strlen (scerver->welcome)) cerver->welcome = str_new (scerver->welcome);

            cerver->use_ipv6 = scerver->use_ipv6;
            cerver->protocol = scerver->protocol;
            cerver->port = scerver->port;

            cerver->auth_required = scerver->auth_required;
            cerver->uses_sessions = scerver->uses_sessions;

            cerver->stats = cerver_stats_new ();
        }
    }

    return cerver;

}

#pragma endregion