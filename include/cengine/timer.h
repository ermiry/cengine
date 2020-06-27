#ifndef _CENGINE_TIMER_H_
#define _CENGINE_TIMER_H_

#ifndef __USE_POSIX199309
    #define __USE_POSIX199309
#endif

#include <stdbool.h>
#include <time.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#pragma region time

typedef struct timespec TimeSpec;

extern void timespec_delete (void *timespec_ptr);

extern TimeSpec *timer_get_timespec (void);

extern double timer_elapsed_time (TimeSpec *start, TimeSpec *end);

extern void timer_sleep_for_seconds (double seconds);

extern struct tm *timer_get_gmt_time (void);

extern struct tm *timer_get_local_time (void);

// returns a string representing the 24h time 
extern String *timer_time_to_string (struct tm *timeinfo);

// returns a string with day/month/year
extern String *timer_date_to_string (struct tm *timeinfo);

// returns a string with day/month/year - 24h time
extern String *timer_date_and_time_to_string (struct tm *timeinfo);

// returns a string representing the time with custom format
extern String *timer_time_to_string_custom (struct tm *timeinfo, const char *format);

#pragma endregion

#pragma region timer

typedef struct Timer {

    u32 startTicks;     // the clock time when the timer started
    u32 pausedTicks;    // the ticks stored when the timer was paused

    u32 ticks;

    bool isPaused;
    bool started;

} Timer;

extern Timer *timer_new (void);

extern void timer_destroy (Timer *timer);

extern void timer_start (Timer *timer);

extern void timer_stop (Timer *timer);

extern void timer_pause (Timer *timer);

extern void timer_unpause (Timer *timer);

extern u32 timer_get_ticks (Timer *timer);

#pragma endregion

#endif