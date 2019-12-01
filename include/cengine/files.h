#ifndef _CENGINE_FILES_H_
#define _CENGINE_FILES_H_

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "cengine/utils/json.h"

// returns an allocated string with the file extensio
// NULL if no file extension
extern char *files_get_file_extension (const char *filename);

// opens a file with the given modes and retur a file ptr
extern FILE *file_open (const char *filename, const char *modes, struct stat *filestatus);

// opens and reads a file into a buffer
extern char *file_read (const char *filename, int *file_size);

// parse a json file into json values
extern json_value *file_json_parse (const char *filename);

#endif