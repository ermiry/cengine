#ifndef _CERVER_CLIENT_GAME_H_
#define _CERVER_CLIENT_GAME_H_

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/collections/dlist.h"

#include "cengine/cerver/packets.h"

typedef struct GameSettings {

    // config
    String *game_type;
    u8 player_timeout;
	u8 fps;

	// rules
	u8 min_players;
	u8 max_players;
	int duration;

} GameSettings;

typedef struct Lobby {

    // lobby info
    String *id;
    time_t creation_timestamp;

    // lobby status
    bool running;
    bool in_game;

    // game / players
    GameSettings *game_settings;
    unsigned int n_players;
    // TODO: owner
    DoubleList *players;

} Lobby;

extern void lobby_delete (void *lobby_ptr);

// handles a game packet from the cerver
extern void client_game_packet_handler (Packet *packet);

/*** serialization ***/

typedef struct SGameSettings {

	// config
	SStringS game_type;
	u8 player_timeout;
	u8 fps;

	// rules
	u8 min_players;
	u8 max_players;
	int duration;

} SGameSettings;

typedef struct SLobby {

	// lobby info
	SStringS id;
	time_t creation_timestamp;

	// lobby status
	bool running;
	bool in_game;

	// game / players
	SGameSettings settings;
	unsigned int n_players;
	// TODO: owner
	
} SLobby;


typedef struct LobbyJoin {

    SStringS lobby_id;
    SStringS game_type;

} LobbyJoin;

#endif