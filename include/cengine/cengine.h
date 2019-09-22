#ifndef _CENGINE_H_
#define _CENGINE_H_

#include <stdbool.h>

#include "cengine/types/string.h"
#include "cengine/renderer.h"

#include "cengine/ui/textbox.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*** assets ***/

extern const String *cengine_assets_path;

// sets the path for the assets folder
extern void cengine_assets_set_path (const char *pathname);

/*** cengine ***/

// call this to initialize the cengine and create a new window
extern int cengine_init (const char *window_title, WindowSize window_size, bool full_screen);

// call this when you want to exit cengine 
extern int cengine_end (void);

// call this when you want to start cengine's main loops (update & render)
extern int cengine_start (int fps);

/*** threads ***/

extern bool running;

extern unsigned int fps_limit;

extern unsigned int cengine_get_fps_limit (void);

extern void cengine_set_main_fps_text (TextBox *text);

extern void cengine_set_update_fps_text (TextBox *text);

/*** other ***/

extern void (*cengine_quit)(void);

// sets the function to be executed on SDL_QUIT event
extern void cengine_set_quit (void (*quit)(void));

#endif