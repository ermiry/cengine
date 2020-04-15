#ifndef _CENGINE_UI_POSITION_H_
#define _CENGINE_UI_POSITION_H_

#include <stdbool.h>

#include "cengine/renderer.h"

#include "cengine/ui/types/types.h"

struct _Renderer;

extern void ui_position_update (struct _Renderer *renderer, void *transform_ptr, UIRect *ref_rect, bool offset);

// updates the transform's position relative to be relative to its parent (x && y = 0)
extern void ui_position_update_to_parent (void *parent_transform_ptr, void *transform_ptr, bool offset);

#endif