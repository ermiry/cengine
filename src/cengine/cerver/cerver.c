#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/cerver.h"
#include "cengine/cerver/packets.h"

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
        cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Connected to cerver...");
        switch (cerver->protocol) {
            case PROTOCOL_TCP: 
                cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Cerver using TCP protocol."); 
                break;
            case PROTOCOL_UDP: 
                cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Cerver using UDP protocol.");
                break;

            default: 
                cengine_log_msg (stdout, WARNING, NO_TYPE, "Cerver using unknown protocol."); 
                break;
        }
        #endif

        if (cerver->use_ipv6) {
            #ifdef CLIENT_DEBUG
            cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Cerver is configured to use ipv6");
            #endif
            // TODO: modify the connection to use ipv6
        }

        #ifdef CLIENT_DEBUG
        switch (cerver->type) {
            case FILE_SERVER:
                cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Cerver is of type: file server");
                break;
            case WEB_SERVER:
                cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Cerver is of type: web server");
                break;
            case GAME_SERVER:
                cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Cerver is of type: game server");
                break;

            default: 
                cengine_log_msg (stderr, ERROR, NO_TYPE, "Cerver is of unknown type."); 
                break;
        }
        #endif

        if (cerver->auth_required) {
            #ifdef CLIENT_DEBUG
            cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Cerver requires authentication.");
            #endif
            if (connection->auth_action) {
                #ifdef CLIENT_DEBUG
                cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Sending auth data to cerver...");
                #endif
                connection->auth_action (connection->auth_data);
                retval = 0;
            }

            else {
                cengine_log_msg (stderr, WARNING, NO_TYPE, 
                    "Can't authenticate with server --- no auth action neither auth data have been setup");
            } 
        }

        else {
            #ifdef CLIENT_DEBUG
            cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Cerver does not requires authentication.");
            #endif
            retval = 0;
        }
    }

    return retval;

}

// handles cerver type packets
void cerver_packet_handler (Packet *packet) {

    if (packet) {
        if (packet->packet_size >= (sizeof (PacketHeader) + sizeof (RequestData))) {
            char *end = packet->packet;
            RequestData *req = (RequestData *) (end += sizeof (PacketHeader));

            switch (req->type) {
                case SERVER_INFO: {
                    #ifdef CLIENT_DEBUG
                    cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, "Recieved a cerver info packet.");
                    #endif
                    packet->connection->cerver = cerver_deserialize ((SCerver *) (end += sizeof (RequestData)));
                    if (cerver_check_info (packet->connection->cerver, packet->connection))
                        cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to correctly check cerver info!");
                } break;

                // TODO:
                case SERVER_TEARDOWN:
                    cengine_log_msg (stdout, WARNING, NO_TYPE, "\n---> Server teardown!! <---\n");
                    break;

                default: 
                    cengine_log_msg (stderr, WARNING, NO_TYPE, "Unknown cerver type packet."); 
                    break;
            }
        }
    }

}