#include <stdio.h>

#include "client/version.h"

// print full version information 
void cerver_client_version_print_full (void) {

    printf ("\n\nCerver Client Version: %s\n", CLIENT_VERSION_NAME);
    printf ("Release Date & time: %s - %s\n", CLIENT_VERSION_DATE, CLIENT_VERSION_TIME);
    printf ("Author: %s\n\n", CLIENT_VERSION_AUTHOR);

}

// print the version id
void cerver_client_version_print_version_id (void) {

    printf ("\n\nCerver Client Version ID: %s\n", CLIENT_VERSION);

}

// print the version name
void cerver_client_version_print_version_name (void) {

    printf ("\n\nCerver Client Version: %s\n", CLIENT_VERSION_NAME);

}