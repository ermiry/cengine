#ifndef _CENGINE_TIMER_H_
#define _CENGINE_TIMER_H_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

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

#endif