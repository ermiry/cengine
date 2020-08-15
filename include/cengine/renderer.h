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
#include "cengine/collections/queue.h"

#include "cengine/threads/thread.h"

#include "cengine/config.h"
#include "cengine/graphics.h"
#include "cengine/video.h"
#include "cengine/window.h"

#include "cengine/ui/ui.h"

#define DEFAULT_BG_LOADING_FACTOR                   1
#define DEFAULT_BG_DESTROYING_FACTOR                1

struct _Window;
struct _UI;

struct _TextureWrapper;

// auxiliary structure to map the source surface to a texture
typedef struct SurfaceTexture {

    SDL_Surface *surface;
    SDL_Texture **texture;
    bool update;

} SurfaceTexture;

CENGINE_PRIVATE SurfaceTexture *surface_texture_new (SDL_Surface *surface, SDL_Texture **texture, bool update);

CENGINE_PRIVATE void surface_texture_delete (void *st_ptr);

/*** Renderer ***/

struct _Renderer {

    u64 id;

    String *name;
    pthread_t thread_id;

    SDL_Renderer *renderer;
    Uint32 render_flags;
    u32 render_count;

    Queue *load_textures_queue;
    u32 bg_loading_factor;

    Queue *destroy_textures_queue;
    u32 bg_destroying_factor;

    SDL_Rect current_viewport;
    SDL_Rect previous_viewport;

    struct _Window *window;

    struct _UI *ui;

    Action update;
    void *update_args;

};

typedef struct _Renderer Renderer;

CENGINE_PRIVATE DoubleList *renderers;

CENGINE_PRIVATE void renderer_delete (void *ptr);

// gets the renderer by its name
CENGINE_PUBLIC Renderer *renderer_get_by_name (const char *name);

// creates a new empty renderer without a window attached to it
CENGINE_PUBLIC Renderer *renderer_create_empty (const char *name, int display_idx);

// creates a new renderer with a window attached to it
CENGINE_PUBLIC Renderer *renderer_create_with_window (const char *name, int display_idx,
    Uint32 render_flags,
    const char *window_title, WindowSize window_size, Uint32 window_flags);

// attaches a new window to a renderer
// creates a new window and then a new render (SDL_Renderer) for it
// retunrs 0 on success, 1 on error
CENGINE_PUBLIC int renderer_window_attach (Renderer *renderer, Uint32 render_flags, int display_idx,
    const char *window_title, WindowSize window_size, Uint32 window_flags);

CENGINE_PRIVATE void renderer_load_queue_push (Renderer *renderer, SurfaceTexture *st);

CENGINE_PRIVATE void renderer_destroy_queue_push (Renderer *renderer, SDL_Texture *texture);

// sets how many textures the renderer can create in the background every loop
// example: if you have a frame rate of 30, the default loading factor is 1,
// so you will load 30 textures in a second, 1 for each frame
CENGINE_EXPORT void renderer_set_background_texture_loading_factor (Renderer *renderer, u32 bg_loading_factor);

// sets how many textures the renderer can destroy in the background every loop
CENGINE_EXPORT void renderer_set_background_texture_destroying_factor (Renderer *renderer, u32 bg_destroying_factor);

// sets an action to executed on every renderer update
// you can use this if you want to perform action son ui elements, like checking for 
// the current ui element under the mouse using the ui_element_hover in UI
CENGINE_EXPORT void renderer_set_update (Renderer *renderer, Action update, void *update_args);

// sets the renderer's viewport to be of the specified size
CENGINE_EXPORT void renderer_set_viewport (Renderer *renderer, u32 x, u32 y, u32 width, u32 height);

// sets the renderer's viewport to the previous size
CENGINE_EXPORT void renderer_set_viewport_to_previous_size (Renderer *renderer);

// sets the renderer's viewport to be the size of the window
CENGINE_EXPORT void renderer_set_viewport_to_window_size (Renderer *renderer);

/*** Layers ***/

typedef struct Layer {

    String *name;
    int pos;
    DoubleList *elements;

} Layer;

extern DoubleList *gos_layers;              // render layers for the gameobjects

CENGINE_PUBLIC Layer *layer_get_by_pos (DoubleList *layers, int pos);

CENGINE_PUBLIC Layer *layer_get_by_name (DoubleList *layers, const char *name);

// creates a new layer; 
// takes the layer name and the layer pos, -1 for last layer
// pos 0 renders first
// returns 0 on success, 1 on error
CENGINE_PUBLIC int layer_create (DoubleList *layers, const char *name, int pos, bool gos);

// adds an element to a layer
// returns 0 on succes, 1 on error
CENGINE_PUBLIC int layer_add_element (Layer *layer, void *ptr);

// adds an element to a layer that is gotten by its name 
// returns 0 on succes, 1 on error
CENGINE_PUBLIC int layer_add_element_by_name (DoubleList *layers, const char *layer_name, void *ptr);

