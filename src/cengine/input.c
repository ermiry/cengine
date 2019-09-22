#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/types/vector2d.h" 

#include "cengine/collections/dlist.h"

#include "cengine/cengine.h"
#include "cengine/input.h"

#include "cengine/ui/inputfield.h"
#include "cengine/ui/components/text.h"

bool typing = false;

static InputField *active_text = NULL;

void input_set_active_text (InputField *text) {

    active_text = text;
    typing = active_text ? true : false;

}

/*** Mouse ***/

Vector2D mousePos = { 0, 0 };

static bool mouse_button_states[N_MOUSE_BUTTONS];

bool input_get_mouse_button_state (MouseButton button) { return mouse_button_states[button]; }

static void input_on_mouse_button_down (SDL_Event event) {

    switch (event.button.button) {
        case SDL_BUTTON_LEFT: mouse_button_states[MOUSE_LEFT] = true; break;
        case SDL_BUTTON_MIDDLE: mouse_button_states[MOUSE_MIDDLE] = true; break;
        case SDL_BUTTON_RIGHT: mouse_button_states[MOUSE_RIGHT] = true; break;

        default: break;
    }

}

static void input_on_mouse_button_up (SDL_Event event) {

    switch (event.button.button) {
        case SDL_BUTTON_LEFT: mouse_button_states[MOUSE_LEFT] = false; break;
        case SDL_BUTTON_MIDDLE: mouse_button_states[MOUSE_MIDDLE] = false; break;
        case SDL_BUTTON_RIGHT: mouse_button_states[MOUSE_RIGHT] = false; break;

        default: break;
    }

}

/*** Keyboard ***/

// a custom action performed with a combination of ctl + key
typedef struct Command {

    SDL_Keycode key;
    Action action;
    void *args;

} Command;

static DoubleList *command_actions = NULL;

static Command *command_new (void) {

    Command *command = (Command *) malloc (sizeof (Command));
    if (command) {
        command->key = 0;
        command->action = NULL;
        command->args = NULL;
    }

    return command;

}

static void command_delete (void *command_ptr) {

    if (command_ptr) free (command_ptr);

}

// creates a new command with an action to be triggered by ctrl + key
u8 input_command_register (SDL_Keycode key, Action action, void *args) {

    u8 retval = 1;

    Command *command = command_new ();
    if (command) {
        command->key = key;
        command->action = action;
        command->args = args;

        dlist_insert_after (command_actions, dlist_end (command_actions), command);

        retval = 0;
    }

    return retval;

}

void input_command_unregister (SDL_Keycode key) {

    Command *command = NULL;
    for (ListElement *le = dlist_start (command_actions); le; le = le->next) {
        command = (Command *) le->data;
        if (key == command->key) {
            command_delete ((Command *) dlist_remove_element (command_actions, le));

            break;
        }
    }   

}

static const u8 *keys_states = NULL;

typedef struct KeyEvent {

    SDL_Keycode key;            // they key code this event refers to
    Action action;              // action to be triggered when key is pressed
    void *args;                 // arguments to be passed to the action

} KeyEvent;

static DoubleList *keys_events = NULL;

static KeyEvent *key_event_new (const SDL_Keycode key, Action action, void *args) {

    KeyEvent *key_event = (KeyEvent *) malloc (sizeof (KeyEvent));
    if (key_event) {
        key_event->key = (SDL_Keycode) key;
        key_event->action = action;
        key_event->args = args;
    }

    return key_event;

}

static void key_event_delete (void *key_event) {

    if (key_event) free (key_event);

}

// registers an action to be triggered whenever a key is pressed
void input_key_event_register (const SDL_Keycode key, Action action, void *args) {

    dlist_insert_after (keys_events, dlist_end (keys_events), key_event_new (key, action, args));

}

void input_key_event_unregister (const SDL_Keycode key) {

    KeyEvent *key_event = NULL;
    for (ListElement *le = dlist_start (keys_events); le; le = le->next) {
        key_event = (KeyEvent *) le->data;
        if (key == key_event->key) {
            key_event_delete ((KeyEvent *) dlist_remove_element (keys_events, le));
            break;
        }
    }

}

