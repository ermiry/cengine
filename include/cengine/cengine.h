#ifndef _CENGINE_H_
#define _CENGINE_H_

#include "cengine/renderer.h"

extern bool running;

extern unsigned int fps_limit;

extern unsigned int main_fps;
extern unsigned int update_fps;

extern int cengine_init (const char *window_title, WindowSize window_size, bool full_screen);
extern int cengine_end (void);

extern int cengine_start (int fps);

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#endif