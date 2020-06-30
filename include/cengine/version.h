#ifndef _CENGINE_VERSION_H_
#define _CENGINE_VERSION_H_

#define CENGINE_VERSION                 "0.9.5rc-2"
#define CENGINE_VERSION_NAME            "Pre-Release 0.9.5rc-2"
#define CENGINE_VERSION_DATE			"30/06/2020"
#define CENGINE_VERSION_TIME			"03:02 CST"
#define CENGINE_VERSION_AUTHOR			"Erick Salas"

// print full cengine version information 
extern void cengine_version_print_full (void);

// print the version id
extern void cengine_version_print_version_id (void);

// print the version name
extern void cengine_version_print_version_name (void);

#endif