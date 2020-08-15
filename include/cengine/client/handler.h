#ifndef _CLIENT_HANDLER_H_
#define _CLIENT_HANDLER_H_

#include "client/config.h"
#include "client/client.h"
#include "client/connection.h"
#include "client/packets.h"

#define RECEIVE_PACKET_BUFFER_SIZE          8192

struct _Client;
struct _Connection;
struct _Packet;

struct _SockReceive {

    struct _Packet *spare_packet;
    size_t missing_packet;

    void *header;
    char *header_end;
    // unsigned int curr_header_pos;
    unsigned int remaining_header;
    bool complete_header;

};

typedef struct _SockReceive SockReceive;

CLIENT_PRIVATE SockReceive *sock_receive_new (void);

CLIENT_PRIVATE void sock_receive_delete (void *sock_receive_ptr);

// receives incoming data from the socket
CLIENT_PUBLIC void client_receive (struct _Client *client, struct _Connection *connection);

#endif