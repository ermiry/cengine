#ifndef _CENGIN_FILE_H_
#define _CENGIN_FILE_H_

#include <stdio.h>

#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/stat.h>

#include "cengine/utils/json.h"

// opens a file with the given modes and retur a file ptr
extern FILE *file_open (const char *filename, const char *modes, struct stat *filestatus);

// opens and reads a file into a buffer
extern char *file_read (const char *filename, int *file_size);

// parse a json file into json values
json_value *file_json_parse (const char *filename);

#endif