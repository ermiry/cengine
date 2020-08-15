#ifndef _CLIENT_CONFIG_H_
#define _CLIENT_CONFIG_H_

#define CLIENT_EXPORT 			extern	// to be used in the application
#define CLIENT_PRIVATE			extern	// to be used by internal client methods
#define CLIENT_PUBLIC 			extern	// used by private methods but can be used by the application

#define DEPRECATED(func) func __attribute__ ((deprecated))

#endif