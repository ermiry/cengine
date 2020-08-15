#ifndef _CENGINE_H_
#define _CENGINE_H_

#include <stdbool.h>

#include "cengine/config.h"
#include "cengine/renderer.h"

#include "cengine/ui/textbox.h"

#define EXIT_FAILURE    1

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*** cengine ***/

// call this to initialize cengine
CENGINE_EXPORT int cengine_init (void);

// call this when you want to exit cengine 
CENGINE_EXPORT int cengine_end (void);

// call this when you want to start cengine's main loops (update & render)
CENGINE_EXPORT int cengine_start (int fps);

/*** threads ***/

CENGINE_PUBLIC bool running;

CENGINE_EXPORT unsigned int fps_limit;

CENGINE_EXPORT unsigned int cengine_get_fps_limit (void);

CENGINE_EXPORT void cengine_set_main_fps_text (TextBox *text);

CENGINE_EXPORT void cengine_set_update_fps_text (TextBox *text);

/*** other ***/

CENGINE_PUBLIC void (*cengine_quit)(void);

// sets the function to be executed on SDL_QUIT event
CENGINE_EXPORT void cengine_set_quit (void (*quit)(void));

#endif