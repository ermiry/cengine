#ifndef _CENGINE_TYPES_STRING_H_
#define _CENGINE_TYPES_STRING_H_

#include "cengine/types/types.h"

typedef struct String {

    unsigned int len;
    char *str;

} String;

extern String *str_new (const char *str);
extern String *str_create (const char *format, ...);
extern void str_delete (void *str_ptr);

extern void str_copy (String *to, String *from);

extern void str_replace (String *old, const char *str);

// concatenates two strings into a new one
extern String *str_concat (String *s1, String *s2);

// appends a char to the end of the string
// reallocates the same string
extern void str_append_char (String *s, const char c);

// appends a c string at the end of the string
// reallocates the same string
extern void str_append_c_string (String *s, const char *c_str);

extern void str_to_upper (String *string);
extern void str_to_lower (String *string);

extern int str_compare (const String *s1, const String *s2);
extern int str_comparator (const void *a, const void *b) ;

extern char **str_split (String *string, const char delim, int *n_tokens);

extern void str_remove_char (String *string, char garbage);

// removes the last char from a string
extern void str_remove_last_char (String *string);

// checks if a string (to_find) is inside string
// returns 0 on exact match
// returns 1 if it match the letters but len is different
// returns -1 if no match
extern int str_contains (String *string, char *to_find);

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
extern void *str_serialize (String *string, SStringSize size);

#endif