#include <stdio.h>

#include "cengine/version.h"

// print full cengine version information 
void cengine_version_print_full (void) {

    printf ("\nCengine Version: %s\n", CENGINE_VERSION_NAME);
    printf ("Release Date & time: %s - %s\n", CENGINE_VERSION_DATE, CENGINE_VERSION_TIME);
    printf ("Author: %s\n\n", CENGINE_VERSION_AUTHOR);

}

// print the version id
void cengine_version_print_version_id (void) {

    printf ("\n\nCengine Version ID: %s\n", CENGINE_VERSION);

}

// print the version name
void cengine_version_print_version_name (void) {

    printf ("\n\nCengine Version: %s\n", CENGINE_VERSION_NAME);

}