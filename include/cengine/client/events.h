#ifndef _CLIENT_EVENTS_H_
#define _CLIENT_EVENTS_H_

#include <stdbool.h>

#include "client/types/types.h"

#include "client/client.h"

struct _Client;

typedef enum ClientEventType {

    EVENT_CONNECTED,            // connected to cerver
    EVENT_DISCONNECTED,         // disconnected from the cerver, either by the cerver or by losing connection

    EVENT_CONNECTION_FAILED,    // failed to connect to cerver
    EVENT_CONNECTION_CLOSE,     // this happens when a call to a recv () methods returns <= 0, the connection is clossed directly by client

    EVENT_CONNECTION_DATA,      // data has been received, only triggered from client request methods

    EVENT_CERVER_INFO,          // received cerver info from the cerver
    EVENT_CERVER_TEARDOWN,      // the cerver is going to teardown (disconnect happens automatically)
    EVENT_CERVER_STATS,         // received cerver stats
    EVENT_CERVER_GAME_STATS,    // received cerver game stats

    EVENT_SUCCESS_AUTH,         // auth with cerver has been successfull
    EVENT_FAILED_AUTH,          // failed to authenticate to cerver
    EVENT_MAX_AUTH_TRIES,       // maxed out attempts to authenticate to cerver, so try again

    EVENT_LOBBY_CREATE,         // a new lobby was successfully created
    EVENT_LOBBY_JOIN,           // correctly joined a new lobby
    EVENT_LOBBY_LEAVE,          // successfully exited a lobby

    EVENT_LOBBY_START,          // the game in the lobby has started

} ClientEventType;

typedef struct ClientEvent {

    ClientEventType event_type;         // the event we are waiting to happen
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

extern u8 client_events_init (struct _Client *client);

extern void client_events_end (struct _Client *client);

// register to trigger an action when the specified event occurs
// if there is an existing action registered to an event, it will be overrided
extern void client_event_register (struct _Client *client, ClientEventType event_type, 
    Action action, void *action_args, Action delete_action_args, 
    bool create_thread, bool drop_after_trigger);

extern void client_event_set_response (struct _Client *client, ClientEventType event_type,
    void *response_data, Action delete_response_data);

// unregister the action associated with an event
extern void client_event_unregister (struct _Client *client, ClientEventType event_type);

// triggers all the actions that are registred to an event
extern void client_event_trigger (struct _Client *client, ClientEventType event_type);

typedef struct ClientEventData {

    void *response_data;                // data that came with the response   
    Action delete_response_data;  

    void *action_args;                  // the action arguments
    Action delete_action_args;

} ClientEventData;

extern void client_event_data_delete (ClientEventData *event_data);

#endif