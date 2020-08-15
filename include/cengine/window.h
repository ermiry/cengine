#ifndef _CENGINE_WINDOW_H_
#define _CENGINE_WINDOW_H_

#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_surface.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/collections/dlist.h"

#include "cengine/config.h"
#include "cengine/input.h"
#include "cengine/renderer.h"
#include "cengine/video.h"

struct _Renderer;
struct _Input;

struct _Window {

    u32 id;

    int display_index;
    SDL_DisplayMode display_mode;

    SDL_Window *window;
    Uint32 window_flags;

    String *window_title;
    WindowSize window_size;
    WindowSize window_original_size;

    SDL_Rect screen_rect;

    bool fullscreen;
    bool borders;

    SDL_Surface *icon;

    // reference to the window renderer
    struct _Renderer *renderer;

    // events
    bool shown;
    bool mouse;
    bool keyboard;
    bool mini;

    // void (*input)(void *);
    struct _Input *input;

};

typedef struct _Window Window;

CENGINE_PUBLIC DoubleList *windows;

CENGINE_PRIVATE Window *window_to_remove;

CENGINE_PRIVATE void window_delete (void *window_ptr);

CENGINE_PUBLIC int window_comparator (const void *a, const void *b);

// creates a new window with the requested arguments
CENGINE_PUBLIC Window *window_create (const char *title, WindowSize window_size, Uint32 window_flags,
    int display_idx);

// gets window size into renderer data struct
CENGINE_PUBLIC int window_get_size (Window *window, WindowSize *window_size);

// toggles window full screen on and off
// fullscreen with a videomode change
CENGINE_PUBLIC void window_toggle_full_screen (Window *window);

// toggles window soft fullscreen
// first removes border from window, and the scales to max resolution
CENGINE_PUBLIC void window_toggle_fullscreen_soft (Window *window);

// toggles window borders on and off
CENGINE_PUBLIC void window_toggle_borders (Window *window);

// resizes the window asscoaited with a renderer
CENGINE_PUBLIC int window_resize (Window *window, u32 new_width, u32 new_height);

// sets the window's icon, the surface gets destroyed when the window is destroyed
CENGINE_PUBLIC void window_set_icon (Window *window, SDL_Surface *icon_surface);

// sets the window's custom use rinput method
CENGINE_PUBLIC void window_set_user_input (Window *window, void (*user_input)(void *));

// sets the focus to the window
CENGINE_PUBLIC void window_focus (Window *window);

// handle windows events
CENGINE_PUBLIC void windows_handle_events (SDL_Event event);

#endif