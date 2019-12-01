#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/collections/dlist.h"

#include "cengine/animation.h"
#include "cengine/assets.h"
#include "cengine/input.h"
#include "cengine/renderer.h"
#include "cengine/window.h"

#include "cengine/threads/thread.h"

#include "cengine/game/go.h"
#include "cengine/game/camera.h"
#include "cengine/manager/manager.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/textbox.h"

#include "cengine/utils/log.h"
#include "cengine/utils/utils.h"

int cengine_init (void) {

    int errors = 0;
    int retval = 0;

    srand ((unsigned) time (NULL));

    if (!SDL_Init (SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
        retval = thread_hub_init_global ();
        if (retval) cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to init cengine's threads!");
        errors |= retval;

        retval = animations_init ();
        if (retval) cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to init cengine's animations!");
        errors |= retval;

        retval = render_init ();
        if (retval) cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to init cengine's renderer!");
        errors |= retval;

        retval = game_objects_init_all ();
        if (retval) cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to init cengine's game objects!");
        errors |= retval;

        retval = ui_init ();
        if (retval) cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to init cengine's ui!");
        errors |= retval;

        input_init ();

        // main_camera = camera_new (main_renderer->window_size.width, main_renderer->window_size.height);
        // retval = main_camera ? 0 : 1;

        errors |= retval;
    }

    else {
        cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to init SDL!");
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, c_string_create ("%s", SDL_GetError ()));
        #endif
        errors = 1;
    }

    return errors;

}

int cengine_end (void) {

    manager->curr_state->on_exit ();
    manager_delete (manager);

    assets_end ();
    input_end ();
    camera_destroy (main_camera);
    ui_end ();
    render_end ();
    game_object_destroy_all ();
    animations_end ();
    thread_hub_end_global ();

    SDL_Quit ();

    return 0;

}

/*** threads ***/

bool running = true;

unsigned int fps_limit = 30;

unsigned int cengine_get_fps_limit (void) { return fps_limit; }

static unsigned int main_fps = 0;
static TextBox *main_fps_text = NULL;

void cengine_set_main_fps_text (TextBox *text) {

    main_fps_text = text;

}

static unsigned int update_fps = 0;
static char update_text[16] = { 0 };
static TextBox *update_fps_text = NULL;

void cengine_set_update_fps_text (TextBox *text) {

    update_fps_text = text;

}

static pthread_t update_thread;

static void *cengine_update (void *args) {

    thread_set_name ("update");

    u32 time_per_frame = 1000 / fps_limit;
    u32 frame_start = 0;
    i32 sleep_time = 0;

    float delta_time = 0;
    u32 delta_ticks = 0;
    update_fps = 0;

    while (running) {
        frame_start = SDL_GetTicks ();

        if (manager->curr_state->update)
            manager->curr_state->update ();

        // limit the FPS
        sleep_time = time_per_frame - (SDL_GetTicks () - frame_start);
        if (sleep_time > 0) SDL_Delay (sleep_time);

        // count fps
        delta_time = SDL_GetTicks () - frame_start;
        delta_ticks += delta_time;
        if (delta_ticks >= 1000) {
            // printf ("update fps: %i\n", update_fps);
            if (update_fps_text) {
                snprintf (update_text, 16, "update: %d", update_fps);
            }
            
            delta_ticks = 0;
            update_fps = 0;
        }

        else update_fps++;
    }
    
}

static void cengine_run (void) {

    SDL_Event event;

    float time_per_frame = 1000 / fps_limit;
    u32 frame_start = 0;
    i32 sleep_time = 0;

    float delta_time = 0;
    u32 delta_ticks = 0;
    main_fps = 0;

    while (running) {
        frame_start = SDL_GetTicks ();

        input_handle (event);

        // TODO: maybe create a new thread for every window
        // update input and renderer for each window
        Window *win = NULL;
        for (ListElement *le = dlist_start (windows); le; le = le->next) {
            win = (Window *) le->data;
            if (win->input) win->input (win);
            win->renderer->ui->ui_element_hover = NULL;
            render (win->renderer);

            if (win->renderer->update) win->renderer->update (win->renderer->update_args);
        }

        // limit the FPS
        // u32 ticks = (SDL_GetTicks () - frame_start);
        // printf ("ticks: %d\n", ticks);
        sleep_time = time_per_frame - (SDL_GetTicks () - frame_start);
        // printf ("sleep: %d\n", sleep_time);
        if (sleep_time > 0) SDL_Delay (sleep_time);

        // count fps
        delta_time = SDL_GetTicks () - frame_start;
        delta_ticks += delta_time;
        if (delta_ticks >= 1000) {
            // printf ("main fps: %i\n", main_fps);
            if (main_fps_text) {
                char *text = c_string_create ("main: %d", main_fps);
                if (text) {
                    // FIXME:
                    // ui_textbox_update_text (main_fps_text, text);
                    free (text);
                }   
            }

            if (update_fps_text) {
                // FIXME:
                // ui_textbox_update_text (update_fps_text, renderer, update_text);
            } 
            
            delta_ticks = 0;
            main_fps = 0;
        }

        else main_fps++;
    }

}

int cengine_start (int fps) {

    fps_limit = fps > 0 ? fps : 30;

    if (thread_create_detachable (cengine_update, NULL)) {
        cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to create update thread!");
        running = false;
    }

    cengine_run ();

    return 0;

}

/*** other ***/

void (*cengine_quit)(void) = NULL;

// sets the function to be executed on SDL_QUIT event
void cengine_set_quit (void (*quit)(void)) {

    cengine_quit = quit;

}