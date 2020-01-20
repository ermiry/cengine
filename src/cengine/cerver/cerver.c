#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/cerver.h"
#include "cengine/cerver/connection.h"
#include "cengine/cerver/packets.h"

#include "cengine/utils/utils.h"
#include "cengine/utils/log.h"

Cerver *cerver_new (void) {

    Cerver *cerver = (Cerver *) malloc (sizeof (Cerver));
    if (cerver) {
        memset (cerver, 0, sizeof (Cerver));
        cerver->ip = NULL;
        cerver->name = NULL;
        cerver->token = NULL;
    }

    return cerver;

}

void cerver_delete (void *ptr) {

    if (ptr) {
        Cerver *cerver = (Cerver *) ptr;

        str_delete (cerver->name);
        str_delete (cerver->ip);

        free (cerver);
    }

}

static Cerver *cerver_deserialize (SCerver *scerver) {
    
    Cerver *cerver = NULL;

    if (scerver) {
        cerver = cerver_new ();
        if (cerver) {
            cerver->use_ipv6 = scerver->use_ipv6;
            cerver->protocol = scerver->protocol;
            cerver->port = scerver->port;

            cerver->name = str_new (scerver->name);
            cerver->type = scerver->type;
            cerver->auth_required = scerver->auth_required;
            cerver->uses_sessions = scerver->uses_sessions;
        }
    }

    return cerver;

}

// compare the info the server sent us with the one we expected 
// and ajust our connection values if necessary
static u8 cerver_check_info (Cerver *cerver, Connection *connection) {

    u8 retval = 1;

    if (cerver && connection) {
        #ifdef CLIENT_DEBUG
        cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, 
            c_string_create ("Connected to cerver %s.", cerver->name->str));
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
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver is of type: custom cerver");
                break;
            case FILE_CERVER:
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver is of type: file cerver");
                break;
            case WEB_CERVER:
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver is of type: web cerver");
                break;
            case GAME_CERVER:
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver is of type: game cerver");
                break;

            default: 
                cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Cerver is of unknown type."); 
                break;
        }
        #endif

        if (cerver->auth_required) {
            #ifdef CLIENT_DEBUG
            cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver requires authentication.");
            #endif
            if (connection->auth_data) {
                #ifdef CLIENT_DEBUG
                cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Sending auth data to cerver...");
                #endif

                if (!connection->auth_packet) connection_generate_auth_packet (connection);

                if (packet_send (connection->auth_packet, 0, NULL, false)) {
                    cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to send connection auth packet!");
                }

                retval = 0;
            }
        }

        else {
            #ifdef CLIENT_DEBUG
            cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Cerver does not requires authentication.");
            #endif
        }
        
        retval = 0;
    }

    return retval;

}

// handles cerver type packets
void cerver_packet_handler (Packet *packet) {

    if (packet) {
        if (packet->data_size >= sizeof (RequestData)) {
            char *end = (char *) packet->data;
            RequestData *req = (RequestData *) (end);

            switch (req->type) {
                case CERVER_INFO: {
                    #ifdef CLIENT_DEBUG
                    cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, "Recieved a cerver info packet.");
                    #endif
                    packet->connection->cerver = cerver_deserialize ((SCerver *) (end += sizeof (RequestData)));
                    if (cerver_check_info (packet->connection->cerver, packet->connection))
                        cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to correctly check cerver info!");
                } break;

                // the cerves is going to be teardown, we have to disconnect
                case CERVER_TEARDOWN:
                    #ifdef CLIENT_DEBUG
                    cengine_log_msg (stdout, LOG_WARNING, LOG_NO_TYPE, "---> Server teardown! <---");
                    #endif
                    client_connection_end (packet->client, packet->connection);
                    client_event_trigger (packet->client, EVENT_DISCONNECTED);
                    break;

                case CERVER_INFO_STATS:
                    break;

                case CERVER_GAME_STATS:
                    break;

                default: 
                    cengine_log_msg (stderr, LOG_WARNING, LOG_NO_TYPE, "Unknown cerver type packet."); 
                    break;
            }
        }
    }

}