#ifndef _CERVER_CLIENT_H_
#define _CERVER_CLIENT_H_

#include <stdbool.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <poll.h>

#include "cengine/types/types.h"
#include "cengine/threads/thpool.h"
#include "cengine/cerver/events.h"
#include "cengine/cerver/errors.h"
#include "cengine/cerver/connection.h"
#include "cengine/collections/dlist.h"

#define MAX_UDP_PACKET_SIZE             65515

#define DEFAULT_MAX_CONNECTIONS         10
#define DEFAULT_POLL_TIMEOUT            2000
#define DEFAULT_PACKET_POOL_INIT        4

#define DEFAULT_THPOOL_INIT             4

struct _Client {

    DoubleList *connections;

    bool running;               // client poll is running

    struct pollfd fds[DEFAULT_MAX_CONNECTIONS];
    u16 n_fds;                  // n of active fds in the pollfd array
    u32 poll_timeout;   

    threadpool thpool;

    // only used in a game server
    bool in_lobby;               // is the client inside a lobby?
    bool owner;                  // is the client the owner of the lobby?

    // all the actions that have been registered to a client
    DoubleList *registered_actions;

};

typedef struct _Client Client;

// creates a new client, whcih may be used to create connections
extern Client *client_create (void);

// stops any activae connection and destroys a client
extern u8 client_teardown (Client *client);

extern void client_set_poll_timeout (Client *client, unsigned int timeout);

// returns a connection (registered to a client) by its name
extern Connection *client_connection_get_by_name (Client *client, const char *name);

// creates a new connection and registers it to the specified client;
// the connection should be ready to be started
// returns 0 on success, 1 on error
extern int client_connection_create (Client *client, const char *name,
    const char *ip_address, u16 port, u8 protocol, bool use_ipv6, bool async);

// registers an existing connection to a client
// retuns 0 on success, 1 on error
extern int client_connection_register (Client *client, Connection *connection);

// starts a client connection
// returns 0 on success, 1 on error
extern int client_connection_start (Client *client, Connection *connection);

// terminates and destroy a connection registered to a client
// returns 0 on success, 1 on error
extern int client_connection_end (Client *client, Connection *connection);

#pragma region GAME

// this is the same as in cerver
typedef enum GameType {

	ARCADE = 1,

} GameType;

typedef struct GameReqData {

    Client *client;
    Connection *connection;

    GameType game_type;

} GameReqData;

extern void *client_game_createLobby (Client *owner, Connection *connection, GameType gameType);
extern void *client_game_joinLobby (Client *client, Connection *connection, GameType gameType);
extern i8 client_game_leaveLobby (Client *client, Connection *connection);
extern i8 client_game_destroyLobby (Client *client, Connection *connection);

extern i8 client_game_startGame (Client *client, Connection *connection);

#pragma endregion

/*** SERIALIZATION ***/

// cerver framework serialized data
#pragma region SERIALIZATION

// session id - token
struct _Token {

    char token[65];

};

typedef struct _Token Token;

typedef struct GameSettings {

	GameType gameType;

	u8 playerTimeout; 	// in seconds.
	u8 fps;

	u8 minPlayers;
	u8 maxPlayers;

} GameSettings;

// info that we need to send to the client about the players
typedef struct Splayer {

	// TODO:
	// char name[64];

	// TODO: 
	// we need a way to add info about the players info for specific game
	// such as their race or level in blackrock

	bool owner;

} SPlayer;

// FIXME: players and a reference to the owner
// info that we need to send to the client about the lobby he is in
typedef struct SLobby {

    GameSettings settings;
    bool inGame;

    // FIXME: how do we want to send this info?
    // Player owner;               // how do we want to send which is the owner
    // Vector players;             // ecah client also needs to keep track of other players in the lobby

} SLobby;

typedef SLobby Lobby;

#pragma endregion

#endif