static void input_key_down (SDL_Event event) { 

    keys_states = SDL_GetKeyboardState (NULL); 
    
    // handle escape
    if (event.key.keysym.sym == SDLK_ESCAPE) {
        if (typing) {
            input_set_active_text (NULL);
        }
    }

    // handle backspace
    else if (event.key.keysym.sym == SDLK_BACKSPACE) {
        if (active_text && typing) {
            if (active_text->is_password) 
                str_remove_last_char (active_text->password);
            else
                str_remove_last_char (active_text->text->text);

            ui_input_field_update (active_text);
        }
    }

    // handle copy to clipboard
    else if (event.key.keysym.sym == SDLK_c && SDL_GetModState () & KMOD_CTRL) {
        if (active_text && typing) {
            if (active_text->is_password) SDL_SetClipboardText (active_text->password->str);
            else SDL_SetClipboardText (active_text->text->text->str);
        }
    }

    // handle paste from clipboard
    else if (event.key.keysym.sym == SDLK_v && SDL_GetModState () & KMOD_CTRL) {
        if (active_text && typing) {
            if (active_text->password)
                str_append_c_string (active_text->password, SDL_GetClipboardText ());
            else
                str_append_c_string (active_text->text->text, SDL_GetClipboardText ());
            
            ui_input_field_update (active_text);
        }
    }

    // handle any custom command + key action set by the user
    else if (SDL_GetModState () & KMOD_CTRL) {
        Command *command = NULL;
        for (ListElement *le = dlist_start (command_actions); le; le = le->next) {
            command = (Command *) le->data;
            if (event.key.keysym.sym == command->key) {
                if (command->action) {
                    command->action (command->args);
                }

                break;      // we only want one to happen each time
            }
        }   
    }

    // handle user defined key events
    else {
        KeyEvent *key_event = NULL;
        for (ListElement *le = dlist_start (keys_events); le; le = le->next) {
            key_event = (KeyEvent *) le->data;
            if (event.key.keysym.sym == key_event->key) {
                if (key_event->action) {
                    key_event->action (key_event->args);
                }

                break;      // we only want one to happen each time
            }
        }
    }

}

static void input_key_up (void) { keys_states = SDL_GetKeyboardState (NULL); }

bool input_is_key_down (const SDL_Scancode key) { 
    
    return (keys_states ? keys_states[key] : false);
    
}

/*** Input ***/

void input_init (void) {

    SDL_StartTextInput ();

    for (u8 i = 0; i < N_MOUSE_BUTTONS; i++) mouse_button_states[i] = false;

    command_actions = dlist_init (command_delete, NULL);

    keys_events = dlist_init (key_event_delete, NULL);

}

void input_end (void) {

    SDL_StopTextInput ();

    dlist_delete (command_actions);

    dlist_delete (keys_events);

}

static void input_handle_text_input (SDL_Event event) {

    // check if we are typing into something
    if (typing) {
        // check for copy or pasting
        if (!(SDL_GetModState () & KMOD_CTRL && (event.text.text[0] == 'c' || 
            event.text.text[0] == 'C' || event.text.text[0] == 'v' || 
            event.text.text[0] == 'V' ))) {
            if (active_text) {
                if (active_text->is_password) 
                    str_append_c_string (active_text->password, event.text.text);
                else 
                    str_append_c_string (active_text->text->text, event.text.text);

                ui_input_field_update (active_text);

                // printf ("%s\n", active_text->text->str);
            }
        }
    }

}

void input_handle (SDL_Event event) {

    while (SDL_PollEvent (&event)) {
        switch (event.type) {
            case SDL_QUIT: if (cengine_quit) cengine_quit (); break;

            case SDL_MOUSEMOTION: 
                mousePos.x = event.motion.x;
                mousePos.y = event.motion.y;
                break;

            case SDL_MOUSEBUTTONDOWN: input_on_mouse_button_down (event); break;
            case SDL_MOUSEBUTTONUP: input_on_mouse_button_up (event); break;

            case SDL_KEYDOWN: input_key_down (event); break;
            case SDL_KEYUP: input_key_up (); break;

            case SDL_TEXTINPUT: input_handle_text_input (event); break;

            default: break;
        }
    }

}