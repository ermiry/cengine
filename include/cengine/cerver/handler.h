#ifndef _CERVER_CLIENT_HANDLER_H_
#define _CERVER_CLIENT_HANDLER_H_

#include "cengine/cerver/client.h"
#include "cengine/cerver/connection.h"
#include "cengine/cerver/packets.h"

#define RECEIVE_PACKET_BUFFER_SIZE          8192

struct _Client;
struct _Connection;
struct _Packet;

typedef struct SockReceive {

    struct _Packet *spare_packet;
    size_t missing_packet;

} SockReceive;

extern SockReceive *sock_receive_new (void);

extern void sock_receive_delete (void *sock_receive_ptr);

// receives incoming data from the socket
extern void client_receive (struct _Client *client, struct _Connection *connection);

#endif