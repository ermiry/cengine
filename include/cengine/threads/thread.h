#ifndef _CENGINE_THREADS_H_
#define _CENGINE_THREADS_H_

#include <pthread.h>

#include "cengine/types/types.h"

#include "cengine/config.h"

#define THREAD_OK       0

// creates a custom detachable thread (will go away on its own upon completion)
// returns 0 on success, 1 on error
CENGINE_PUBLIC u8 thread_create_detachable (pthread_t *thread, void *(*work) (void *), void *args);

// sets thread name from inisde it
CENGINE_PUBLIC int thread_set_name (const char *name);

#endif