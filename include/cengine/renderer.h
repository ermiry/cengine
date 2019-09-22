#ifndef _CENGINE_RENDERER_H_
#define _CENGINE_RENDERER_H_

#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/collections/dlist.h"

typedef struct WindowSize {

    u32 width, height;

} WindowSize;

typedef struct Renderer {

    String *name;
    SDL_Renderer *renderer;
    int index;
    Uint32 flags;

    int display_index;
    SDL_DisplayMode display_mode;

    String *window_title;
    SDL_Window *window;
    WindowSize window_size;
    bool full_screen;

} Renderer;

/*** Window ***/

// gets window size into renderer data struct
// returns 0 on success, 1 on error
extern int window_get_size (SDL_Window *window, WindowSize *window_size);

// toggle full screen on and off
// returns 0 on success, 1 on error
extern int window_toggle_full_screen (Renderer *renderer);

// resizes the window asscoaited with a renderer
// returns 0 on success, 1 on error
extern int window_resize (Renderer *renderer, u32 new_width, u32 new_height);

/*** Renderer ***/

// creates a new renderer
// returns a new renderer on success, NULL on error
extern Renderer *render_create_renderer (const char *renderer_name, Uint32 flags, int display_index,
    const char *window_title, WindowSize window_size, bool full_screen);

// TODO: as of 03/06/2019 we only have support for one renderer, the main one
// the plan is to have as many as you want in order to support multiple windows 
extern Renderer *main_renderer;

extern int renderer_init_main (Uint32 flags,
    const char *window_title, WindowSize window_size, bool full_screen);

extern void renderer_delete_main (void);

/*** Layers ***/

typedef struct Layer {

    String *name;
    int pos;
    DoubleList *elements;

} Layer;

extern DoubleList *gos_layers;              // render layers for the gameobjects
extern DoubleList *ui_elements_layers;      // render layers for the ui elements

extern Layer *layer_get_by_pos (DoubleList *layers, int pos);

extern Layer *layer_get_by_name (DoubleList *layers, const char *name);

// creates a new layer; 
// takes the layer name and the layer pos, -1 for last layer
// pos 0 renders first
// returns 0 on success, 1 on error
extern int layer_create (DoubleList *layers, const char *name, int pos, bool gos);

// adds an element to a layer
// returns 0 on succes, 1 on error
extern int layer_add_element (Layer *layer, void *ptr);

// adds an element to a layer that is gotten by its name 
// returns 0 on succes, 1 on error
extern int layer_add_element_by_name (DoubleList *layers, const char *layer_name, void *ptr);

// removes an element from a layer
// returns 0 on succes, 1 on error
extern int layer_remove_element (Layer *layer, void *ptr);

// removes an element from a layer taht is gotten by its name
// returns 0 on succes, 1 on error
extern int layer_remove_element_by_name (DoubleList *layers, const char *layer_name, void *ptr);

/*** Surfaces ***/

extern SDL_Surface *surface_create (int width, int height);

/*** Render Basic ***/

// renders a dot
extern void render_basic_dot (int x, int y, SDL_Color color);

// renders a horizontal line of dots
extern void render_basic_dot_line_horizontal (int start, int y, int length, int offset, SDL_Color color);

// renders a vertical line of dots
extern void render_basic_dot_line_vertical (int x, int start, int length, int offset, SDL_Color color);

// renders a filled rect
extern void render_basic_filled_rect (SDL_Rect *rect, SDL_Color color);

// renders an outline rect
extern void render_basic_outline_rect (SDL_Rect *rect, SDL_Color color);

// renders a line
extern void render_basic_line (int x1, int x2, int y1, int y2, SDL_Color color);

/*** Render Complex ***/

// renders a rect with transparency
extern SDL_Texture *render_complex_transparent_rect (SDL_Rect *rect, SDL_Color color);

/*** Render ***/

extern void render (void);

/*** Public ***/

// inits cengine render capabilities
extern u8 render_init (void);

extern void render_end (void);

#endif