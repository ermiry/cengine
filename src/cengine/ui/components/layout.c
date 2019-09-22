#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/ui/position.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/layout.h"

static LayoutGroup *ui_layout_group_new (void) {

    LayoutGroup *layout_group = (LayoutGroup *) malloc (sizeof (LayoutGroup));
    if (layout_group) {
        memset (layout_group, 0, sizeof (LayoutGroup));
        layout_group->transform = NULL;
        layout_group->bottom = false;
        layout_group->elements = NULL;
        layout_group->elements_max_display = -1;
    }

    return layout_group;

}

void ui_layout_group_delete (void *layout_group_ptr) {

    if (layout_group_ptr) {
        LayoutGroup *layout_group = (LayoutGroup *) layout_group_ptr;
        ui_transform_component_delete (layout_group->transform);
        dlist_delete (layout_group->elements);
    }

}

// creates a new layout group
LayoutGroup *ui_layout_group_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos) {

    LayoutGroup *layout_group = ui_layout_group_new ();
    if (layout_group) {
        layout_group->transform = ui_transform_component_create (x, y, w, h);
        ui_transform_component_set_pos (layout_group->transform, NULL, pos, false);
        layout_group->elements = dlist_init (ui_transform_component_delete_dummy, NULL);
    }

    return layout_group;

}

// sets the layout group's elements options
// max display: the max number of elements to display; -1 for no limit
// offset: distance between elements
void ui_layout_group_set_options (LayoutGroup *layout_group, int elements_max_display, u32 elements_offset) {

    if (layout_group) {
        layout_group->elements_max_display = elements_max_display;
        layout_group->elements_offset = elements_offset;
    }

}

// updates the layout group's elements positions
// called automatically every time a new element is added or removed
void ui_layout_group_update (LayoutGroup *layout_group) {

    if (layout_group) {
        if (layout_group->elements->size > 0) {
            // get the height for every element
            u32 height = layout_group->transform->rect.h / layout_group->elements->size;

            u32 offset = 0;
            UITransform *transform = NULL;
            for (ListElement *le = dlist_start (layout_group->elements); le; le = le->next) {
                transform = (UITransform *) le->data;
                transform->rect.w = layout_group->transform->rect.w;
                transform->rect.h = height;

                transform->rect.x = layout_group->transform->rect.x;
                transform->rect.y = layout_group->transform->rect.y + offset;
                offset += height;
            }
        }
    }

}

// adds a new element to the layout group
void ui_layout_group_add (LayoutGroup *layout_group, UITransform *element_trans) {
    
    if (layout_group && element_trans) {
        dlist_insert_after (layout_group->elements, dlist_end (layout_group->elements), element_trans);
        ui_layout_group_update (layout_group);
    }

}

// removes an element from the layout group
void ui_layout_group_remove (LayoutGroup *layout_group, UITransform *element_trans) {

    if (layout_group && element_trans) {
        // FIXME: remove the element!
        ui_layout_group_update (layout_group);
    }

}