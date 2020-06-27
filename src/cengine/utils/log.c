#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>

#include "cengine/utils/utils.h"
#include "cengine/utils/log.h"

static char *cengine_get_msg_type (LogMsgType type) {

	char temp[16] = { 0 };

	switch (type) {
		case LOG_ERROR: strcpy (temp, "[ERROR]"); break;
		case LOG_WARNING: strcpy (temp, "[WARNING]"); break;
		case LOG_SUCCESS: strcpy (temp, "[SUCCESS]"); break;
		case LOG_DEBUG: strcpy (temp, "[DEBUG]"); break;
		case LOG_TEST: strcpy (temp, "[TEST]"); break;

		case LOG_CERVER: strcpy (temp, "[CERVER]"); break;
		case LOG_CLIENT: strcpy (temp, "[CLIENT]"); break;

		case LOG_REQ: strcpy (temp, "[REQ]"); break;
		case LOG_FILE: strcpy (temp, "[FILE]"); break;
		case LOG_PACKET: strcpy (temp, "[PACKET]"); break;
		case LOG_PLAYER: strcpy (temp, "[PLAYER]"); break;
		case LOG_GAME: strcpy (temp, "[GAME]"); break;

		default: break;
	}

	char *retval = (char *) calloc (strlen (temp) + 1, sizeof (temp));
    if (retval) strcpy (retval, temp);

	return retval;

}

void cengine_log_msg (FILE *__restrict __stream, LogMsgType first_type, LogMsgType second_type,
    const char *msg, ...) {

    char *first = cengine_get_msg_type (first_type);
    char *second = NULL;
    char *message = NULL;

    if (second_type != 0) {
        second = cengine_get_msg_type (second_type);

        if (first_type == LOG_DEBUG)
            message = c_string_create ("%s: %s\n", second, msg);
        
        else message = c_string_create ("%s%s: %s\n", first, second, msg);
    }

    else if (first_type != LOG_DEBUG)
        message = c_string_create ("%s: %s\n", first, msg);

    // log messages with color
    switch (first_type) {
        case LOG_DEBUG: fprintf (__stream, COLOR_MAGENTA "%s: " COLOR_RESET "%s\n", first, msg); break;

        case LOG_ERROR: fprintf (__stream, COLOR_RED "%s" COLOR_RESET, message); break;
        case LOG_WARNING: fprintf (__stream, COLOR_YELLOW "%s" COLOR_RESET, message); break;
        case LOG_SUCCESS: fprintf (__stream, COLOR_GREEN "%s" COLOR_RESET, message); break;

        case LOG_CLIENT: fprintf (__stream, COLOR_BLUE "%s" COLOR_RESET, message); break;

        default: fprintf (__stream, "%s", message); break;
    }

    if (message) free (message);

}

// prints a red error message to stderr
void cengine_log_error (const char *msg) {

	if (msg) fprintf (stderr, COLOR_RED "[ERROR]: " "%s\n" COLOR_RESET, msg);

}

// prints a yellow warning message to stderr
void cengine_log_warning (const char *msg) {

	if (msg) fprintf (stderr, COLOR_YELLOW "[WARNING]: " "%s\n" COLOR_RESET, msg);

}

// prints a green success message to stdout
void cengine_log_success (const char *msg) {

	if (msg) fprintf (stdout, COLOR_GREEN "[SUCCESS]: " "%s\n" COLOR_RESET, msg);

}

// prints a debug message to stdout
void cengine_log_debug (const char *msg) {

	if (msg) fprintf (stdout, COLOR_MAGENTA "[DEBUG]: " COLOR_RESET "%s\n", msg);

}