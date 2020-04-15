#ifndef _CERVER_ADMIN_H_
#define _CERVER_ADMIN_H_

#include "cengine/types/string.h"

// serialized admin credentials
struct _SAdminCredentials {

	SStringM username;
	SStringM password;

};

typedef struct _SAdminCredentials SAdminCredentials;

#endif