#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blackrock.h"
#include "myos.h"

#if defined OS_LINUX
    #include <sys/prctl.h>
#endif

#include "cengine/thread.h"

#include "utils/log.h"

// FIXME: handle portability
// creates a custom detachable thread
int thread_create_detachable (void *(*work) (void *), void *args) {

    u8 retval = 1;

    #ifdef OS_LINUX
        pthread_attr_t attr;
        pthread_t thread;

        int rc = pthread_attr_init (&attr);
        rc = pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create (&thread, &attr, work, args) != THREAD_OK) 
            logMsg (stderr, ERROR, NO_TYPE, "Failed to create detachable thread!");
        else retval = 0;
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
        #elif defined   BLACK_DEBUG
            logMsg (stdout, WARNING, NO_TYPE, "pthread_setname_np is not supported on this system.");
        #endif
    }

    return retval;

}

#pragma region HubWorker

static HubWorker *hub_worker_new (void *(*work) (void *), void *args, const char *name) {

    HubWorker *worker = (HubWorker *) malloc (sizeof (HubWorker));
    if (worker) {
        memset (worker, 0, sizeof (HubWorker));

        worker->name = (char *) calloc (strlen (name) + 1, sizeof (char));
        strcpy ((char *) worker->name, (char *) name);

        worker->job = work;
        worker->args = args;
    }

    return worker;

}

// FIXME: handle portability for destroying all of our threads!!
static void hub_worker_destroy (HubWorker *worker) {

    if (worker) {
        if (worker->name) free ((char *) worker->name);

        free (worker);
    }

}

static int hub_worker_init (HubWorker *worker) {

    int retval = 1;

    if (worker) {
        #ifdef  OS_LINUX 
            retval = pthread_create (&worker->thread, NULL, worker->job, worker->args);
        #endif
    }

    return retval;

}

#pragma endregion

#pragma region ThreadHub

static ThreadHub *global_hub;

static ThreadHub *thread_hub_new (const char *name) {

    ThreadHub *hub = (ThreadHub *) malloc (sizeof (ThreadHub));
    if (hub) {
        memset (hub, 0, sizeof (ThreadHub));
        hub->name = (char *) calloc (strlen (name) + 1, sizeof (char));
        strcpy ((char *) hub->name, (char *) name);
    }

}

// inits a new thread hub
ThreadHub *thread_hub_int (const char *name) {  

    ThreadHub *hub = NULL;
    
    if (name) hub = thread_hub_new (name);

    return hub;

}

// inits the global thread hub
int thread_hub_init_global (void) {

    global_hub = thread_hub_new ("global");
    return (global_hub ? 0 : 1);

}

// ends a thread hub
int thread_hub_end (ThreadHub *hub) {

    int retval = 1;

    if (hub) {
        if (hub->name) free ((char *) hub->name);

        if (hub->n_workers > 0) 
            for (int i = 0; i < hub->n_workers; i++) hub_worker_destroy (hub->workers[i]);

        free (hub);
    }

    return retval;

}

static int thread_hub_add_worker (ThreadHub *hub, HubWorker *worker) {

    int retval = 1;

    if (hub && worker) {

    }

    return retval;

}

// adds a worker to the hub
// NULL hub to add to global
int thread_hub_add (ThreadHub *hub, void *(*work) (void *), void *args, const char *worker_name) {

    int retval = 1;

    if (work && worker_name) {
        HubWorker *worker = hub_worker_new (work, args, worker_name);

        // if no hub provided, add the work to the global thread, but only if it exists...
        ThreadHub *h = hub ? hub : global_hub;
        if (h) 
            if (thread_hub_add_worker (h, worker))
                retval = hub_worker_init (worker);

    }

    return retval;

}

// FIXME:
// removes a worker from the hub
// NULL hub to remove from global
int thread_hub_remove (ThreadHub *hub, const char *worker_name) {

    int retval = 1;

    if (worker_name) {
        // if no hub provided, remove from the global hub
        ThreadHub *h = hub ? hub : global_hub;
        if (h) {
            // search the worker
            for (int i = 0; i < h->n_workers; i++) {
                if (!strcmp (h->workers[i]->name, worker_name)) {
                    // if (h->n_workers == 1) free ()
                }
            }
        }
    }

    return retval;

}

#pragma endregion