#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/errors.h"
#include "cengine/cerver/packets.h"

#include "cengine/utils/log.h"

Error *error_new (const char *msg) {

    Error *error = (Error *) malloc (sizeof (Error));
    if (error) {
        memset (error, 0, sizeof (Error));
        error->msg = msg ? str_new (msg) : NULL;
    }

    return error;

}

void error_delete (void *ptr) {

    if (ptr) {
        Error *error = (Error *) ptr;
        str_delete (error->msg);
        free (error);
    }

}

// handles error packets
void error_packet_handler (Packet *packet) {

    if (packet) {
        if (packet->packet_size >= (sizeof (PacketHeader) + sizeof (SError))) {
            char *end = packet->packet;
            SError *s_error = (SError *) (end += sizeof (PacketHeader));

            switch (s_error->error_type) {
                case ERR_SERVER_ERROR: break;
                case ERR_CREATE_LOBBY: break;
                case ERR_JOIN_LOBBY: break;
                case ERR_LEAVE_LOBBY: break;
                case ERR_FIND_LOBBY: break;
                case ERR_GAME_INIT: break;

                // FIXME:
                case ERR_FAILED_AUTH: {
                    // #ifdef CLIENT_DEBUG
                    // logMsg (stderr, ERROR, NO_TYPE, 
                    //     createString ("Failed to authenticate - %s", error->msg)); 
                    // #endif
                    // last_error.type = ERR_FAILED_AUTH;
                    // memset (last_error.msg, 0, sizeof (last_error.msg));
                    // strcpy (last_error.msg, error->msg);
                    // if (pack_info->client->errorType == ERR_FAILED_AUTH)
                    //     pack_info->client->errorAction (pack_info->client->errorArgs);
                }
                    
                break;

                default: 
                    cengine_log_msg (stderr, WARNING, NO_TYPE, "Unknown error recieved from server."); 
                    break;
            }
        }
    }

}