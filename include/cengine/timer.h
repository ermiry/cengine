#ifndef _CENGINE_TIMER_H_
#define _CENGINE_TIMER_H_

#ifndef __USE_POSIX199309
    #define __USE_POSIX199309
#endif

#include <stdbool.h>
#include <time.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/config.h"

#pragma region time

typedef struct timespec TimeSpec;

CENGINE_PUBLIC void timespec_delete (void *timespec_ptr);

CENGINE_PUBLIC TimeSpec *timer_get_timespec (void);

CENGINE_PUBLIC double timer_elapsed_time (TimeSpec *start, TimeSpec *end);

CENGINE_PUBLIC void timer_sleep_for_seconds (double seconds);

CENGINE_PUBLIC struct tm *timer_get_gmt_time (void);

CENGINE_PUBLIC struct tm *timer_get_local_time (void);

// returns a string representing the 24h time 
CENGINE_PUBLIC String *timer_time_to_string (struct tm *timeinfo);

// returns a string with day/month/year
CENGINE_PUBLIC String *timer_date_to_string (struct tm *timeinfo);

// returns a string with day/month/year - 24h time
CENGINE_PUBLIC String *timer_date_and_time_to_string (struct tm *timeinfo);

// returns a string representing the time with custom format
CENGINE_PUBLIC String *timer_time_to_string_custom (struct tm *timeinfo, const char *format);

#pragma endregion

#pragma region timer

typedef struct Timer {

    u32 startTicks;     // the clock time when the timer started
    u32 pausedTicks;    // the ticks stored when the timer was paused

    u32 ticks;

    bool isPaused;
    bool started;

} Timer;

CENGINE_PUBLIC Timer *timer_new (void);

CENGINE_PUBLIC void timer_destroy (Timer *timer);

CENGINE_PUBLIC void timer_start (Timer *timer);

CENGINE_PUBLIC void timer_stop (Timer *timer);

CENGINE_PUBLIC void timer_pause (Timer *timer);

CENGINE_PUBLIC void timer_unpause (Timer *timer);

CENGINE_PUBLIC u32 timer_get_ticks (Timer *timer);

#pragma endregion

#endif