#ifndef _CENGINE_TYPES_STRING_H_
#define _CENGINE_TYPES_STRING_H_

#include "cengine/types/types.h"

#include "cengine/config.h"

typedef struct String {

    unsigned int len;
    char *str;

} String;

CENGINE_PUBLIC String *str_new (const char *str);

CENGINE_PUBLIC void str_delete (void *str_ptr);

CENGINE_PUBLIC String *str_create (const char *format, ...);

CENGINE_PUBLIC int str_compare (const String *s1, const String *s2);

CENGINE_PUBLIC int str_comparator (const void *a, const void *b);

CENGINE_PUBLIC void str_copy (String *to, String *from);

CENGINE_PUBLIC void str_replace (String *old, const char *str);

// concatenates two strings into a new one
CENGINE_PUBLIC String *str_concat (String *s1, String *s2);

// appends a char to the end of the string
// reallocates the same string
CENGINE_PUBLIC void str_append_char (String *s, const char c);

// appends a c string at the end of the string
// reallocates the same string
CENGINE_PUBLIC void str_append_c_string (String *s, const char *c_str);

CENGINE_PUBLIC void str_to_upper (String *string);

CENGINE_PUBLIC void str_to_lower (String *string);

CENGINE_PUBLIC char **str_split (String *string, const char delim, int *n_tokens);

CENGINE_PUBLIC void str_remove_char (String *string, char garbage);

// removes the last char from a string
CENGINE_PUBLIC void str_remove_last_char (String *string);

// checks if a string (to_find) is inside string
// returns 0 on exact match
// returns 1 if it match the letters but len is different
// returns -1 if no match
CENGINE_PUBLIC int str_contains (String *string, char *to_find);

/*** serialization ***/

typedef enum SStringSize {

    SS_SMALL = 64,
    SS_MEDIUM = 128,
    SS_LARGE = 256,
    SS_EXTRA_LARGE = 512

} SStringSize;

// serialized string (small)
typedef struct SStringS {

    u16 len;
    char string[64];

} SStringS;

// serialized string (medium)
typedef struct SStringM {

    u16 len;
    char string[128];

} SStringM;

// serialized string (large)
typedef struct SStringL {

    u16 len;
    char string[256];

} SStringL;

// serialized string (extra large)
typedef struct SStringXL {

    u16 len;
    char string[512];

} SStringXL;

// returns a ptr to a serialized string
CENGINE_PUBLIC void *str_serialize (String *string, SStringSize size);

#endif