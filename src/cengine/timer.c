#include <stdbool.h>

#include <time.h>
#include <errno.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/timer.h"

#pragma region time

static TimeSpec *timespec_new (void) {

    TimeSpec *t = (TimeSpec *) malloc (sizeof (TimeSpec));
    if (t) {
        t->tv_nsec = 0;
        t->tv_sec = 0;
    }

    return t;

}

void timespec_delete (void *timespec_ptr) { if (timespec_ptr) free (timespec_ptr); }

TimeSpec *timer_get_timespec (void) {

    TimeSpec *t = timespec_new ();
    if (t) {
        clock_gettime (4, t);
    }

    return t;

}

double timer_elapsed_time (TimeSpec *start, TimeSpec *end) {

    return (start && end) ? (double) end->tv_sec + (double) end->tv_nsec / 1000000000
        - (double) start->tv_sec - (double) start->tv_nsec / 1000000000 : 0;

}

void timer_sleep_for_seconds (double seconds) {

    TimeSpec timespc = { 0 };
	timespc.tv_sec = (time_t) seconds;
	timespc.tv_nsec = (long) ((seconds - timespc.tv_sec) * 1e+9);

	int result;
	do {
		result = nanosleep (&timespc, &timespc);
	} while (result == -1 && errno == EINTR);

}

struct tm *timer_get_gmt_time (void) {

    time_t rawtime = 0;
    time (&rawtime);
    return gmtime (&rawtime);

}

struct tm *timer_get_local_time (void) {

    time_t rawtime = 0;
    time (&rawtime);
    return localtime (&rawtime);

}

// returns a string representing the 24h time 
String *timer_time_to_string (struct tm *timeinfo) {

    if (timeinfo) {
        char buffer[128] = { 0 };
        strftime (buffer, 128, "%T", timeinfo);
        return str_new (buffer);
    }

    return NULL;

}

// returns a string with day/month/year
String *timer_date_to_string (struct tm *timeinfo) {

    if (timeinfo) {
        char buffer[128] = { 0 };
        strftime (buffer, 128, "%d/%m/%y", timeinfo);
        return str_new (buffer);
    }

    return NULL;

}

// returns a string with day/month/year - 24h time
String *timer_date_and_time_to_string (struct tm *timeinfo) {

    if (timeinfo) {
        char buffer[128] = { 0 };
        strftime (buffer, 128, "%d/%m/%y - %T", timeinfo);
        return str_new (buffer);
    }

    return NULL;

}

// returns a string representing the time with custom format
String *timer_time_to_string_custom (struct tm *timeinfo, const char *format) {

    if (timeinfo) {
        char buffer[128] = { 0 };
        strftime (buffer, 128, format, timeinfo);
        return str_new (buffer);
    }
    
    return NULL;

}

#pragma endregion

#pragma region timer

static void timer_reset (Timer *timer) {

    if (timer) {
        timer->startTicks = 0;
        timer->pausedTicks = 0;

        timer->ticks = 0;

        timer->isPaused = false;
        timer->started = false;
    }

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

    u32 time_ret = 0;
    if (timer) {
        if (timer->started) {
            if (timer->isPaused) time_ret = timer->pausedTicks;
            else time_ret = SDL_GetTicks () - timer->startTicks;
        }

    }

    return time_ret;
    
}

#pragma endregion