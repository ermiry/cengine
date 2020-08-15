#ifndef _CENGINE_CONFIG_H_
#define _CENGINE_CONFIG_H_

#define CENGINE_EXPORT 			extern	// to be used in the application
#define CENGINE_PRIVATE			extern	// to be used by internal cengine methods
#define CENGINE_PUBLIC 			extern	// used by private methods but can be used by the application

#define DEPRECATED(func) func __attribute__ ((deprecated))

#endif