#ifndef _CEVER_CLIENT_CONNECTION_H_
#define _CEVER_CLIENT_CONNECTION_H_

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cerver/network.h"
#include "cengine/cerver/cerver.h"
#include "cengine/cerver/handler.h"
#include "cengine/cerver/packets.h"

#define CONNECTION_MAX_SLEEP                60        // used for connection with exponential backoff (secs)     

struct _Cerver;
struct _Client;
struct _Packet;

struct _Connection {

    String *name;

    i32 sock_fd;
    bool use_ipv6;  
    Protocol protocol;
    u16 port; 

    u32 max_sleep;
    bool connected;                     // is the socket connected?

    String *ip;                         // cerver's ip
    struct sockaddr_storage address;    // cerver's address

    // info about the cerver we are connected to
    struct _Cerver *cerver;

    SockReceive *sock_receive;

    void *auth_data;                    // maybe auth credentials
    size_t auth_data_size;
    Action delete_auth_data;            // destroys the auth data when the connection ends
    struct _Packet *auth_packet;

};

typedef struct _Connection Connection;

extern Connection *connection_new (void);
extern void connection_delete (void *ptr);

extern int connection_comparator (void *one, void *two);

// sets the connection max sleep (wait time) to try to connect to the cerver
extern void connection_set_max_sleep (Connection *connection, u32 max_sleep);

// sets the connection auth data to send whenever the cerver requires authentication 
// and a method to destroy it once the connection has ended
extern void connection_set_auth_data (Connection *connection, void *auth_data, size_t auth_data_size, Action delete_auth_data);

// removes the connection auth data and its destroy method
extern void connection_remove_auth_data (Connection *connection);

// generates the connection auth packet to be send to the server
// this is also generated automatically whenever the cerver ask for authentication
extern void connection_generate_auth_packet (Connection *connection);

// creates a new connection that is ready to be started
extern Connection *connection_create (const char *name,
     const char *ip_address, u16 port, Protocol protocol, bool use_ipv6);

// starts a connection -> connects to the specified ip and port
// returns 0 on success, 1 on error
extern int connection_start (Connection *connection);

// starts listening and receiving data in the connection sock
extern void client_connection_update (void *ptr);

// ends a connection
extern void connection_end (struct _Client *client, Connection *connection);

#endif