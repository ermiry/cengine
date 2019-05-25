#include <stdbool.h>

#include <SDL2/SDL.h>

#include "blackrock.h"

#include "settings.h"

#include "cengine/renderer.h"

#include "game/game.h"
#include "ui/ui.h"

#ifdef DEV
#include "utils/myUtils.h"
#include "utils/log.h"
#endif

SDL_Window *main_window = NULL;
SDL_Renderer *main_renderer = NULL;

static SDL_DisplayMode displayMode;

WindowSize windowSize;

bool isFullscreen;

#pragma region RENDERER

// TODO: render by layers
void render (void) {

    SDL_RenderClear (main_renderer);

    // render current game screen
    if (game_manager->currState->render)
        game_manager->currState->render ();

    ui_render ();       // render ui elements

    SDL_RenderPresent (main_renderer);

}

/*** OLD WAY OF RENDERING ***/

/*** SCREEN ***/

// FIXME: move this to a separate file
// TODO: are we cleanning up the console and the screen??
// do we want that to happen?
// void renderScreen (SDL_Renderer *renderer, SDL_Texture *screen, UIScreen *scene) {

    // render the views from back to front for the current screen
    // UIView *v = NULL;
    // for (ListElement *e = dlist_start (scene->views); e != NULL; e = e->next) {
    //     v = (UIView *) LIST_DATA (e);
    //     clearConsole (v->console);
    //     v->render (v->console);
    //     SDL_UpdateTexture (screen, v->pixelRect, v->console->pixels, v->pixelRect->w * sizeof (u32));
    // }

    // SDL_RenderClear (renderer);
    // SDL_RenderCopy (renderer, screen, NULL, NULL);
    // SDL_RenderPresent (renderer);

// }

static int render_init_main (void) {

    int retval = 1;

    main_renderer = SDL_CreateRenderer (main_window, 0, SDL_RENDERER_SOFTWARE | SDL_RENDERER_ACCELERATED);
    if (main_renderer) {
        SDL_SetRenderDrawColor (main_renderer, 0, 0, 0, 255);
        SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "0");
        SDL_RenderSetLogicalSize (main_renderer, windowSize.width, windowSize.height);

        retval = 0;
    }
    
    return retval;

}

#pragma endregion

#pragma region WINDOW

void window_update_size (SDL_Window *window) {

    if (window) {
        SDL_GetWindowSize (main_window, &windowSize.width, &windowSize.height);
        #ifdef DEV
        logMsg (stdout, DEBUG_MSG, NO_TYPE, 
            createString ("Window size: %dx%dpx.", windowSize.width, windowSize.height));
        #endif
    }

}

void window_toggle_full_screen (SDL_Window *window) {

    if (window) {
        u32 fullscreenFlag = SDL_WINDOW_FULLSCREEN;
        isFullscreen = SDL_GetWindowFlags (window) & fullscreenFlag;
        SDL_SetWindowFullscreen (window, isFullscreen ? 0 : fullscreenFlag);
    }

}

void window_resize (SDL_Window *window, u32 newWidth, u32 newHeight) {

    if (window) {
        // check if we have a valid new size
        if (newWidth <= displayMode.w && newWidth > 0 &&
            newHeight <= displayMode.h && newHeight > 0) {
            SDL_SetWindowSize (window, newWidth, newHeight);
            window_update_size (window);
        }
    }

}

// FIXME: players with higher resolution have an advantage -> they see more of the world
// TODO: check SDL_GetCurrentDisplayMode
// TODO: get refresh rate -> do we need vsync?
// TODO: check for a prefernces saved file to get current screen size & if we are full screen
static int window_init_main (const char *title) {

    int retval = 1;

    SDL_GetCurrentDisplayMode (0, &displayMode); 
    #ifdef DEV
    logMsg (stdout, DEBUG_MSG, NO_TYPE, 
        createString ("Main display mode is %dx%dpx @ %dhz.", 
        displayMode.w, displayMode.h, displayMode.refresh_rate));
    #endif

    // creates a window of the size of the screen
    main_window = SDL_CreateWindow (title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        main_settings->resolution.width, main_settings->resolution.height,
        main_settings->window ? 0 : SDL_WINDOW_FULLSCREEN);

    if (main_window) {
        window_update_size (main_window);
        retval = 0;
    }

    return retval;

}

#pragma endregion

int video_init_main (const char *title) {

    int errors = 0;

    errors = window_init_main (title);
    errors = render_init_main ();

    return errors;

}

void video_destroy_main (void) {

    if (main_renderer) SDL_DestroyRenderer (main_renderer);
    if (main_window) SDL_DestroyWindow (main_window);

}