#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cengine/types/types.h"

#include "cengine/cerver/client.h"
#include "cengine/cerver/events.h"

#include "cengine/threads/thread.h"

#include "cengine/collections/dlist.h"

static ClientEventData *client_event_data_new (ClientEvent *event) {

    ClientEventData *event_data = (ClientEventData *) malloc (sizeof (ClientEventData));
    if (event_data) {
        event_data->response_data = event->response_data;
        event_data->delete_response_data = event->delete_response_data;
        event_data->action_args = event->action_args;
        event_data->delete_action_args = event->delete_action_args;
    }

    return event_data;

}

void client_event_data_delete (ClientEventData *event_data) {

    if (event_data) free (event_data);

}

static ClientEvent *client_event_new (void) {

    ClientEvent *event = (ClientEvent *) malloc (sizeof (ClientEvent));
    if (event) {
        memset (event, 0, sizeof (ClientEvent));

        event->response_data = NULL;
        event->delete_response_data = NULL;

        event->action = NULL;
        event->action_args = NULL;
        event->delete_action_args = NULL;
    }

    return event;

}

static void client_event_delete (void *ptr) {

    if (ptr) {
        ClientEvent *event = (ClientEvent *) ptr;

        if (event->response_data) {
            if (event->delete_response_data) 
                event->delete_response_data (event->response_data);
            else free (event->response_data);
        }
        
        if (event->action_args) {
            if (event->delete_action_args)
                event->delete_action_args (event->action_args);
            else free (event->action_args);
        }

        free (event);
    }

}

static void client_event_pop (DoubleList *list, ListElement *le) {

    if (le) {
        void *data = dlist_remove_element (list, le);
        if (data) free (data);
    }

}

u8 client_events_init (Client *client) {

    u8 retval = 1;

    if (client) {
        client->registered_actions = dlist_init (client_event_delete, NULL);
        retval = client->registered_actions ? 0 : 1;
    }

    return retval;

}

void client_events_end (Client *client) { 

    if (client) dlist_delete (client->registered_actions);

}

static ClientEvent *client_event_get (Client *client, ClientEventType event_type, 
    ListElement **le_ptr) {

    if (client) {
        if (client->registered_actions) {
            ClientEvent *event = NULL;
            for (ListElement *le = dlist_start (client->registered_actions); le; le = le->next) {
                event = (ClientEvent *) le->data;
                if (event->event_type == event_type) {
                    if (le_ptr) *le_ptr = le;
                    return event;
                } 
            }
        }
    }

    return NULL;

}

// unregister the action associated with an event
void client_event_unregister (Client *client, ClientEventType event_type) {

    if (client) {
        if (client->registered_actions) {
            ClientEvent *event = NULL;
            for (ListElement *le = dlist_start (client->registered_actions); le; le = le->next) {
                event = (ClientEvent *) le->data;
                if (event->event_type == event_type) {
                    client_event_delete (dlist_remove_element (client->registered_actions, le));
                    break;
                }
            }
        }
    }

}

// register to trigger an action when the specified event occurs
// if there is an existing action registered to an event, it will be overrided
void client_event_register (Client *client, ClientEventType event_type, 
    Action action, void *action_args, Action delete_action_args, 
    bool create_thread, bool drop_after_trigger) {

    if (client) {
        if (client->registered_actions) {
            ClientEvent *event = client_event_new ();
            if (event) {
                event->create_thread = create_thread;
                event->drop_after_trigger = drop_after_trigger;

                event->event_type = event_type;
                event->action = action;
                event->action_args = action_args;
                event->delete_action_args = delete_action_args;

                // search if there is an action already registred for that event
                client_event_unregister (client, event_type);

                dlist_insert_after (client->registered_actions, 
                    dlist_end (client->registered_actions), event);
            }
        }
    }
    
}

void client_event_set_response (Client *client, ClientEventType event_type,
    void *response_data, Action delete_response_data) {

    if (client) {
        ClientEvent *event = client_event_get (client, event_type, NULL);
        if (event) {
            event->response_data = response_data;
            event->delete_response_data = delete_response_data;
        }
    }

} 

// triggers all the actions that are registred to an event
void client_event_trigger (Client *client, ClientEventType event_type) {

    if (client) {
        ListElement *le = NULL;
        ClientEvent *event = client_event_get (client, event_type, &le);
        if (event) {
            // trigger the action
            if (event->action) {
                if (event->create_thread) {
                    thread_create_detachable ((void *(*)(void *)) event->action, 
                        client_event_data_new (event));
                }

                else {
                    event->action (client_event_data_new (event));
                }
                
                if (event->drop_after_trigger) client_event_pop (client->registered_actions, le);
            }
        }
    }

}