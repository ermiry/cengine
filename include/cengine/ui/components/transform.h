#ifndef _CENGINE_UI_COMPONENT_TRANSFORM_
#define _CENGINE_UI_COMPONENT_TRANSFORM_

#include <stdbool.h>

#include "cengine/renderer.h"

#include "cengine/ui/types/types.h"
#include "cengine/ui/ui.h"
#include "cengine/ui/position.h"

struct _Renderer;

struct _UITransform {

    UIPosition pos;
    UIRect rect;
    int x_scale, y_scale;

    // 06/02/2020
    int x_offset, y_offset;

};

typedef struct _UITransform UITransform;

extern UITransform *ui_transform_component_new (void);

extern void ui_transform_component_delete (void *transform_ptr);

extern void ui_transform_component_delete_dummy (void *transform_ptr);

extern void ui_transform_component_set_pos (UITransform *transform, struct _Renderer *renderer, UIRect *ref_rect, UIPosition pos, bool offset);

extern void ui_transform_component_set_values (UITransform *transform,
    int x, int y, int w, int h);

extern void ui_transform_component_set_scale (UITransform *transform, int x_scale, int y_scale);

extern UITransform *ui_transform_component_create (int x, int y, int w, int h);

#endif