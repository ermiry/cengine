#ifndef _CENGINE_MANAGER_H_
#define _CENGINE_MANAGER_H_

#include "cengine/config.h"

#include "cengine/manager/state.h"

typedef enum SessionType {

    SINGLE_PLAYER = 0,
    MULTIPLAYER

} SessionType;

CENGINE_PUBLIC SessionType sessionType; 

typedef struct Manager {

    State *curr_state;

} Manager;

CENGINE_PUBLIC Manager *manager;

CENGINE_PUBLIC Manager *manager_new (State *init_state);

CENGINE_PUBLIC void manager_delete (Manager *manager);

CENGINE_PUBLIC State *manager_state_get_current (void);

CENGINE_PUBLIC void manager_state_change_state (State *new_state);

#endif