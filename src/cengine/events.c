#include <stdlib.h>

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/events.h"

static DoubleList *cengine_events = NULL;

#pragma region event actions

static EventActionData *event_action_data_new (void *event_data, void *action_args) {

	EventActionData *event_action_data = (EventActionData *) malloc (sizeof (EventActionData));
	if (event_action_data) {
		event_action_data->event_data = event_data;
		event_action_data->action_args = action_args;
	}

	return event_action_data;	

}

static inline void event_action_data_delete (void *ptr) { if (ptr) free (ptr); }

static EventAction *event_action_new (void) {

	EventAction *event_action = (EventAction *) malloc (sizeof (EventAction));
	if (event_action) {
		event_action->action = NULL;
		event_action->args = NULL;
	}

	return event_action;

}

static void event_action_delete (void *ptr) { if (ptr) free (ptr); }

static EventAction *event_action_create (Action action, void *args) {

	EventAction *event_action = event_action_new ();
	if (event_action) {
		event_action->action = action;
		event_action->args = args;
	}

	return event_action;

}

#pragma endregion

static Event *event_new (void) {

	Event *e = (Event *) malloc (sizeof (Event));
	if (e) {
		e->type = 0;
		e->event_actions = NULL;
	}

	return e;

}

static void event_delete (void *e_ptr) {

	if (e_ptr) {
		Event *e = (Event *) e_ptr;

		dlist_delete (e->event_actions);

		free (e);
	}

}

static Event *event_create (CengineEventType type) {

	Event *e = event_new ();
	if (e) {
		e->type = type;
		e->event_actions = dlist_init (event_action_delete, NULL);
	}

	return e;

}

// register a new action to be triggered when an event takes place
// returns 0 on succes, 1 on error
u8 cengine_event_register (CengineEventType type, Action action, void *args) {

	u8 retval = 1;

	if (action) {
		EventAction *event_action = event_action_create (action, args);

		// register to the correct event
		Event *e = NULL;
		for (ListElement *le = dlist_start (cengine_events); le; le = le->next) {
			e = (Event *) le->data;

			if (e->type == type) {
				retval = dlist_insert_after (e->event_actions, dlist_end (e->event_actions), event_action);
				break;
			}
		}
	}

	return retval;

}

// called by internal cengine methods to trigger all the actions
// that have been registered to an event
void cengine_event_trigger (CengineEventType type, void *event_data) {

	// searh the event
	Event *e = NULL;
	for (ListElement *le = dlist_start (cengine_events); le; le = le->next) {
		e = (Event *) le->data;

		if (e->type == type) {
			EventAction *event_action = NULL;
			for (ListElement *le_sub = dlist_start (e->event_actions); le_sub; le_sub = le_sub->next) {
				event_action = (EventAction *) le_sub->data;
				if (event_action->action) {
					EventActionData *event_action_data = event_action_data_new (event_data, event_action->args);
					event_action->action (event_action_data);
					event_action_data_delete (event_action_data);
				}
			}

			break;
		}
	}

}

#pragma region main

u8 cengine_events_init (void) {

	u8 errors = 0;
	u8 retval = 0;

	cengine_events = dlist_init (event_delete, NULL);
	retval = cengine_events ? 0 : 1;
	errors |= retval;

	errors |= dlist_insert_after (cengine_events, dlist_end (cengine_events), event_create (CENGINE_EVENT_SCROLL_UP));
	errors |= dlist_insert_after (cengine_events, dlist_end (cengine_events), event_create (CENGINE_EVENT_SCROLL_DOWN));

	return errors;

}

u8 cengine_events_end (void) {

	dlist_delete (cengine_events);

	return 0;

}

#pragma endregion