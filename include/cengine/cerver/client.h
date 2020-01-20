#ifndef _CERVER_CLIENT_H_
#define _CERVER_CLIENT_H_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/cerver/network.h"
#include "cengine/cerver/events.h"
#include "cengine/cerver/errors.h"
#include "cengine/cerver/connection.h"
#include "cengine/cerver/packets.h"

struct _Client;
struct _Connection;
struct _Packet;
struct _PacketsPerType;

typedef struct ClientStats {

    time_t threshold_time;                          // every time we want to reset cerver stats (like packets), defaults 24hrs
    u64 n_packets_received;                         // total number of packets received (packet header + data)
    u64 n_receives_done;                            // total amount of actual calls to recv ()
    u64 total_bytes_received;                       // total amount of bytes received in the cerver
    u64 n_packets_sent;                             // total number of packets sent
    u64 total_bytes_sent;                           // total amount of bytes sent by the cerver

    struct _PacketsPerType *received_packets;
    struct _PacketsPerType *sent_packets;

} ClientStats;

extern void client_stats_print (struct _Client *client);

struct _Client {

    DoubleList *connections;

    bool running;                   // any connection is active

    // all the actions that have been registered to a client
    DoubleList *registered_actions;

    // custom packet handlers
    Action app_packet_handler;
    Action app_error_packet_handler;
    Action custom_packet_handler;

    time_t time_started;
    u64 uptime;

    ClientStats *stats;

};

typedef struct _Client Client;

// sets the client's poll timeout
extern void client_set_poll_timeout (Client *client, u32 timeout);

// sets the clients's thpool number of threads
extern void client_set_thpool_n_threads (Client *client, u16 n_threads);

// sets a cutom app packet hanlder and a custom app error packet handler
extern void client_set_app_handlers (Client *client, Action app_handler, Action app_error_handler);

// sets a custom packet handler
extern void client_set_custom_handler (Client *client, Action custom_handler);

// creates a new client, whcih may be used to create connections
extern Client *client_create (void);

// stops any activae connection and destroys a client
extern u8 client_teardown (Client *client);

// returns a connection assocaited with a socket
extern struct _Connection *client_connection_get_by_socket (Client *client, i32 sock_fd);

// returns a connection (registered to a client) by its name
extern struct _Connection *client_connection_get_by_name (Client *client, const char *name);

// creates a new connection and registers it to the specified client;
// the connection should be ready to be started
// returns 0 on success, 1 on error
extern int client_connection_create (Client *client, const char *name,
    const char *ip_address, u16 port, Protocol protocol, bool use_ipv6);

// registers an existing connection to a client
// retuns 0 on success, 1 on error
extern int client_connection_register (Client *client, struct _Connection *connection);

// this is a blocking method and ONLY works for cerver packets
// connects the client connection and makes a first request to the cerver
// then listen for packets until the target one is received, 
// then it returns the packet data as it is
// returns 0 on success, 1 on error
extern int client_connection_request_to_cerver (Client *client, struct _Connection *connection, 
    struct _Packet *request_packet);

// starts a client connection
// returns 0 on success, 1 on error
extern int client_connection_start (Client *client, struct _Connection *connection);

// terminates and destroy a connection registered to a client
// returns 0 on success, 1 on error
extern int client_connection_end (Client *client, struct _Connection *connection);

/*** Files ***/

// requests a file from the server
// filename: the name of the file to request
// file complete event will be sent when the file is finished
// appropiate error is set on bad filename or error in file transmission
// returns 0 on success sending request, 1 on failed to send request
extern u8 client_file_get (Client *client, struct _Connection *connection, const char *filename);

// sends a file to the server
// filename: the name of the file the cerver will receive
// file is opened using the filename
// when file is completly sent, event is set appropriately
// appropiate error is sent on cerver error or on bad file transmission
// returns 0 on success sending request, 1 on failed to send request
extern u8 client_file_send (Client *client, struct _Connection *connection, const char *filename);

/*** Game ***/

// requets the cerver to create a new lobby
// game type: is the type of game to create the lobby, the configuration must exist in the cerver
// returns 0 on success sending request, 1 on failed to send request
extern u8 client_game_create_lobby (Client *owner, struct _Connection *connection,
    const char *game_type);

// requests the cerver to join a lobby
// game type: is the type of game to create the lobby, the configuration must exist in the cerver
// lobby id: if you know the id of the lobby to join to, if not, the cerver witll search one for you
// returns 0 on success sending request, 1 on failed to send request
extern u8 client_game_join_lobby (Client *client, struct _Connection *connection,
    const char *game_type, const char *lobby_id);

// request the cerver to leave the currect lobby
// returns 0 on success sending request, 1 on failed to send request
extern u8 client_game_leave_lobby (Client *client, struct _Connection *connection,
    const char *lobby_id);

// requests the cerver to start the game in the current lobby
// returns 0 on success sending request, 1 on failed to send request
extern u8 client_game_start_lobby (Client *client, struct _Connection *connection,
    const char *lobby_id);

/*** aux ***/

typedef struct ClientConnection {

    Client *client;
    struct _Connection *connection;

} ClientConnection;

extern ClientConnection *client_connection_aux_new (Client *client, struct _Connection *connection);

extern void client_connection_aux_delete (void *ptr);

/*** Serialization ***/

// session id - token
struct _Token {

    char token[65];

};

typedef struct _Token Token;

#endif