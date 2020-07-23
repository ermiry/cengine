#ifndef _CLIENT_EVENTS_H_
#define _CLIENT_EVENTS_H_

#include <stdbool.h>

#include "client/types/types.h"

#include "client/client.h"

struct _Client;
struct _Connection;

typedef enum ClientEventType {

    CLIENT_EVENT_NONE                  = 0, 

    CLIENT_EVENT_CONNECTED,            // connected to cerver
    CLIENT_EVENT_DISCONNECTED,         // disconnected from the cerver, either by the cerver or by losing connection

    CLIENT_EVENT_CONNECTION_FAILED,    // failed to connect to cerver
    CLIENT_EVENT_CONNECTION_CLOSE,     // this happens when a call to a recv () methods returns <= 0, the connection is clossed directly by client

    CLIENT_EVENT_CONNECTION_DATA,      // data has been received, only triggered from client request methods

    CLIENT_EVENT_CERVER_INFO,          // received cerver info from the cerver
    CLIENT_EVENT_CERVER_TEARDOWN,      // the cerver is going to teardown (disconnect happens automatically)
    CLIENT_EVENT_CERVER_STATS,         // received cerver stats
    CLIENT_EVENT_CERVER_GAME_STATS,    // received cerver game stats

    CLIENT_EVENT_AUTH_SENT,            // auth data has been sent to the cerver
    CLIENT_EVENT_SUCCESS_AUTH,         // auth with cerver has been successfull
    CLIENT_EVENT_MAX_AUTH_TRIES,       // maxed out attempts to authenticate to cerver, so try again

    CLIENT_EVENT_LOBBY_CREATE,         // a new lobby was successfully created
    CLIENT_EVENT_LOBBY_JOIN,           // correctly joined a new lobby
    CLIENT_EVENT_LOBBY_LEAVE,          // successfully exited a lobby

    CLIENT_EVENT_LOBBY_START,          // the game in the lobby has started

} ClientEventType;

typedef struct ClientEvent {

    ClientEventType type;         // the event we are waiting to happen
    bool create_thread;                 // create a detachable thread to run action
    bool drop_after_trigger;            // if we only want to trigger the event once

    // the request that triggered the event
    // this is usefull for custom events
    u32 request_type; 
    void *response_data;                // data that came with the response   
    Action delete_response_data;       

    Action action;                      // the action to be triggered
    void *action_args;                  // the action arguments
    Action delete_action_args;          // how to get rid of the data

} ClientEvent;

// registers an action to be triggered when the specified event occurs
// if there is an existing action registered to an event, it will be overrided
// a newly allocated ClientEventData structure will be passed to your method 
// that should be free using the client_event_data_delete () method
// returns 0 on success, 1 on error
extern u8 client_event_register (struct _Client *client, const ClientEventType event_type, 
    Action action, void *action_args, Action delete_action_args, 
    bool create_thread, bool drop_after_trigger);

// unregister the action associated with an event
// deletes the action args using the delete_action_args () if NOT NULL
// returns 0 on success, 1 on error
extern u8 client_event_unregister (struct _Client *client, const ClientEventType event_type);

extern void client_event_set_response (struct _Client *client, const ClientEventType event_type,
    void *response_data, Action delete_response_data);

// triggers all the actions that are registred to an event
extern void client_event_trigger (const ClientEventType event_type,
    const struct _Client *client, const struct _Connection *connection);

#pragma region data

// structure that is passed to the user registered method
typedef struct ClientEventData {

    const struct _Client *client;
    const struct _Connection *connection;

    void *response_data;                // data that came with the response   
    Action delete_response_data;  

    void *action_args;                  // the action arguments
    Action delete_action_args;

} ClientEventData;

extern void client_event_data_delete (ClientEventData *event_data);

#pragma endregion

#pragma region main

extern u8 client_events_init (struct _Client *client);

extern void client_events_end (struct _Client *client);

#pragma endregion

#endif