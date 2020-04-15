#ifndef _CENGINE_INPUT_H_
#define _CENGINE_INPUT_H_

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/vector2d.h"

#include "cengine/ui/inputfield.h"

struct _Input {

    // current active input field
    bool typing;
    InputField *active_text;

    // custom user input method
    void (*user_input)(void *);

};

typedef struct _Input Input;

extern Input *input_new (void);

extern void input_delete (void *input_ptr);

extern void input_set_active_text (Input *input, InputField *text);

#define N_MOUSE_BUTTONS     3

typedef enum MouseButton {

    MOUSE_LEFT          = 0,
    MOUSE_MIDDLE        = 1,
    MOUSE_RIGHT         = 2

} MouseButton;

extern Vector2D mousePos;

extern bool input_get_mouse_button_state (MouseButton button);

// sets and action to be performed when the mouse scrolls up
// wants a reference to a positive integer referencing the amount scrolled
extern void input_set_on_mouse_wheel_scroll_up (Action action);

// sets and action to be performed when the mouse scrolls down
// wants a reference to a negative integer referencing the amount scrolled
extern void input_set_on_mouse_wheel_scroll_down (Action action);

// sets and action to be performed when the mouse scrolls right
// wants a reference to a positive integer referencing the amount scrolled
extern void input_set_on_mouse_wheel_scroll_right (Action action);

// sets and action to be performed when the mouse scrolls left
// wants a reference to a negative integer referencing the amount scrolled
extern void input_set_on_mouse_wheel_scroll_left (Action action);

extern bool input_is_key_down (const SDL_Scancode key);

extern void input_init (void);

extern u8 input_end (void);

extern void input_handle (SDL_Event event);

/*** user defined events ***/

// creates a new command with an action to be triggered by ctrl + key
extern u8 input_command_register (SDL_Keycode key, Action action, void *args);

extern void input_command_unregister (SDL_Keycode key);

// registers an action to be triggered whenever a key is pressed
extern void input_key_event_register (const SDL_Keycode key, Action action, void *args);

extern void input_key_event_unregister (const SDL_Keycode key);

#endif