#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cengine/utils/utils.h"
#include "cengine/utils/log.h"

static char *cengine_get_msg_type (LogMsgType type) {

    char temp[10];

    switch (type) {
        case ERROR: strcpy (temp, "[ERROR]"); break;
        case WARNING: strcpy (temp, "[WARNING]"); break;
        case SUCCESS: strcpy (temp, "[SUCCESS]"); break;
        case DEBUG_MSG: strcpy (temp, "[DEBUG]"); break;
        case TEST: strcpy (temp, "[TEST]"); break;

        default: break;
    }

    char *retval = (char *) calloc (strlen (temp) + 1, sizeof (temp));
    strcpy (retval, temp);

    return retval;

}

void cengine_log_msg (FILE *__restrict __stream, LogMsgType firstType, LogMsgType secondType,
    const char *msg) {

    char *first = cengine_get_msg_type (firstType);
    char *second = NULL;
    char *message = NULL;

    if (secondType != 0) {
        second = cengine_get_msg_type (secondType);

        if (firstType == DEBUG_MSG)
            message = c_string_create ("%s: %s\n", second, msg);
        
        else message = c_string_create ("%s%s: %s\n", first, second, msg);
    }

    else if (firstType != DEBUG_MSG)
        message = c_string_create ("%s: %s\n", first, msg);

    // log messages with color
    switch (firstType) {
        case DEBUG_MSG: 
            fprintf (__stream, COLOR_MAGENTA "%s: " COLOR_RESET "%s\n", first, msg); break;

        case ERROR: fprintf (__stream, COLOR_RED "%s" COLOR_RESET, message); break;
        case WARNING: fprintf (__stream, COLOR_YELLOW "%s" COLOR_RESET, message); break;
        case SUCCESS: fprintf (__stream, COLOR_GREEN "%s" COLOR_RESET, message); break;

        default: fprintf (__stream, "%s", message); break;
    }

    if (message) free (message);

}