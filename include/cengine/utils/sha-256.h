#ifndef _SHA_256_H_
#define _SHA_256_H_

#include <stdlib.h>
#include <stdint.h>

#include "cengine/config.h"

CENGINE_PUBLIC void sha_256_calc (uint8_t hash[32], const void *input, size_t len);

CENGINE_PUBLIC void sha_256_hash_to_string (char string[65], const uint8_t hash[32]);

#endif