// removes an element from a layer
// returns 0 on succes, 1 on error
CENGINE_PUBLIC int layer_remove_element (Layer *layer, void *ptr);

// removes an element from a layer taht is gotten by its name
// returns 0 on succes, 1 on error
CENGINE_PUBLIC int layer_remove_element_by_name (DoubleList *layers, const char *layer_name, void *ptr);

// init the ui elements layers
CENGINE_PRIVATE DoubleList *ui_layers_init (void);

/*** Surfaces ***/

// creates a new empty surface
CENGINE_PUBLIC SDL_Surface *surface_create (int width, int height);

// loads an image into a new surface
CENGINE_PUBLIC SDL_Surface *surface_load_image (const char *filename);

// wrapper function to destroy a sdl surface
CENGINE_PUBLIC void surface_delete (SDL_Surface *surface);

/*** Render Basic ***/

// draws a pixel in currently set color
// returns 0 on success, -1 on error
CENGINE_EXPORT int render_basic_pixel (Renderer *renderer, int x, int y);

// draws a pixel with blending enabled if a < 255
// returns 0 on success, -1 on failure
CENGINE_EXPORT int render_basic_pixel_rgba (Renderer *renderer, 
    int x, int y,
    u8 r, u8 g, u8 b, u8 a);

// draws a pixel with blending enabled if a < 255
// the color value of the pixel to draw must be in format 0xRRGGBBAA
// returns 0 on success, -1 on failure
CENGINE_EXPORT int render_basic_pixel_color (Renderer *renderer, int x, int y, u32 color);

// renders a dot
CENGINE_EXPORT void render_basic_dot (Renderer *renderer, int x, int y, SDL_Color color,
    float x_scale, float y_scale);

// renders a horizontal line of dots
CENGINE_EXPORT void render_basic_dot_line_horizontal (Renderer *renderer, int start, int end, int y, int offset, SDL_Color color,
    float x_scale, float y_scale);

// renders a vertical line of dots
CENGINE_EXPORT void render_basic_dot_line_vertical (Renderer *renderer, int start, int end, int x, int offset, SDL_Color color,
    float x_scale, float y_scale);

// draws a box (filled rectangle) with blending
// returns 0 on success, -1 on failure
CENGINE_EXPORT int render_basic_box (Renderer *renderer, int x1, int y1, int x2, int y2, SDL_Color color);

// renders a filled rect
CENGINE_EXPORT void render_basic_filled_rect (Renderer *renderer, SDL_Rect *rect, SDL_Color color);

// renders an outline rect
// scale works better with even numbers
CENGINE_EXPORT void render_basic_outline_rect (Renderer *renderer, SDL_Rect *rect, SDL_Color color, float scale_x, float scale_y);

// renders a line
// scale works better with even numbers
CENGINE_EXPORT void render_basic_line (Renderer *renderer, int x1, int x2, int y1, int y2, SDL_Color color, float scale_x, float scale_y);

// draws a horizontal line with blending
// returns 0 on success, -1 on failure
CENGINE_EXPORT int render_basic_horizontal_line (Renderer *renderer, 
    int x1, int x2, int y,
    SDL_Color color);

// draws a vertical line with blending
// returns 0 on success, -1 on failure
CENGINE_EXPORT int render_basic_vertical_line (Renderer *renderer,
    int x, int y1, int y2,
    SDL_Color color);

/*** Render Complex ***/

// renders a rect with transparency
CENGINE_EXPORT void render_complex_transparent_rect (Renderer *renderer, SDL_Texture **texture, SDL_Rect *rect, SDL_Color color);

// draws and arc with blending
// returns 0 on success, -1 on error
CENGINE_EXPORT int render_complex_arc (Renderer *renderer, 
    int x, int y, int rad,
    int start, int end,
    SDL_Color color);

// draws rounded-corner rectangle (outline) with blending
// returns 0 on success, -1 on failure
CENGINE_EXPORT int render_complex_rounded_rect (Renderer *renderer, 
    int x1, int y1, int x2, int y2, int rad,
    SDL_Color color);

// draws rounded-corner box (filled rectangle) with blending
// returns 0 on success, -1 on failure
CENGINE_EXPORT int render_complex_rounded_box (Renderer *renderer, 
    int x1, int y1, int x2, int y2, int rad,
    SDL_Color color);

// draws an ellipse or a filled ellipse with blending
// returns 0 on success, -1 on failure
CENGINE_EXPORT int render_complex_ellipse (Renderer *renderer,
    int x, int y,
    int rx, int ry,
    SDL_Color color, bool filled);

// draws a circle with blending
// returns 0 on success, -1 on failure
CENGINE_EXPORT int render_complex_circle (Renderer *renderer,
    int x, int y,
    int rad, 
    SDL_Color color, bool filled);

/*** Render ***/

CENGINE_PRIVATE void render (Renderer *renderer);

/*** Public ***/

// inits cengine render capabilities
CENGINE_PRIVATE u8 render_init (void);

CENGINE_PRIVATE u8 render_end (void);

#endif