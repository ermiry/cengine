#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"

UITransform *ui_transform_component_new (void) {

    UITransform *transform = (UITransform *) malloc (sizeof (UITransform));
    if (transform) memset (transform, 0, sizeof (UITransform));
    return transform;

}

void ui_transform_component_delete (void *transform_ptr) {

    if (transform_ptr) free (transform_ptr);

}

void ui_transform_component_delete_dummy (void *transform_ptr) {}

void ui_transform_component_set_pos (UITransform *transform, UIRect *ref_rect, UIPosition pos, bool offset) {

    if (transform) {
        transform->pos = pos;
        ui_position_update (transform, ref_rect, offset);
    } 

}

void ui_transform_component_set_values (UITransform *transform,
    int x, int y, int w, int h) {

    if (transform) {
        transform->rect.x = x;
        transform->rect.y = y;
        transform->rect.w = w;
        transform->rect.h = h;
    }

}

void ui_transform_component_set_scale (UITransform *transform, int x_scale, int y_scale) {

    if (transform) {
        transform->x_scale = x_scale;
        transform->y_scale = y_scale;
    }

}

UITransform *ui_transform_component_create (int x, int y, int w, int h) {

    UITransform *transform = ui_transform_component_new ();
    if (transform) {
        transform->pos = UI_POS_FREE;

        transform->rect.x = x;
        transform->rect.y = y;
        transform->rect.w = w;
        transform->rect.h = h;
    }

    return transform;

}