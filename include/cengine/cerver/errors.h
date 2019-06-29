#ifndef _CERVER_CLIENT_ERRORS_H_
#define _CERVER_CLIENT_ERRORS_H_

#include "cengine/types/string.h"

#include "cengine/cerver/packets.h"

typedef enum ErrorType {

    ERR_SERVER_ERROR = 0,   // internal server error, like no memory

    ERR_CREATE_LOBBY = 1,
    ERR_JOIN_LOBBY,
    ERR_LEAVE_LOBBY,
    ERR_FIND_LOBBY,

    ERR_GAME_INIT,

    ERR_FAILED_AUTH,

} ErrorType;

// when a client error happens, it sets the appropaited msg (if any)
// and an event is triggered
typedef struct Error {

    // TODO: maybe add time?
    ErrorType error_type;
    String *msg;

} Error;

extern Error *error_new (const char *msg);
extern void error_delete (void *ptr);

// handles error packets
extern void error_packet_handler (Packet *packet);

// serialized error data
typedef struct SError {

    ErrorType error_type;
    char msg[64];

} SError;

#endif