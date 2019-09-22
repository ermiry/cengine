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

#pragma region HubWorker

static HubWorker *hub_worker_new (void *(*work) (void *), void *args, const char *name) {

    HubWorker *worker = (HubWorker *) malloc (sizeof (HubWorker));
    if (worker) {
        memset (worker, 0, sizeof (HubWorker));

        if (name) worker->name = str_new (name);
        else worker->name = NULL;

        worker->job = work;
        worker->args = args;

        worker->thread = NULL;
    }

    return worker;

}

static void hub_worker_destroy (void *ptr) {

    if (ptr) {
        HubWorker *worker = (HubWorker *) ptr;

        str_delete (worker->name);

        #ifdef OS_LINUX
            if (pthread_join (worker->pthread, NULL)) {
                if (worker->name) cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, 
                    c_string_create ("Failed to join thread %s!", worker->name->str));

                else cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to join thread!");
            }
                
        #else
            SDL_WaitThread (worker->thread, NULL);
        #endif

        free (worker);
    }

}

static int hub_worker_init (HubWorker *worker) {

    int retval = 1;

    if (worker) {
        #ifdef  OS_LINUX 
            retval = pthread_create (&worker->pthread, NULL, worker->job, worker->args);
        #else
            worker->thread = SDL_CreateThread ((int (*) (void *)) worker->job, 
                worker->name->str, worker->args);
            if (worker->thread) retval = 0;
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
        if (name) hub->name = str_new (name);
        else hub->name = NULL;
        hub->threads = dlist_init (hub_worker_destroy, NULL);
    }

}

// inits a new thread hub
ThreadHub *thread_hub_int (const char *name) { return (name ? thread_hub_new (name) : NULL); }

// inits the global thread hub
int thread_hub_init_global (void) {

    global_hub = thread_hub_new ("global");
    return (global_hub ? 0 : 1);

}

// ends a thread hub
void thread_hub_end (ThreadHub *hub) {

    if (hub) {
        str_delete (hub->name);
        dlist_delete (hub->threads);

        free (hub);
    }

}

void thread_hub_end_global (void) { thread_hub_end (global_hub); }

// adds a worker to the hub
// NULL hub to add to global
// returns 0 on success, 1 on error
int thread_hub_add (ThreadHub *hub, void *(*work) (void *), void *args, const char *worker_name) {

    int retval = 1;

    if (work && worker_name) {
        HubWorker *worker = hub_worker_new (work, args, worker_name);

        // if no hub provided, add the work to the global thread, but only if it exists...
        ThreadHub *h = hub ? hub : global_hub;
        if (h) {
            dlist_insert_after (h->threads, dlist_end (h->threads), worker);
            retval = hub_worker_init (worker);
        }
    }

    return retval;

}

// removes a worker from the hub
// NULL hub to remove from global
int thread_hub_remove (ThreadHub *hub, const char *worker_name) {

    int retval = 1;

    if (worker_name) {
        // if no hub provided, remove from the global hub
        ThreadHub *h = hub ? hub : global_hub;
        if (h) {
            HubWorker *worker = NULL;
            for (ListElement *le = dlist_start (h->threads); le; le = le->next) {
                worker = (HubWorker *) le->data;
                if (!strcmp (worker_name, worker->name->str)) {
                    dlist_remove_element (hub->threads, le);
                    hub_worker_destroy (worker);
                    retval = 0;
                    break;
                }
            }
        }
    }

    return retval;

}

#pragma endregion