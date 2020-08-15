#ifndef _CENGINE_ASSETS_H_
#define _CENGINE_ASSETS_H_

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/config.h"

CENGINE_EXPORT const String *cengine_assets_path;

// sets the path for the assets folder
CENGINE_EXPORT void cengine_assets_set_path (const char *pathname);

CENGINE_EXPORT const String *ui_default_assets_path;

// sets the location of cengine's default ui assets
CENGINE_EXPORT void cengine_default_assets_set_path (const char *pathname);

// loads cengine's default ui assets
CENGINE_PUBLIC u8 ui_default_assets_load (void);

CENGINE_PRIVATE u8 assets_end (void);

#endif