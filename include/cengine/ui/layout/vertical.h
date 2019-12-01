#ifndef _CENGINE_UI_LAYOUT_VERTICAL_H_
#define _CENGINE_UI_LAYOUT_VERTICAL_H_

#include "cengine/types/types.h"

#include "cengine/collections/dlist.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"

typedef struct VerticalLayout {

    UITransform *transform;

    DoubleList *ui_elements;
    u32 max_n_ui_elements;
    u32 curr_n_ui_elements;

    u32 cell_width;
    u32 cell_height;

    int elements_max_display;
    bool bottom;
    u32 elements_offset;            // offset between elements

} VerticalLayout;

extern void ui_layout_vertical_delete (void *horizontal_ptr);

// sets the layout group's elements options
// max display: the max number of elements to display; -1 for no limit
// offset: distance between elements
extern void ui_layout_vertical_set_options (VerticalLayout *vertical, int elements_max_display, u32 elements_offset);

// creates a new horizontal layout
// x and y for position, w and h for dimensions
extern VerticalLayout *ui_layout_vertical_create (i32 x, i32 y, u32 w, u32 h);

// updates the layout group's elements positions
// called automatically every time a new element is added or removed
extern void ui_layout_vertical_update (VerticalLayout *vertical);

// adds a new element to the layout group
extern void ui_layout_vertical_add (VerticalLayout *vertical, UIElement *ui_element);

// removes an element from the layout group
extern void ui_layout_vertical_remove (VerticalLayout *vertical, UIElement *ui_element);

#endif