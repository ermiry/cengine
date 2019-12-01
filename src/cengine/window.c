#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_events.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/collections/dlist.h"

#include "cengine/renderer.h"
#include "cengine/window.h"
#include "cengine/video.h"

#ifdef CENGINE_DEBUG
#include "cengine/utils/utils.h"
#include "cengine/utils/log.h"
#endif

DoubleList *windows = NULL;
Window *window_to_remove = NULL;

int window_get_size (Window *window, WindowSize *window_size);
void window_toggle_borders (Window *window);

static Window *window_new (void) {

    Window *window = (Window *) malloc (sizeof (Window));
    if (window) {
        memset (window, 0, sizeof (Window));
        window->window = NULL;  
        window->window_title = NULL;

        window->icon = NULL;

        window->renderer = NULL;

        window->input = NULL;
    }

    return window;

}

void window_delete (void *window_ptr) {

    if (window_ptr) {
        Window *window = (Window *) window_ptr;
        if (window->window) SDL_DestroyWindow (window->window);
        str_delete (window->window_title);

        surface_delete (window->icon);

        free (window);
    }

}

int window_comparator (const void *a, const void *b) {

    if (a && b) {
        Window *win_a = (Window *) a;
        Window *win_b = (Window *) b;

        if (win_a->id < win_b->id) return -1;
        if (win_a->id == win_b->id) return 0;
        else return 1;
    }

    return -1;

}

// creates a new window with the requested arguments
Window *window_create (const char *title, WindowSize window_size, Uint32 window_flags,
    int display_idx) {

    Window *window = NULL;

    if (title) {
        window = window_new ();
        if (window) {
            window->display_index = display_idx;

            if (!video_get_display_mode (window->display_index, &window->display_mode)) {
                u32 width = window_size.width;
                u32 height = window_size.height;

                // cap to maximum display size
                if (width > window->display_mode.w) width = window->display_mode.w;
                if (height > window->display_mode.h) height = window->display_mode.h;

                // creates a window of the size of the screen
                window->window = SDL_CreateWindow (title,
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                    width, height, window_flags);

                window->id = SDL_GetWindowID (window->window);
                // printf ("id: %d\n", window->id);

                // sets the actual window size in the correct places
                window_get_size (window, &window->window_size);
                window_get_size (window, &window->window_original_size);
                SDL_Rect screen_rect = { .x = 0, .y = 0, 
                    .w = window->window_size.width, window->window_size.height };
                window->screen_rect = screen_rect;

                window->window_title = title ? str_new (title) : NULL;
                window->window_flags = window_flags;
                window->fullscreen = window_flags & SDL_WINDOW_FULLSCREEN;

                dlist_insert_after (windows, dlist_end (windows), window);
            }

            else {
                window_delete (window);
                window = NULL;
            }
        }
    }

    return window;

}

// gets window size into renderer data struct
int window_get_size (Window *window, WindowSize *window_size) {

    int retval = 1;

    if (window) {
        SDL_GetWindowSize (window->window, &window_size->width, &window_size->height);
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE, 
            c_string_create ("Window size: %dx%dpx.", window_size->width, window_size->height));
        #endif
        retval = 0;
    }

    return retval;

}


