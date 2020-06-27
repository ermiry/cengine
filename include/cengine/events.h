#ifndef _CENGINE_EVENTS_H_
#define _CENGINE_EVENTS_H_

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

typedef enum CengineEventType {

	CENGINE_EVENT_NONE					= 0,

	CENGINE_EVENT_SCROLL_UP				= 1,
	CENGINE_EVENT_SCROLL_DOWN			= 2,

	CENGINE_EVENT_MOUSE_LEFT_UP			= 3,
	CENGINE_EVENT_MOUSE_MIDDLE_UP		= 4,
	CENGINE_EVENT_MOUSE_RIGHT_UP		= 5,

} CengineEventType;

// aux structure to pass the individual event data + the action args
struct _EventActionData {

	void *event_data;
	void *action_args;

};

typedef struct _EventActionData EventActionData;

struct _EventAction {

	CengineEventType type;
	int id;

	Action action;
    void *args;

};

typedef struct _EventAction EventAction;

struct _Event {

	CengineEventType type;
	DoubleList *event_actions;

};

typedef struct _Event Event;

// register a new action to be triggered when an event takes place
extern EventAction *cengine_event_register (CengineEventType type, Action action, void *args);

// unregister from an event
// returns 0 on succes, 1 on error
extern u8 cengine_event_unregister (EventAction *event_action);

// called by internal cengine methods to trigger all the actions
// that have been registered to an event
extern void cengine_event_trigger (CengineEventType type, void *event_data);

/*** main ***/

extern u8 cengine_events_init (void);

extern u8 cengine_events_end (void);

#endif