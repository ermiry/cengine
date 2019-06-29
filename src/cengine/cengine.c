#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/animation.h"
#include "cengine/input.h"
#include "cengine/renderer.h"
#include "cengine/threads/thread.h"
#include "cengine/game/go.h"
#include "cengine/game/camera.h"
#include "cengine/manager/manager.h"
#include "cengine/utils/log.h"
#include "cengine/utils/utils.h"

bool running = true;

unsigned int fps_limit = 30;

unsigned int main_fps = 0;
unsigned int update_fps = 0;

// TODO: create a similar function to sdl init to pass what we want to init
int cengine_init (const char *window_title, WindowSize window_size, bool full_screen) {

    int errors = 0;

    if (!SDL_Init (SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
        errors = thread_hub_init_global ();
        errors = animations_init ();
        errors = renderer_init_main (SDL_RENDERER_SOFTWARE | SDL_RENDERER_ACCELERATED, 
            window_title, window_size, full_screen);
        layers_init ();
        errors = game_objects_init_all ();
        errors = ui_init ();

        main_camera = camera_new (main_renderer->window_size.width, main_renderer->window_size.height);
        errors = main_camera ? 0 : 1;
    }

    else {
        cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to init SDL!");
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stderr, ERROR, NO_TYPE, c_string_create ("%s", SDL_GetError ()));
        #endif
        errors = 1;
    }

    return errors;

}

int cengine_end (void) {

    camera_destroy (main_camera);
    ui_destroy ();
    layers_end ();
    renderer_delete_main ();
    game_object_destroy_all ();
    animations_end ();
    thread_hub_end_global ();

    SDL_Quit ();

    return 0;

}

static pthread_t update_thread;

void *cengine_update (void *args) {

    thread_set_name ("update");

    u32 timePerFrame = 1000 / fps_limit;
    u32 frameStart = 0;
    i32 sleepTime = 0;

    float deltaTime = 0;
    u32 deltaTicks = 0;
    u32 fps = 0;

    while (running) {
        frameStart = SDL_GetTicks ();

        if (manager->curr_state->update)
            manager->curr_state->update ();

        // limit the FPS
        sleepTime = timePerFrame - (SDL_GetTicks () - frameStart);
        if (sleepTime > 0) SDL_Delay (sleepTime);

        // count fps
        deltaTime = SDL_GetTicks () - frameStart;
        deltaTicks += deltaTime;
        fps++;
        if (deltaTicks >= 1000) {
            // printf ("update fps: %i\n", fps);
            update_fps = fps;
            deltaTicks = 0;
            fps = 0;
        }
    }
    
}

static void cengine_run (void) {

    SDL_Event event;

    u32 timePerFrame = 1000 / fps_limit;
    u32 frameStart = 0;
    i32 sleepTime = 0;

    float deltaTime = 0;
    u32 deltaTicks = 0;
    u32 fps = 0;

    while (running) {
        frameStart = SDL_GetTicks ();

        input_handle (event);

        render ();

        // limit the FPS
        sleepTime = timePerFrame - (SDL_GetTicks () - frameStart);
        if (sleepTime > 0) SDL_Delay (sleepTime);

        // count fps
        deltaTime = SDL_GetTicks () - frameStart;
        deltaTicks += deltaTime;
        fps++;
        if (deltaTicks >= 1000) {
            #ifdef CENGINE_DEBUG
            printf ("main fps: %i\n", fps);
            #endif
            main_fps = fps;
            deltaTicks = 0;
            fps = 0;
        }
    }

}

int cengine_start (int fps) {

    fps_limit = fps > 0 ? fps : 30;

    if (thread_create_detachable (cengine_update, NULL, "update")) {
        cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to create update thread!");
        running = false;
    }

    cengine_run ();

    return 0;

}