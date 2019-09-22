#ifndef _CENGINE_UTILS_H_
#define _CENGINE_UTILS_H_

#include <stdbool.h>
#include <stdint.h>

/*** misc ***/

extern bool system_is_little_endian (void);

/*** math ***/

extern int clamp_int (int val, int min, int max);

int abs_int (int value);

extern float lerp (float first, float second, float by);

/*** random ***/

// init psuedo random generator based on our seed
extern void random_set_seed (unsigned int seed);

extern int random_int_in_range (int min, int max);

/*** converters ***/

// convert a string representing a hex to a string
extern int xtoi (char *hexString);

extern char *itoa (int i, char *b);

extern uint32_t convert_rgba_to_hex (uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/*** c strings ***/

// creates a new c string with the desired format, as in printf
extern char *c_string_create (const char *format, ...);

// splits a c string into tokens based on a delimiter
extern char **c_string_split (char *string, const char delim, int *n_tokens);

// copies a c string into another one previuosly allocated
extern void c_string_copy (char *to, const char *from);

// revers a c string
extern char *c_string_reverse (char *str);

#endif