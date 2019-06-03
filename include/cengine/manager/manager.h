#ifndef _CENGINE_MANAGER_H_
#define _CENGINE_MANAGER_H_

#include "cengine/manager/state.h"

typedef enum SessionType {

    SINGLE_PLAYER = 0,
    MULTIPLAYER

} SessionType;

SessionType sessionType; 

typedef struct Manager {

    State *curr_state;

} Manager;

extern Manager *manager;

extern Manager *manager_new (State *init_state);
extern void manager_delete (Manager *manager);

extern State *manager_state_get_current (void);
extern void manger_state_change_state (State *new_state);

#endif