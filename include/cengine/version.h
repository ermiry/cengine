#ifndef _CENGINE_VERSION_H_
#define _CENGINE_VERSION_H_

#define VERSION                 "0.9.1"
#define VERSION_NAME            "Pre-Release 0.9.1"
#define VERSION_DATE			"20/01/2020"
#define VERSION_TIME			"12:19 CST"
#define VERSION_AUTHOR			"Erick Salas"

// print full cengine version information 
extern void version_print_full (void);

// print the version id
extern void version_print_version_id (void);

// print the version name
extern void version_print_version_name (void);

#endif