#ifndef _CENGINE_INPUT_H_
#define _CENGINE_INPUT_H_

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/vector2d.h"

#include "cengine/ui/inputfield.h"

extern bool typing;

extern void input_set_active_text (InputField *text);

#define N_MOUSE_BUTTONS     3

typedef enum MouseButton {

    MOUSE_LEFT          = 0,
    MOUSE_MIDDLE        = 1,
    MOUSE_RIGHT         = 2

} MouseButton;

extern Vector2D mousePos;

extern bool input_get_mouse_button_state (MouseButton button);

extern bool input_is_key_down (const SDL_Scancode key);

extern void input_init (void);
extern void input_end (void);
extern void input_handle (SDL_Event event);

/*** user defined events ***/

// creates a new command with an action to be triggered by ctrl + key
extern u8 input_command_register (SDL_Keycode key, Action action, void *args);

// registers an action to be triggered whenever a key is pressed
extern void input_key_event_register (const SDL_Keycode key, Action action, void *args);

extern void input_key_event_unregister (const SDL_Keycode key);

#endif