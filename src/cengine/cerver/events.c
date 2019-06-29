#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"

#include "cengine/cerver/client.h"
#include "cengine/cerver/events.h"

#include "cengine/collections/dlist.h"

static ClientEvent *client_event_new (void) {

    ClientEvent *event = (ClientEvent *) malloc (sizeof (ClientEvent));
    if (event) {
        memset (event, 0, sizeof (ClientEvent));

        event->action = NULL;
        event->data = NULL;
        event->destroy_data = NULL;
    }

    return event;

}

static void client_event_delete (void *ptr) {

    if (ptr) {
        ClientEvent *event = (ClientEvent *) ptr;

        if (event->destroy_data) event->destroy_data (event->data);
        else free (event->data);

        free (event);
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

    if (client) dlist_destroy (client->registered_actions);

}

// unregister the action associated with an event
void client_event_unregister (Client *client, ClientEventType event_type) {

    if (client) {
        if (client->registered_actions) {
            ClientEvent *event = NULL;
            for (ListElement *le = dlist_start (client->registered_actions); le; le = le->next) {
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
    Action action, void *data, Action destroy_data) {

    if (client) {
        if (client->registered_actions) {
            ClientEvent *event = client_event_new ();
            if (event) {
                event->event_type = event_type;
                event->action = action;
                event->data = data;
                event->destroy_data = destroy_data;

                // serach if there is an action already registred for that event
                client_event_unregister (client, event_type);

                dlist_insert_after (client->registered_actions, 
                    dlist_end (client->registered_actions), event);
            }
        }
    }
    
}

// triggers all the actions that are registred to an event
void client_event_trigger (Client *client, ClientEventType event_type) {

    if (client) {
        if (client->registered_actions) {
            ClientEvent *event = NULL;
            for (ListElement *le = dlist_start (client->registered_actions); le; le = le->next) {
                event = (ClientEvent *) le->data;
                if (event->event_type == event_type) {
                    // trigger the action
                    if (event->action) 
                        event->action (event->data);
                    
                }
            }
        }
    }

}