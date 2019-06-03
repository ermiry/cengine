#ifndef _CENGINE_INPUT_H_
#define _CENGINE_INPUT_H_

#include <SDL2/SDL.h>

#include "ui/ui.h"

#include "cengine/types/vector2d.h"

extern bool typing;

extern void input_start_typing (void);
extern void input_stop_typing (void);

/*** NEW INPUT ***/

#define N_MOUSE_BUTTONS     3

typedef enum MouseButton {

    MOUSE_LEFT = 0,
    MOUSE_MIDDLE = 1,
    MOUSE_RIGHT = 2

} MouseButton;

extern Vector2D mousePos;

extern bool input_get_mouse_button_state (MouseButton button);

extern bool input_is_key_down (const SDL_Scancode key);

extern void input_init (void);
extern void input_handle (SDL_Event event);

#endif