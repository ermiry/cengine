#ifndef _CLIENT_INPUT_H_
#define _CLIENT_INPUT_H_

#include "client/config.h"

CLIENT_PUBLIC void input_clean_stdin (void);

// returns a newly allocated c string
CLIENT_PUBLIC char *input_get_line (void);

CLIENT_PUBLIC unsigned int input_password (char *password);

#endif