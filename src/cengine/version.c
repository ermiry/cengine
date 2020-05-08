#include <stdio.h>

#include "cengine/version.h"

// print full cengine version information 
void cengine_version_print_full (void) {

    printf ("\n\nCengine Version: %s\n", VERSION_NAME);
    printf ("Release Date & time: %s - %s\n", VERSION_DATE, VERSION_TIME);
    printf ("Author: %s\n\n", VERSION_AUTHOR);

}

// print the version id
void cengine_version_print_version_id (void) {

    printf ("\n\nCengine Version ID: %s\n", VERSION);

}

// print the version name
void cengine_version_print_version_name (void) {

    printf ("\n\nCengine Version: %s\n", VERSION_NAME);

}