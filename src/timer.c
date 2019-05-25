#include <SDL2/SDL.h>

#include "cengine/timer.h"

static void timer_reset (Timer *timer) {

    timer->startTicks = 0;
    timer->pausedTicks = 0;

    timer->ticks = 0;

    timer->isPaused = false;
    timer->started = false;

}

Timer *timer_new (void) {

    Timer *new_timer = (Timer *) malloc (sizeof (Timer));
    if (new_timer) timer_reset (new_timer);

    return new_timer;

}

void timer_destroy (Timer *timer) { if (timer) free (timer); }

void timer_start (Timer *timer) {

    if (timer) {
        timer->started = true;
        timer->isPaused = false;

        timer->startTicks = SDL_GetTicks ();
        timer->ticks = 0;
        timer->pausedTicks = 0;
    }

}

void timer_stop (Timer *timer) { if (timer) timer_reset (timer); }

void timer_pause (Timer *timer) {

    if (timer) {
        if (timer->started && !timer->isPaused) {
            timer->isPaused = true;
            timer->pausedTicks = SDL_GetTicks () - timer->startTicks;
            timer->startTicks = 0;
        }
    }

}

void timer_unpause (Timer *timer) {

    if (timer) {
        if (timer->started && timer->isPaused) {
            timer->isPaused = false;
            timer->startTicks = SDL_GetTicks () - timer->pausedTicks;
            timer->pausedTicks = 0;
        }
    }

}

u32 timer_getTicks (Timer *timer) {

    if (timer) {
        // u32 time_ret = 0;
        // if (timer->started) {
        //     // if (timer->isPaused) time_ret = timer->pausedTicks;
        //     // else time_ret = SDL_GetTicks () - timer->startTicks;
        //     timer
        // }

        // return time_ret;
    }

}