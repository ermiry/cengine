#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/timer.h"

struct tm *timer_get_gmt_time (void) {

    time_t rawtime;
    time (&rawtime);
    return gmtime (&rawtime);

}

struct tm *timer_get_local_time (void) {

    time_t rawtime;
    time (&rawtime);
    return localtime (&rawtime);

}

// returns a string representing the 24h time 
String *timer_time_to_string (struct tm *timeinfo) {

    if (timeinfo) {
        char buffer[80];
        strftime (buffer, sizeof (buffer), "%T", timeinfo);
        return str_new (buffer);
    }

    return NULL;

}

// returns a string with day/month/year
String *timer_date_to_string (struct tm *timeinfo) {

    if (timeinfo) {
        char buffer[80];
        strftime (buffer, sizeof (buffer), "%d/%m/%y", timeinfo);
        return str_new (buffer);
    }

    return NULL;

}

// returns a string with day/month/year - 24h time
String *timer_date_and_time_to_string (struct tm *timeinfo) {

    if (timeinfo) {
        char buffer[80];
        strftime (buffer, sizeof (buffer), "%d/%m/%y - %T", timeinfo);
        return str_new (buffer);
    }

    return NULL;

}

// returns a string representing the time with custom format
String *timer_time_to_string_custom (struct tm *timeinfo, const char *format) {

    if (timeinfo) {
        char buffer[80];
        strftime (buffer, sizeof (buffer), format, timeinfo);
        return str_new (buffer);
    }
    
    return NULL;

}

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

u32 timer_get_ticks (Timer *timer) {

    if (timer) {
        u32 time_ret = 0;
        if (timer->started) {
            if (timer->isPaused) time_ret = timer->pausedTicks;
            else time_ret = SDL_GetTicks () - timer->startTicks;
        }

        return time_ret;
    }

}