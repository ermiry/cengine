#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "client/types/types.h"
#include "client/types/string.h"

#include "client/collections/dlist.h"

#include "client/packets.h"
#include "client/events.h"
#include "client/game.h"

#include "client/utils/log.h"

static GameSettings *game_settings_new (void) {

    GameSettings *settings = (GameSettings *) malloc (sizeof (GameSettings));
    if (settings) {
        memset (settings, 0, sizeof (GameSettings));
        settings->game_type = NULL;
    }

    return settings;

}

static void game_settings_delete (GameSettings *settings) {

    if (settings) {
        str_delete (settings->game_type);
        free (settings);
    }

}

// FIXME: dlist
static Lobby *lobby_new (void) {

    Lobby *lobby = (Lobby *) malloc (sizeof (Lobby));
    if (lobby) {
        memset (lobby, 0, sizeof (Lobby));
        lobby->id = NULL;
        lobby->game_settings = game_settings_new ();
        lobby->players = dlist_init (NULL, NULL);
    }

    return lobby;

}

void lobby_delete (void *lobby_ptr) {

    if (lobby_ptr) {
        Lobby *lobby = (Lobby *) lobby_ptr;

        str_delete (lobby->id);
        game_settings_delete (lobby->game_settings);
        dlist_delete (lobby->players);

        free (lobby);
    }

}

static Lobby *lobby_deserialize (SLobby *slobby) {

    Lobby *lobby = NULL;

    if (slobby) {
        lobby = lobby_new ();
        
        lobby->id = str_new (slobby->id.string);
        lobby->creation_timestamp = slobby->creation_timestamp;

        lobby->running = slobby->running;
        lobby->in_game = slobby->in_game;

        lobby->game_settings->game_type = str_new (slobby->settings.game_type.string);
        lobby->game_settings->player_timeout = slobby->settings.player_timeout;
        lobby->game_settings->fps = slobby->settings.fps;
        lobby->game_settings->min_players = slobby->settings.min_players;
        lobby->game_settings->max_players = slobby->settings.max_players;
        lobby->game_settings->duration = slobby->settings.duration;

        lobby->n_players = slobby->n_players;
    }

    return lobby;

}

static void client_game_lobby_create (Packet *packet) {

    if (packet) {
        if (packet->data_size >= sizeof (RequestData) + sizeof (SLobby)) {
            // get the lobby data from the cerver
            char *end = (char *) packet->data;
            end += sizeof (RequestData);
            SLobby *slobby = (SLobby *) end;

            // deserialize the lobby
            Lobby *lobby = lobby_deserialize (slobby);

            // trigger the event
            client_event_set_response (packet->client, CLIENT_EVENT_LOBBY_CREATE, 
                lobby, lobby_delete);
            client_event_trigger (packet->client, packet->connection, CLIENT_EVENT_LOBBY_CREATE);
        }

        else {
            // TODO: trigger error, bad packet
            client_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, 
                "client_game_lobby_create () - packets to small to get a lobby!");
        }
    }

}

static void client_game_lobby_join (Packet *packet) {

    if (packet) {
        if (packet->data_size >= sizeof (RequestData) + sizeof (SLobby)) {
            // get the lobby data from the cerver
            char *end = (char *) packet->data;
            end += sizeof (RequestData);
            SLobby *slobby = (SLobby *) end;

            // deserialize the lobby
            Lobby *lobby = lobby_deserialize (slobby);

            // trigger the event
            client_event_set_response (packet->client, CLIENT_EVENT_LOBBY_JOIN, 
                lobby, lobby_delete);
            client_event_trigger (packet->client, packet->connection, CLIENT_EVENT_LOBBY_JOIN);
        }

        else {
            // TODO: trigger error, bad packet
            client_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, 
                "client_game_lobby_join () - packets to small to get a lobby!");
        }
    }

}

// TODO:
static void client_game_lobby_leave (Packet *packet) {

    if (packet) {
        if (packet->data_size >= sizeof (RequestData)) {
            // get the lobby data from the cerver

            // trigger the event
            client_event_trigger (packet->client, packet->connection, CLIENT_EVENT_LOBBY_JOIN);
        }
    }

}

static void client_game_lobby_start (Packet *packet) {

    if (packet) {
        // trigger the event
        client_event_trigger (packet->client, packet->connection, CLIENT_EVENT_LOBBY_START);
    }

}

// handles a game packet from the cerver
void client_game_packet_handler (Packet *packet) {

    if (packet) {
        if (packet->data && (packet->data_size >= sizeof (RequestData))) {
            RequestData *req = (RequestData *) (packet->data);
            switch (req->type) {
                case GAME_LOBBY_CREATE: client_game_lobby_create (packet); break;
                case GAME_LOBBY_JOIN: client_game_lobby_join (packet); break;
                case GAME_LOBBY_LEAVE: client_game_lobby_leave (packet); break;
                case GAME_LOBBY_UPDATE: break;
                case GAME_LOBBY_DESTROY: break;

                case GAME_INIT: break;
                case GAME_START: client_game_lobby_start (packet); break;

                default:
                    client_log_msg (stderr, LOG_WARNING, LOG_CLIENT,
                        "Got a game packet of unknown type!");
                    break;
            }
        }
    }

}