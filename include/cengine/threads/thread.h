#ifndef _CENGINE_THREAD_H_
#define _CENGINE_THREAD_H_

#include <SDL2/SDL.h>

#include "cengine/os.h"

#if defined OS_LINUX
    #include <pthread.h>
#endif

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/collections/dlist.h"

#define THREAD_OK   0

// creates a custom detachable thread (will go away on its own upon completion)
// handle manually in linux, with no name
// in any other platform, created with sdl api and you can pass a custom name
// returns 0 on success, 1 on error
extern u8 thread_create_detachable (void *(*work) (void *), void *args);

// sets thread name from inisde it
extern int thread_set_name (const char *name);

#endif