#ifndef _CENGINE_UI_COMPONENT_LAYOUT_
#define _CENGINE_UI_COMPONENT_LAYOUT_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/ui/position.h"
#include "cengine/ui/components/transform.h"

typedef struct LayoutGroup {

    UITransform *transform;
    bool bottom;

    DoubleList *elements;
    int elements_max_display;
    u32 elements_offset;            // offset between elements

} LayoutGroup;

extern void ui_layout_group_delete (void *layout_group_ptr);

// creates a new layout group
extern LayoutGroup *ui_layout_group_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos);

// sets the layout group's elements options
// max display: the max number of elements to display; -1 for no limit
// offset: distance between elements
extern void ui_layout_group_set_options (LayoutGroup *layout_group, int elements_max_display, u32 elements_offset);

// updates the layout group's elements positions
// called automatically every time a new element is added or removed
extern void ui_layout_group_update (LayoutGroup *layout_group);

// adds a new element to the layout group
extern void ui_layout_group_add (LayoutGroup *layout_group, UITransform *element_trans);

// removes an element from the layout group
extern void ui_layout_group_remove (LayoutGroup *layout_group, UITransform *element_trans);

#endif