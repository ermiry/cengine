#ifndef _CENGINE_INPUT_H_
#define _CENGINE_INPUT_H_

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/vector2d.h"

#include "cengine/config.h"
#include "cengine/events.h"

#include "cengine/ui/inputfield.h"
#include "cengine/ui/tooltip.h"

struct _EventActionData;

struct _Input {

    // current active input field
    bool typing;
    InputField *active_text;

    // 15/05/2020
    Tooltip *right_click_menu;
    EventAction *right_click_event;

    // custom user input method
    void (*user_input)(void *);

};

typedef struct _Input Input;

CENGINE_PRIVATE Input *input_new (void);

CENGINE_PRIVATE void input_delete (void *input_ptr);

CENGINE_PUBLIC void input_set_active_text (Input *input, InputField *text);

// sets the menu (tooltip) to be displayed in the current window
CENGINE_PUBLIC void input_set_right_click_menu (Input *input, Tooltip *right_click_menu);

/*** mouse ***/

#define N_MOUSE_BUTTONS     3

typedef enum MouseButton {

    MOUSE_LEFT          = 0,
    MOUSE_MIDDLE        = 1,
    MOUSE_RIGHT         = 2

} MouseButton;

CENGINE_PUBLIC Vector2D mousePos;

CENGINE_PUBLIC void mouse_pos_get (int *x, int *y);

CENGINE_PUBLIC bool input_get_mouse_button_state (MouseButton button);

// sets and action to be performed when the mouse scrolls up
// wants a reference to a positive integer referencing the amount scrolled
CENGINE_EXPORT void input_set_on_mouse_wheel_scroll_up (Action action);

// sets and action to be performed when the mouse scrolls down
// wants a reference to a negative integer referencing the amount scrolled
CENGINE_EXPORT void input_set_on_mouse_wheel_scroll_down (Action action);

// sets and action to be performed when the mouse scrolls right
// wants a reference to a positive integer referencing the amount scrolled
CENGINE_EXPORT void input_set_on_mouse_wheel_scroll_right (Action action);

// sets and action to be performed when the mouse scrolls left
// wants a reference to a negative integer referencing the amount scrolled
CENGINE_EXPORT void input_set_on_mouse_wheel_scroll_left (Action action);

/*** keyboard ***/

CENGINE_PUBLIC bool input_is_key_down (const SDL_Scancode key);

// creates a new command with an action to be triggered by ctrl + key
CENGINE_EXPORT u8 input_command_register (SDL_Keycode key, Action action, void *args);

CENGINE_EXPORT void input_command_unregister (SDL_Keycode key);

// registers an action to be triggered whenever a key is pressed
CENGINE_EXPORT void input_key_event_register (const SDL_Keycode key, Action action, void *args);

CENGINE_EXPORT void input_key_event_unregister (const SDL_Keycode key);

/*** main ***/

CENGINE_PRIVATE void input_init (void);

CENGINE_PRIVATE u8 input_end (void);

CENGINE_PRIVATE void input_handle (SDL_Event event);

#endif