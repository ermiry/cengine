#ifndef _CENGINE_ASSETS_H_
#define _CENGINE_ASSETS_H_

#include "cengine/types/types.h"
#include "cengine/types/string.h"

extern const String *cengine_assets_path;

// sets the path for the assets folder
extern void cengine_assets_set_path (const char *pathname);

// sets the location of cengine's default ui assets
extern void ui_default_assets_set_path (const char *pathname);

// loads cengine's default ui assets
extern u8 ui_default_assets_load (void);

extern void assets_end (void);

#endif