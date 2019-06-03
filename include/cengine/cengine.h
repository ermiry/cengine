#ifndef _CENGINE_H_
#define _CENGINE_H_

extern unsigned int main_fps;
extern unsigned int update_fps;

extern int cengine_init (const char *window_name);
extern int cengine_end (void);

extern int cengine_start (int fps);

#endif