// toggles window full screen on and off
// fullscreen with a videomode change
void window_toggle_full_screen (Window *window) {

    if (window) {
        u32 new_width = 0;
        u32 new_height = 0;

        // return window to original size before the fullscreen
        if (window->fullscreen) {
            new_width = window->window_original_size.width;
            new_height = window->window_original_size.height;
        }

        else {
            new_width = window->display_mode.w;
            new_height = window->display_mode.h;
        }

        SDL_SetWindowSize (window->window, new_width, new_height);
        SDL_Rect viewport = { .x = 0, .y = 0, .w = new_width, .h = new_height };
        SDL_RenderSetViewport (window->renderer->renderer, &viewport);
        SDL_RenderSetLogicalSize (window->renderer->renderer, new_width, new_height);

        window->fullscreen = SDL_GetWindowFlags (window->window) & SDL_WINDOW_FULLSCREEN;
        SDL_SetWindowFullscreen (window->window, window->fullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
        window->fullscreen = SDL_GetWindowFlags (window->window) & SDL_WINDOW_FULLSCREEN;

        window_get_size (window, &window->window_size);
        SDL_Rect screen_rect = { .x = 0, .y = 0, 
            .w = window->window_size.width, window->window_size.height };
        window->screen_rect = screen_rect;

        // int w, h;
        // SDL_RenderGetLogicalSize (window->renderer->renderer, &w, & h);
        // SDL_RenderPresent (window->renderer->renderer);
        // printf ("logical size: %d - %d\n", w, h);

        ui_resize (window);
    }

}

// toggles window soft fullscreen
// first removes border from window, and the scales to max resolution
void window_toggle_fullscreen_soft (Window *window) {

    if (window) {
        u32 new_width = 0;
        u32 new_height = 0;

        // return window to original size before the fullscreen
        if (window->fullscreen) {
            new_width = window->window_original_size.width;
            new_height = window->window_original_size.height;
        }

        else {
            new_width = window->display_mode.w;
            new_height = window->display_mode.h;
        }

        window_toggle_borders (window);
        // SDL_MaximizeWindow (window->window);
        // SDL_SetWindowMaximumSize (window->window, &new_width, &new_height)
        SDL_SetWindowFullscreen (window->window, SDL_WINDOW_FULLSCREEN_DESKTOP);

        // SDL_SetWindowSize (window->window, new_width, new_height);
        SDL_Rect viewport = { .x = 0, .y = 0, .w = new_width, .h = new_height };
        SDL_RenderSetViewport (window->renderer->renderer, &viewport);
        SDL_RenderSetLogicalSize (window->renderer->renderer, new_width, new_height);

        window_get_size (window, &window->window_size);
        SDL_Rect screen_rect = { .x = 0, .y = 0, 
            .w = window->window_size.width, window->window_size.height };
        window->screen_rect = screen_rect;

        SDL_SetWindowPosition (window->window, 
            ((window->display_mode.w / 2) - (window->window_size.width / 2)),
            ((window->display_mode.h / 2) - (window->window_size.height / 2)));

        ui_resize (window);
    }

}

// toggles window borders on and off
void window_toggle_borders (Window *window) {

    if (window) {
        window->borders = SDL_GetWindowFlags (window->window) & SDL_WINDOW_BORDERLESS;
        SDL_SetWindowBordered (window->window, window->borders ? SDL_WINDOW_BORDERLESS : 0);
        window->borders = SDL_GetWindowFlags (window->window) & SDL_WINDOW_BORDERLESS;
    }

}

// resizes the window asscoaited with a renderer
int window_resize (Window *window, u32 new_width, u32 new_height) {

    int retval = 1;

    if (window) {
        // cap to maximum display size
        u32 width = new_width;
        u32 height = new_height;

        if (width > window->display_mode.w) width = window->display_mode.w;
        if (height > window->display_mode.h) height = window->display_mode.h;

        SDL_SetWindowSize (window->window, new_width, new_height);

        // sets the actual window size in the correct places
        window_get_size (window, &window->window_size);

        SDL_RenderSetLogicalSize (window->renderer->renderer, 
            window->window_size.width, window->window_size.height);

        retval = 0;
    }

    return retval;

}

// sets the window's icon, the surface gets destroyed when the window is destroyed
void window_set_icon (Window *window, SDL_Surface *icon_surface) {

    if (window && icon_surface) {
        window->icon = icon_surface;
        SDL_SetWindowIcon (window->window, window->icon);
    }

}

// sets the focus to the window
void window_focus (Window *window) {

    if (window) SDL_RaiseWindow (window->window);

}

// TODO: maybe close program completely when main window is closed
// handle windows events
void windows_handle_events (SDL_Event event) {

    Window *win = NULL;
    for (ListElement *le = dlist_start (windows); le; le = le->next) {
        win = (Window *) le->data;

        if (event.window.windowID == win->id) {
            switch (event.window.event) {
                // Window appeared
                case SDL_WINDOWEVENT_SHOWN:
                    win->shown = true;
                    // printf ("show!\n");
                    break;

                // Window disappeared
                case SDL_WINDOWEVENT_HIDDEN:
                    win->shown = false;
                    // printf ("hide!\n");
                    break;

                // TODO:
                // Get new dimensions and repaint
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    // mWidth = e.window.data1;
                    // mHeight = e.window.data2;
                    // SDL_RenderPresent (mRenderer);
                    break;

                // TODO:
                // Repaint on expose
                case SDL_WINDOWEVENT_EXPOSED:
                    // SDL_RenderPresent (win->renderer->renderer);
                    break;

                // Mouse enter
                case SDL_WINDOWEVENT_ENTER:
                    win->mouse = true;
                    break;

                // Mouse exit
                case SDL_WINDOWEVENT_LEAVE:
                    win->mouse = false;
                    break;

                // Keyboard focus gained
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    win->keyboard = true;
                    break;

                // Keyboard focus lost
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    win->keyboard = false;
                    break;

                // Window minimized
                case SDL_WINDOWEVENT_MINIMIZED:
                    win->mini = true;
                    break;

                // Window maximized
                case SDL_WINDOWEVENT_MAXIMIZED:
                    win->mini = false;
                    break;

                // Window restored
                case SDL_WINDOWEVENT_RESTORED:
                    win->mini = false;
                    break;

                // Hide on close
                case SDL_WINDOWEVENT_CLOSE:
                    // SDL_HideWindow (win->window);
                    window_to_remove = win;
                    // printf ("close!\n");
                    break;

                default: break;
            }
        }
    }

    if (window_to_remove) {
        // printf ("%s\n", (window_to_remove)->renderer->name->str);
        renderer_delete (dlist_remove (renderers, (window_to_remove)->renderer));
        window_delete (dlist_remove (windows, window_to_remove));
        window_to_remove = NULL;
    } 

}