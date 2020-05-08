#ifndef _CENGINE_UI_LAYOUT_VERTICAL_H_
#define _CENGINE_UI_LAYOUT_VERTICAL_H_

#include "cengine/types/types.h"

#include "cengine/collections/dlist.h"

#include "cengine/renderer.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"

#define VERTICAL_LAYOUT_DEFAULT_SCROLL          5

struct _Renderer;

typedef struct VerticalLayout {

    // usefull reference to the window's renderer
    struct _Renderer *renderer;

    UITransform *transform;

    DoubleList *ui_elements;
    u32 curr_element_height;

    // constraints
    u32 element_height;
    u32 y_padding;

    // options
    bool scrolling;             // enable / disable scrolling
    u32 scroll_sensitivity;     // how fast the elements move

} VerticalLayout;

extern void ui_layout_vertical_delete (void *horizontal_ptr);

// sets a preffered height for your elements
// if there are more elements than vertical layout height / element height cna manage
// scrolling needs to be set, otherwise, remaining elements wont be displayed
// if this option is not set, the element height will be manage automatically
extern void ui_layout_vertical_set_element_height (VerticalLayout *vertical, u32 height);

// sets the y padding between the elements
// the default padidng is 0, so no space between elements
extern void ui_layout_vertical_set_element_padding (VerticalLayout *vertical, u32 y_padding);

// enables / disbale scrolling in the vertical layout
extern void ui_layout_vertical_toggle_scrolling (VerticalLayout *vertical, bool enable);

// sets how fast the elements move when scrolling, the default is a value of 5
extern void ui_layout_vertical_set_scroll_sensitivity (VerticalLayout *vertical, u32 sensitivity);

// creates a new horizontal layout
// x and y for position, w and h for dimensions
extern VerticalLayout *ui_layout_vertical_create (i32 x, i32 y, u32 w, u32 h, struct _Renderer *renderer);

// updates ALL the layout group's elements positions
// called automatically every time a new element is added or removed
extern void ui_layout_vertical_update (VerticalLayout *vertical);

// adds a new elemenet in the specified pos of the vertical layout group
// returns 0 on success, 1 on error
extern u8 ui_layout_vertical_add_at_pos (VerticalLayout *vertical, UIElement *ui_element, u32 pos);

// adds a new element to the end of the vertical layout group
// this method avoids updating all the other elements positions as well
// returns 0 on success, 1 on error
extern u8 ui_layout_vertical_add_at_end (VerticalLayout *vertical, UIElement *ui_element);

// returns the ui element that is at the required position in the layout
extern UIElement *ui_layout_vertical_get_element_at (VerticalLayout *vertical, unsigned int pos);

// removes an element from the vertical layout group
extern void ui_layout_vertical_remove (VerticalLayout *vertical, UIElement *ui_element);

#endif