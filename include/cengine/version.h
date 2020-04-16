#ifndef _CENGINE_VERSION_H_
#define _CENGINE_VERSION_H_

#define VERSION                 "0.9.3"
#define VERSION_NAME            "Pre-Release 0.9.3"
#define VERSION_DATE			"15/04/2020"
#define VERSION_TIME			"23:46 CST"
#define VERSION_AUTHOR			"Erick Salas"

// print full cengine version information 
extern void version_print_full (void);

// print the version id
extern void version_print_version_id (void);

// print the version name
extern void version_print_version_name (void);

#endif