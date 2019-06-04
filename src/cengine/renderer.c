#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/renderer.h"
#include "cengine/manager/manager.h"
#include "cengine/ui/ui.h"
#include "cengine/utils/utils.h"
#include "cengine/utils/log.h"

#pragma region Window

// gets window size into renderer data struct
int window_get_size (SDL_Window *window, WindowSize *window_size) {

    int retval = 1;

    if (window) {
        SDL_GetWindowSize (window, &window_size->width, &window_size->height);
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE, 
            c_string_create ("Window size: %dx%dpx.", window_size->width, window_size->height));
        #endif
        retval = 0;
    }

    return retval;

}

// toggle full screen on and off
int window_toggle_full_screen (Renderer *renderer) {

    int retval = 1;

    if (renderer) {
        renderer->full_screen = SDL_GetWindowFlags (renderer->window) & SDL_WINDOW_FULLSCREEN;
        retval = SDL_SetWindowFullscreen (renderer->window, renderer->full_screen ? 0 : SDL_WINDOW_FULLSCREEN);
        renderer->full_screen = SDL_GetWindowFlags (renderer->window) & SDL_WINDOW_FULLSCREEN;
    }

    return retval;

}

// FIXME: do we need to update the renderer?
// resizes the window asscoaited with a renderer
int window_resize (Renderer *renderer, u32 new_width, u32 new_height) {

    int retval = 1;

    if (renderer) {
        // check if we have a valid new size
        if (new_width <= renderer->display_mode.w && new_width > 0 &&
            new_height <= renderer->display_mode.h && new_height > 0) {
            SDL_SetWindowSize (renderer->window, new_width, new_height);
            window_get_size (renderer->window, &renderer->window_size);
            retval = 0;
        }
    }

    return retval;

}

static SDL_Window *window_create (const char *title, WindowSize window_size, bool full_screen) {

    SDL_Window *window = NULL;

    // creates a window of the size of the screen
    window = SDL_CreateWindow (title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        window_size.width, window_size.height,
        full_screen ? SDL_WINDOW_FULLSCREEN : 0);

    return window;

}

#pragma endregion

#pragma region Renderer

// TODO: as of 03/06/2019 we only have support for one renderer, the main one
// the plan is to have as many as you want in order to support multiple windows 
Renderer *main_renderer = NULL;

static Renderer *renderer_new (void) {

    Renderer *renderer = (Renderer *) malloc (sizeof (Renderer));
    if (renderer) {
        memset (renderer, 0, sizeof (Renderer));
        renderer->name = NULL;
        renderer->window_title = NULL;
        renderer->window = NULL;
        renderer->renderer = NULL;
    }

    return renderer;

}

static void *renderer_delete (void *ptr) {

    if (ptr) {
        Renderer *renderer = (Renderer *) ptr;

        str_delete (renderer->name);
        str_delete (renderer->window_title);
        if (renderer->window) SDL_DestroyWindow (renderer->window);
        if (renderer->renderer) SDL_DestroyRenderer (renderer->renderer);

        free (renderer);
    }

}

// FIXME: players with higher resolution have an advantage -> they see more of the world
// TODO: check SDL_GetCurrentDisplayMode
// TODO: get refresh rate -> do we need vsync?
Renderer *render_create_renderer (const char *renderer_name, Uint32 flags, int display_index,
    const char *window_title, WindowSize window_size, bool full_screen) {

    Renderer *renderer = renderer_new ();

    renderer->display_index = display_index;
    if (!SDL_GetCurrentDisplayMode (display_index, &renderer->display_mode)) {
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stdout, DEBUG_MSG, NO_TYPE,
            c_string_create ("Display with idx %i mode is %dx%dpx @ %dhz.",
            renderer->display_index, 
            renderer->display_mode.w, renderer->display_mode.h, 
            renderer->display_mode.refresh_rate))
        #endif

        // first init the window
        renderer->window = window_create (window_title, window_size, full_screen);
        if (renderer->window) {
            window_get_size (renderer->window, &window_size);

            // init the sdl renderer
            // SDL_CreateRenderer (main_window, 0, SDL_RENDERER_SOFTWARE | SDL_RENDERER_ACCELERATED);
            renderer->renderer = SDL_CreateRenderer (renderer->window, 0, flags);
            if (renderer->renderer) {
                SDL_SetRenderDrawColor (renderer->renderer, 0, 0, 0, 255);
                SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "0");
                SDL_RenderSetLogicalSize (renderer->renderer, 
                    renderer->window_size.width, renderer->window_size.height);

                renderer->name = str_new (renderer_name);
                renderer->flags = flags;
                renderer->window_title = str_new (window_title);
                renderer->full_screen = full_screen;
            }

            else {
                cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to create renderer!"); 
                renderer_delete (renderer);
                renderer = NULL;
            }
        }

        else {
            cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to create window!"); 
            renderer_delete (renderer);
            renderer = NULL;
        }
    }

    else {
        cengine_log_msg (stderr, ERROR, NO_TYPE, 
            c_string_create ("Failed to get display mode for display with idx %i", display_index));
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stderr, ERROR, NO_TYPE, SDL_GetError ());
        #endif
        renderer_delete (renderer);
        renderer = NULL;
    }

    return renderer;

}

// FIXME:!!!
// TODO: render by layers
void render (void) {

    SDL_RenderClear (main_renderer->renderer);

    // FIXME: we dont want the user to be resposible of this!!
    // render current game screen
    // if (manager->curr_state->render)
    //     manager->curr_state->render ();

    ui_render ();       // render ui elements

    SDL_RenderPresent (main_renderer->renderer);

}

int renderer_init_main (Uint32 flags,
    const char *window_title, WindowSize window_size, bool full_screen) {

    return ((main_renderer = render_create_renderer ("main", flags, 0, 
        window_title, window_size, full_screen)) ? 0 : 1);

}

void renderer_delete_main (void) { renderer_delete (main_renderer); }

#pragma endregion