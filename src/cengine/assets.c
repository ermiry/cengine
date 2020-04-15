#include <stdlib.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/utils/log.h"

const String *cengine_assets_path = NULL;

// sets the path for the assets folder
void cengine_assets_set_path (const char *pathname) {

    if (cengine_assets_path) str_delete ((String *) cengine_assets_path);
    cengine_assets_path = pathname ? str_new (pathname) : NULL;

}

const String *ui_default_assets_path = NULL;

// sets the location of cengine's default ui assets
void cengine_default_assets_set_path (const char *pathname) {

    str_delete ((String *) ui_default_assets_path);
    ui_default_assets_path = pathname ? str_new (pathname) : NULL;

}

// loads cengine's default ui assets
u8 ui_default_assets_load (void) {

    u8 retval = 1;

    if (ui_default_assets_path) {
        // printf ("%s\n", ui_default_assets_path->str);

        retval = 0;
    }

    else {
        cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, 
            "Failed to load cengine's default assets - ui default assets path set to NULL!");
    }

    return retval;

}

u8 assets_end (void) {

    str_delete ((String *) cengine_assets_path);

    str_delete ((String *) ui_default_assets_path);

    return 0;

}