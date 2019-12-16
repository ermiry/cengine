#ifndef _CENGINE_UI_POSITION_H_
#define _CENGINE_UI_POSITION_H_

#include <stdbool.h>

#include "cengine/renderer.h"

#include "cengine/ui/types/types.h"

struct _Renderer;

extern void ui_position_update (struct _Renderer *renderer, void *transform_ptr, UIRect *ref_rect, bool offset);

#endif