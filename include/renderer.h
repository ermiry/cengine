#ifndef _CENGINE_RENDERER_H_
#define _CENGINE_RENDERER_H_

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "types/types.h"

extern SDL_Window *main_window;
extern SDL_Renderer *main_renderer;

typedef struct WindowSize {

    u32 width, height;

} WindowSize;

extern WindowSize windowSize;;
extern bool isFullscreen;

extern void window_toggle_full_screen (SDL_Window *window);
extern void window_resize (SDL_Window *window, u32 newWidth, u32 newHeight);

extern int video_init_main (const char *title);
extern void video_destroy_main (void);

extern void render (void);

#endif