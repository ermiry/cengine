#include <stdlib.h>

#include <SDL2/SDL.h>

#include "cengine/animation.h"
#include "cengine/input.h"
#include "cengine/renderer.h"
#include "cengine/thread.h"
#include "cengine/game/go.h"
#include "cengine/manager/manager.h"
#include "cengine/utils/log.h"
#include "cengine/utils/utils.h"

static unsigned int fps_limit = 30;

unsigned int main_fps = 0;
unsigned int update_fps = 0;

// TODO: create a similar function to sdl init to pass what we want to init
int cengine_init (const char *window_name) {

    int errors = 0;

    if (!SDL_Init (SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
        errors = thread_hub_init_global ();
        errors = video_init_main (window_name);
        errors = animations_init ();
        errors = game_objects_init_all ();
        errors = ui_init ();
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

    game_object_destroy_all ();
    animations_end ();
    ui_destroy ();
    video_destroy_main ();
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