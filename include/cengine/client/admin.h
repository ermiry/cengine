#ifndef _CLIENT_ADMIN_H_
#define _CLIENT_ADMIN_H_

#include "client/types/string.h"

// serialized admin credentials
struct _SAdminCredentials {

	SStringM username;
	SStringM password;

};

typedef struct _SAdminCredentials SAdminCredentials;

#endif