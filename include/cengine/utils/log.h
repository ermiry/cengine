#ifndef _CENGINE_LOG_H_
#define _CENGINE_LOG_H_

#include <stdio.h>

#define COLOR_RED       "\x1b[31m"
#define COLOR_GREEN     "\x1b[32m"
#define COLOR_YELLOW    "\x1b[33m"
#define COLOR_BLUE      "\x1b[34m"
#define COLOR_MAGENTA   "\x1b[35m"
#define COLOR_CYAN      "\x1b[36m"
#define COLOR_RESET     "\x1b[0m"

typedef enum LogMsgType {

	LOG_NO_TYPE = 0,

	LOG_ERROR = 1,
	LOG_WARNING,
	LOG_SUCCESS,
	LOG_DEBUG,

	LOG_CLIENT,

	LOG_TEST,

} LogMsgType;

void cengine_log_msg (FILE *__restrict __stream, LogMsgType first_type, LogMsgType second_type,
	const char *msg, ...);

// prints a red error message to stderr
void cengine_log_error (const char *msg);

// prints a yellow warning message to stderr
void cengine_log_warning (const char *msg);

// prints a green success message to stdout
void cengine_log_success (const char *msg);

// prints a debug message to stdout
void cengine_log_debug (const char *msg);

#endif