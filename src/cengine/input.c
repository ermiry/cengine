#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/input.h"
#include "cengine/types/vector2d.h" 

extern void quit (void);

/*** Mouse ***/

Vector2D mousePos = { 0, 0 };

bool mouseButtonStates[N_MOUSE_BUTTONS];

bool input_get_mouse_button_state (MouseButton button) { return mouseButtonStates[button]; }

void input_on_mouse_button_down (SDL_Event event) {

    switch (event.button.button) {
        case SDL_BUTTON_LEFT: mouseButtonStates[MOUSE_LEFT] = true; break;
        case SDL_BUTTON_MIDDLE: mouseButtonStates[MOUSE_MIDDLE] = true; break;
        case SDL_BUTTON_RIGHT: mouseButtonStates[MOUSE_RIGHT] = true; break;

        default: break;
    }

}

void input_on_mouse_button_up (SDL_Event event) {

    switch (event.button.button) {
        case SDL_BUTTON_LEFT: mouseButtonStates[MOUSE_LEFT] = false; break;
        case SDL_BUTTON_MIDDLE: mouseButtonStates[MOUSE_MIDDLE] = false; break;
        case SDL_BUTTON_RIGHT: mouseButtonStates[MOUSE_RIGHT] = false; break;

        default: break;
    }

}

/*** KEYBOARD ***/

const u8 *keys_states = NULL;

void input_key_down (void) { keys_states = SDL_GetKeyboardState (NULL); }

void input_key_up (void) { keys_states = SDL_GetKeyboardState (NULL); }

bool input_is_key_down (const SDL_Scancode key) { 
    
    if (keys_states) if (keys_states[key]) return true;
    return false;
    
}

/*** INPUT ***/

void input_init (void) {

    for (u8 i = 0; i < N_MOUSE_BUTTONS; i++) mouseButtonStates[i] = false;

}

void input_handle (SDL_Event event) {

    while (SDL_PollEvent (&event)) {
        switch (event.type) {
            case SDL_QUIT: quit (); break;

            case SDL_MOUSEMOTION: 
                mousePos.x = event.motion.x;
                mousePos.y = event.motion.y;
                break;

            case SDL_MOUSEBUTTONDOWN: input_on_mouse_button_down (event); break;
            case SDL_MOUSEBUTTONUP: input_on_mouse_button_up (event); break;

            case SDL_KEYDOWN: input_key_down (); break;
            case SDL_KEYUP: input_key_up (); break;

            default: break;
        }
    }

}