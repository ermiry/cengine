#ifndef _CENGINE_UTILS_H_
#define _CENGINE_UTILS_H_

#include <stdbool.h>

/*** MISC ***/

extern bool system_is_little_endian (void);

/*** MATH ***/

extern int clamp_int (int val, int min, int max);
extern int abs_int (int value);
extern float lerp (float first, float second, float by);

/*** RANDOM ***/

extern void random_set_seed (unsigned int seed);
extern int random_int_in_range (int min, int max);

/*** CONVERTERS ***/

extern int xtoi (char *hexString);
extern char* itoa (int i, char b[]);

/*** STRINGS ***/

extern void copy (char *to, const char *from);
extern char **splitString (char *str, const char delim);
extern char *createString (const char *stringWithFormat, ...);
extern char *reverseString (char *str);

#endif