#ifndef _CENGINE_VERSION_H_
#define _CENGINE_VERSION_H_

#define VERSION                 "0.9.4"
#define VERSION_NAME            "Pre-Release 0.9.4"
#define VERSION_DATE			"07/05/2020"
#define VERSION_TIME			"19:20 CST"
#define VERSION_AUTHOR			"Erick Salas"

// print full cengine version information 
extern void cengine_version_print_full (void);

// print the version id
extern void cengine_version_print_version_id (void);

// print the version name
extern void cengine_version_print_version_name (void);

#endif