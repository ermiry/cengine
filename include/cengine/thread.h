#ifndef _CENGINE_THREAD_H_
#define _CENGINE_THREAD_H_

#include "myos.h"

#if defined OS_LINUX
    #include <pthread.h>
#endif

// creates a custom detachable thread
extern int pthread_create_detachable (void *(*work) (void *), void *args);

// sets thread name from inisde it
extern int thread_set_name (const char *name);

typedef struct HubWorker {

    // FIXME: handle portability
    #if defined     OS_LINUX
        pthread_t thread;
    #endif

    const char *name;
    void *(*job) (void *);
    void *args;

} HubWorker;

typedef struct ThreadHub {

    const char *name;           // thread hub name

    unsigned int n_workers;
    HubWorker **workers;

} ThreadHub;

// inits a new thread hub
extern ThreadHub *thread_hub_int (const char *name);

// inits the global thread hub
extern int thread_hub_init_global (void);

// ends a thread hub
extern int thread_hub_end (ThreadHub *hub);

// adds a worker to the hub
extern int thread_hub_add (ThreadHub *hub, void *(*work) (void *), void *args, const char *worker_name);

// removes a worker from the hub
extern int thread_hub_remove (ThreadHub *hub, const char *worker_name);

#endif