#ifndef _CENGINE_UTILS_H_
#define _CENGINE_UTILS_H_

#include <stdbool.h>

#include <stdint.h>
#include <stddef.h>

#include "cengine/config.h"

/*** misc ***/

CENGINE_PUBLIC bool system_is_little_endian (void);

/*** math ***/

CENGINE_PUBLIC int clamp_int (int val, int min, int max);

CENGINE_PUBLIC int abs_int (int value);

CENGINE_PUBLIC float lerp (float first, float second, float by);

/*** random ***/

// init psuedo random generator based on our seed
CENGINE_PUBLIC void random_set_seed (unsigned int seed);

CENGINE_PUBLIC int random_int_in_range (int min, int max);

// abds = 5 for random float values between 0.0001 and 4.9999
CENGINE_PUBLIC float random_float (float abs);

/*** converters ***/

// convert a string representing a hex to a string
CENGINE_PUBLIC int xtoi (char *hexString);

CENGINE_PUBLIC char *itoa (int i, char *b);

CENGINE_PUBLIC uint32_t convert_rgba_to_hex (uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/*** c strings ***/

// copies a c string into another one previuosly allocated
CENGINE_PUBLIC void c_string_copy (char *to, const char *from);

// copies n bytes from a c string into another one previuosly allocated
CENGINE_PUBLIC void c_string_n_copy (char *to, const char *from, size_t n);

// creates a new c string with the desired format, as in printf
CENGINE_PUBLIC char *c_string_create (const char *format, ...);

// get how many tokens will be extracted by counting the number of apperances of the delim
// the original string won't be affected
CENGINE_PUBLIC size_t c_string_count_tokens (const char *original, const char delim);

// splits a c string into tokens based on a delimiter
// the original string won't be affected
// this method is thread safe as it uses __strtok_r () instead of the regular strtok ()
CENGINE_PUBLIC char **c_string_split (const char *original, const char delim, size_t *n_tokens);

// revers a c string
// returns a newly allocated c string
CENGINE_PUBLIC char *c_string_reverse (const char *str);

// removes all ocurrances of a char from a string
CENGINE_PUBLIC void c_string_remove_char (char *string, char garbage);

// removes the exact sub string from the main one
// returns a newly allocated copy of the original str but withput the sub
CENGINE_PUBLIC char *c_string_remove_sub (char *str, const char *sub);

// removes any white space from the string
CENGINE_PUBLIC char *c_string_trim (char *str);

// removes quotes from string
CENGINE_PUBLIC char *c_string_strip_quotes (char *str);

// returns true if the string starts with the selected sub string
CENGINE_PUBLIC bool c_string_starts_with (const char *str, const char *substr);

// creates a newly allocated string using the data between the two pointers of the SAME string
// returns a new string, NULL on error
CENGINE_PUBLIC char *c_string_create_with_ptrs (char *first, char *last);

// removes a substring from a c string that is defined after a token
// returns a newly allocated string without the sub,
// and option to retrieve the actual substring
CENGINE_PUBLIC char *c_string_remove_sub_after_token (char *str, const char token, char **sub);

// removes a substring from a c string after the idex of the token
// returns a newly allocated string without the sub,
// and option to retrieve the actual substring
// idx set to -1 for the last token match
// example: /home/ermiry/Documents, token: '/', idx: -1, returns: Documents
CENGINE_PUBLIC char *c_string_remove_sub_after_token_with_idx (char *str, const char token, char **sub, int idx);

// removes a substring from a c string delimited by two equal tokens
// takes the first and last appearance of the token
// example: test_20191118142101759__TEST__.png - token: '_'
// result: test.png
// returns a newly allocated string, and a option to get the substring
CENGINE_PUBLIC char *c_string_remove_sub_simetric_token (char *str, const char token, char **sub);

// removes a substring from a c string delimitied by two equal tokens
// and you can select the idx of the token; use -1 for last token
// example: test_20191118142101759__TEST__.png - token: '_' - idx (first: 1,  last: 3)
// result: testTEST__.png
// returns a newly allocated string, and a option to get the substring
CENGINE_PUBLIC char *c_string_remove_sub_range_token (char *str, const char token, unsigned int first, unsigned int last,
	char **sub);

#endif