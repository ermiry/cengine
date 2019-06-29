#ifndef _CERVER_CLIENT_EVENTS_H_
#define _CERVER_CLIENT_EVENTS_H_

#include "cengine/types/types.h"

struct _Client;

typedef enum ClientEventType {

    EVENT_SUCCESS_AUTH,         // auth with cerver has been successfull

    EVENT_LOBBY_CREATE,         // a new lobby was successfully created
    EVENT_LOBBY_JOIN,           // correctly joined a new lobby
    EVENT_LOBBY_LEAVE,          // successfully exited a lobby

} ClientEventType;

typedef struct ClientEvent {

    ClientEventType event_type;     // the event we are waiting to happen

    Action action;                 // the action to be triggered
    void *data;                     // the action arguments
    Action destroy_data;           // how to get rid of the data

} ClientEvent;

extern u8 client_events_init (struct _Client *client);
extern void client_events_end (struct _Client *client);

// register to trigger an action when the specified event occurs
// if there is an existing action registered to an event, it will be overrided
extern void client_event_register (struct _Client *client, ClientEventType event_type, 
    Action action, void *data, Action destroy_data);

// unregister the action associated with an event
extern void client_event_unregister (struct _Client *client, ClientEventType event_type);

// triggers all the actions that are registred to an event
extern void client_event_trigger (struct _Client *client, ClientEventType event_type);

#endif