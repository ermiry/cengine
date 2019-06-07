#ifndef _CENGINE_STATE_
#define _CENGINE_STATE_

typedef enum StateType {

    LOGO = 0,
    PRELOADING = 1, 
    MAIN_MENU = 2,
    IN_GAME = 3,
    GAME_OVER = 4,

    LOADING = 10,

} StateType;

typedef struct State {

    void (*onEnter)(void);
    void (*onExit)(void);

    void (*update)(void);

} State;

#endif