#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"

#include "cengine/collections/dlist.h"

#include "cengine/ui/components/transform.h"
#include "cengine/ui/layout/vertical.h"

static VerticalLayout *ui_layout_vertical_new (void) {

    VerticalLayout *vertical = (VerticalLayout *) malloc (sizeof (VerticalLayout));
    if (vertical) {
        memset (vertical, 0, sizeof (VerticalLayout));
        vertical->transform = NULL;
        vertical->ui_elements = NULL;

        vertical->bottom = false;
    }

    return vertical;

}

void ui_layout_vertical_delete (void *vertical_ptr) {

    if (vertical_ptr) {
        VerticalLayout *vertical = (VerticalLayout *) vertical_ptr;

        ui_transform_component_delete (vertical->transform);
        dlist_delete (vertical->ui_elements);

        free (vertical);
    }

}

// sets the layout group's elements options
// max display: the max number of elements to display; -1 for no limit
// offset: distance between elements
void ui_layout_vertical_set_options (VerticalLayout *vertical, int elements_max_display, u32 elements_offset) {

    if (vertical) {
        vertical->elements_max_display = elements_max_display;
        vertical->elements_offset = elements_offset;
    }

}

// creates a new horizontal layout
// x and y for position, w and h for dimensions
VerticalLayout *ui_layout_vertical_create (i32 x, i32 y, u32 w, u32 h) {

    VerticalLayout *vertical = ui_layout_vertical_new ();

    vertical->transform = ui_transform_component_create (x, y, w, h);
    // ui_transform_component_set_pos (vertical->transform, NULL, NULL, pos, false);
    vertical->ui_elements = dlist_init (ui_element_delete_dummy, NULL);

    return vertical;

}

// updates the layout group's elements positions
// called automatically every time a new element is added or removed
void ui_layout_vertical_update (VerticalLayout *vertical) {

    if (vertical) {
        if (vertical->ui_elements->size > 0) {
            // get the height for every element
            u32 height = vertical->transform->rect.h / vertical->ui_elements->size;

            u32 offset = 0;
            UITransform *transform = NULL;
            for (ListElement *le = dlist_start (vertical->ui_elements); le; le = le->next) {
                transform = (UITransform *) le->data;
                transform->rect.w = vertical->transform->rect.w;
                transform->rect.h = height;

                transform->rect.x = vertical->transform->rect.x;
                transform->rect.y = vertical->transform->rect.y + offset;
                offset += height;
            }
        }
    }

}

// adds a new element to the layout group
void ui_layout_vertical_add (VerticalLayout *vertical, UIElement *ui_element) {
    
    if (vertical && ui_element) {
        dlist_insert_after (vertical->ui_elements, dlist_end (vertical->ui_elements), ui_element);
        ui_layout_vertical_update (vertical);
    }

}

// removes an element from the layout group
void ui_layout_vertical_remove (VerticalLayout *vertical, UIElement *ui_element) {

    if (vertical && ui_element) {
        // FIXME: remove the element!
        ui_layout_vertical_update (vertical);
    }

}