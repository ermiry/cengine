#ifndef _CENGINE_VIDEO_H_
#define _CENGINE_VIDEO_H_

#include <SDL2/SDL_video.h>

#include "cengine/types/types.h"

typedef struct WindowSize {

    u32 width, height;

} WindowSize;

// returns 0 on success, 1 on error
extern int video_get_display_mode (int display_index, SDL_DisplayMode *display_mode);

#endif