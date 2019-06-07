#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/renderer.h"
#include "cengine/textures.h"
#include "cengine/manager/manager.h"
#include "cengine/game/go.h"
#include "cengine/game/camera.h"
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
            renderer->display_mode.refresh_rate));
        #endif

        // first init the window
        renderer->window = window_create (window_title, window_size, full_screen);
        if (renderer->window) {
            window_get_size (renderer->window, &renderer->window_size);

            // init the sdl renderer
            // SDL_CreateRenderer (main_window, 0, SDL_RENDERER_SOFTWARE | SDL_RENDERER_ACCELERATED);
            renderer->renderer = SDL_CreateRenderer (renderer->window, display_index, flags);
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

int renderer_init_main (Uint32 flags,
    const char *window_title, WindowSize window_size, bool full_screen) {

    return ((main_renderer = render_create_renderer ("main", flags, 0, 
        window_title, window_size, full_screen)) ? 0 : 1);

}

void renderer_delete_main (void) { renderer_delete (main_renderer); }

#pragma endregion

/*** Layers ***/

#pragma region Layers

static DoubleList *layers = NULL;
static u8 layer_pos = 0;

static Layer *layer_get_by_pos (const int pos) {

    Layer *layer = NULL;

    if (layers) {
        for (ListElement *le = dlist_start (layers); le; le = le->next) {
            layer = (Layer *) le->data;
            if (layer->pos == pos) return layer;
        }
    }

    return layer;

}

static Layer *layer_get_by_name (const char *name) {

    Layer *layer = NULL;

    if (name && layers) {
        for (ListElement *le = dlist_start (layers); le; le = le->next) {
            layer = (Layer *) le->data;
            if (!strcmp (layer->name->str, name)) return layer;
        }
    }

    return layer;

}

static Layer *layer_new (const char *name, int pos) {

    Layer *layer = (Layer *) malloc (sizeof (Layer));
    if (layer) {
        if (name) layer->name = str_new (name);
        else layer->name = NULL;

        layer->gos = dlist_init (game_object_destroy_dummy, game_object_comparator);

        if (pos >= 0) {
            layer->pos = pos;

            // check for a layer with that pos
            Layer *l = layer_get_by_pos (pos);
            if (l) {
                pos += 1;
                l->pos = pos;

                // we need to update the other layers pos
                Layer *update_layer = NULL;
                do {
                    update_layer = layer_get_by_pos (pos);
                    update_layer->pos = pos;
                    pos += 1;
                } while (update_layer);
            }
        }

        else {
            layer->pos = layer_pos;
            layer_pos++;
        }
    }

    return layer;

}

static void layer_delete (void *ptr) {

    if (ptr) {
        Layer *layer = (Layer *) ptr;
        str_delete (layer->name);
        dlist_destroy (layer->gos);

        free (layer);
    }

}

static int layer_comparator (void *one, void *two) {

    if (one && two) {
        Layer *layer_one = (Layer *) one;
        Layer *layer_two = (Layer *) two;

        if (layer_one->pos < layer_two->pos) return -1;
        else if (layer_one->pos == layer_two->pos) return 0;
        else return 1;
    }

}

// creates a new layer; 
// takes the layer name and the layer pos, -1 for last layer
// pos 0 renders first
// returns 0 on success, 1 on error
int layer_create (const char *name, int pos) {

    int retval = 1;

    if (name) {
        Layer *l = layer_new (name, pos);
        dlist_insert_after (layers, dlist_end (layers), l);

        // after we have inserted a new layer, we want to sort the layers
        // to render in the correct order
        retval = dlist_sort (layers);
    }

    return retval;

}

// add a game object into a layer
// returns 0 on succes, 1 on error
int layer_add_object (const char *layer_name, void *ptr) {

    int retval = 1;

    if (layer_name && ptr) {
        Layer *layer = layer_get_by_name (layer_name);
        if (layer) {
            dlist_insert_after (layer->gos, dlist_end (layer->gos), ptr);
            retval = 0;
        }
    }

    return retval;

}

// removes a game object from a layer
// returns 0 on succes, 1 on error
int layer_remove_object (const char *layer_name, void *ptr) {

    int retval = 0;

    if (layer_name && ptr) {
        Layer *layer = layer_get_by_name (layer_name);
        if (layer) {
            void *obj = dlist_remove_element (layer->gos, dlist_get_element (layer->gos, ptr));
            retval = obj ? 0 : 1;
        }
    }

    return retval;

}

void layers_init (void) {

    layers = dlist_init (layer_delete, layer_comparator);

    // add the default layer to the list
    Layer *default_layer = layer_new ("default", 0);
    dlist_insert_after (layers, dlist_end (layers), default_layer);

}

void layers_end (void) { 
    
    dlist_destroy (layers); 
    layers = NULL;
    
}

#pragma endregion

// FIXME: we need to implement occlusion culling!
void render (void) {

    SDL_RenderClear (main_renderer->renderer);

    // render by layers
    Layer *layer = NULL;
    GameObject *go = NULL;
    Transform *transform = NULL;
    Graphics *graphics = NULL;
    for (ListElement *layer_le = dlist_start (layers); layer_le; layer_le = layer_le->next) {
        layer = (Layer *) layer_le->data;

        for (ListElement *le = dlist_start (layer->gos); le; le = le->next) {
            go = (GameObject *) le->data;

            transform = (Transform *) game_object_get_component (go, TRANSFORM_COMP);
            graphics = (Graphics *) game_object_get_component (go, GRAPHICS_COMP);
            if (transform && graphics) {
                if (graphics->multipleSprites) {
                    texture_draw_frame (main_camera, graphics->spriteSheet, 
                        transform->position.x, transform->position.y, 
                        graphics->x_sprite_offset, graphics->y_sprite_offset,
                        graphics->flip);
                }
                
                else {
                    texture_draw (main_camera, graphics->sprite, 
                        transform->position.x, transform->position.y, 
                        graphics->flip);
                }
            }
        }
    }

    ui_render ();       // render ui elements

    SDL_RenderPresent (main_renderer->renderer);

}