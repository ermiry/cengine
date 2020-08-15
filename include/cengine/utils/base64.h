#ifndef _UTILS_BASE64_H_
#define _UTILS_BASE64_H_

#include <stddef.h>

#include "cengine/config.h"

CENGINE_PUBLIC char *base64_encode (size_t *enclen, size_t len, unsigned char *data);

CENGINE_PUBLIC unsigned char *base64_decode (size_t *declen, size_t len, char *data);

#endif
