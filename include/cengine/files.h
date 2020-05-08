#ifndef _CENGINE_FILES_H_
#define _CENGINE_FILES_H_

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "cengine/collections/dlist.h"

#include "cengine/utils/json.h"

// reads eachone of the file's lines into a newly created string and returns them inside a dlist
extern DoubleList *file_get_lines (const char *filename);

// returns an allocated string with the file extensio
// NULL if no file extension
extern char *files_get_file_extension (const char *filename);

// returns a list of strings containg the names of all the files in the directory
extern DoubleList *files_get_from_dir (const char *dir);

// opens a file and returns it as a FILE
extern FILE *file_open_as_file (const char *filename, const char *modes, struct stat *filestatus);

// opens and reads a file into a buffer
// sets file size to the amount of bytes read
extern char *file_read (const char *filename, int *file_size);

// opens a file as a file descriptor (fd)
// returns fd on success, -1 on error
// extern int file_open_as_fd (const char *filename, struct stat *filestatus);

extern json_value *file_json_parse (const char *filename);

#endif