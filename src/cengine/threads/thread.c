#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "cengine/os.h"

#if defined OS_LINUX
    #include <sys/prctl.h>
#endif

#include "cengine/types/types.h"
#include "cengine/threads/thread.h"
#include "cengine/utils/log.h"
#include "cengine/utils/utils.h"

// creates a custom detachable thread (will go away on its own upon completion)
// handle manually in linux, with no name
// in any other platform, created with sdl api and you can pass a custom name
u8 thread_create_detachable (void *(*work) (void *), void *args) {

    u8 retval = 1;

    #ifdef OS_LINUX
        pthread_attr_t attr;
        pthread_t thread;

        int rc = pthread_attr_init (&attr);
        rc = pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create (&thread, &attr, work, args) != THREAD_OK)
            cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to create detachable thread!");
        else retval = 0;
    #else
        SDL_Thread *thread = SDL_CreateThread ((int (*) (void *)) work, NULL, args);
        if (thread) {
            SDL_DetachThread (thread);  // will go away on its own upon completion
            retval = 0;
        }
    #endif

    return retval;

}

// sets thread name from inisde it
int thread_set_name (const char *name) {

    int retval = 1;

    if (name) {
        #if defined     OS_LINUX
            // use prctl instead to prevent using _GNU_SOURCE flag and implicit declaration
            retval = prctl (PR_SET_NAME, name);
        #elif defined   OS_MACOS
            retval = pthread_setname_np (name);
        #elif defined   CENGINE_DEBUG
            cengine_log_msg (stdout, LOG_WARNING, LOG_NO_TYPE, "pthread_setname_np is not supported on this system.");
        #endif
    }

    return retval;

}