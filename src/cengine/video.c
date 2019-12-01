#include <SDL2/SDL_video.h>

#include "cengine/utils/utils.h"
#include "cengine/utils/log.h"

// retunrs 0 on success, 1 on error
int video_get_display_mode (int display_index, SDL_DisplayMode *display_mode) {

    int retval = 1;

    if (!SDL_GetCurrentDisplayMode (display_index, display_mode)) {
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stdout, LOG_DEBUG, LOG_NO_TYPE,
            c_string_create ("Display with idx %i mode is %dx%dpx @ %dhz.",
            display_index, 
            display_mode->w, display_mode->h, 
            display_mode->refresh_rate));
        #endif

        retval = 0;
    }

    else {
        cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, 
            c_string_create ("Failed to get display mode for display with idx %i", display_index));
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, SDL_GetError ());
        #endif
    }

    return retval;

}