#ifndef _CENGINE_UI_LAYOUT_VERTICAL_H_
#define _CENGINE_UI_LAYOUT_VERTICAL_H_

#include "cengine/types/types.h"

#include "cengine/collections/dlist.h"

#include "cengine/renderer.h"
#include "cengine/events.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"

#define VERTICAL_LAYOUT_DEFAULT_SCROLL          5

struct _Renderer;
struct _UIElement;
struct _UITransform;

typedef struct VerticalLayout {

    // usefull reference to the window's renderer
    struct _Renderer *renderer;

    struct _UITransform *transform;

    DoubleList *ui_elements;
    u32 curr_element_height;

    // constraints
    u32 element_height;
    u32 y_padding;

    // options
    bool scrolling;             // enable / disable scrolling
    u32 scroll_sensitivity;     // how fast the elements move
    EventAction *event_scroll_up;
    EventAction *event_scroll_down;

} VerticalLayout;

CENGINE_PRIVATE void ui_layout_vertical_delete (void *horizontal_ptr);

// get the amount of elements that are inside the vertical layout
CENGINE_PUBLIC size_t ui_layout_vertical_get_elements_count (VerticalLayout *vertical);

// sets a preffered height for your elements
// if there are more elements than vertical layout height / element height cna manage
// scrolling needs to be set, otherwise, remaining elements wont be displayed
// if this option is not set, the element height will be manage automatically
CENGINE_PUBLIC void ui_layout_vertical_set_element_height (VerticalLayout *vertical, u32 height);

// sets the y padding between the elements
// the default padidng is 0, so no space between elements
CENGINE_PUBLIC void ui_layout_vertical_set_element_padding (VerticalLayout *vertical, u32 y_padding);

// enables / disbale scrolling in the vertical layout
CENGINE_PUBLIC void ui_layout_vertical_toggle_scrolling (VerticalLayout *vertical, bool enable);

// sets how fast the elements move when scrolling, the default is a value of 5
CENGINE_PUBLIC void ui_layout_vertical_set_scroll_sensitivity (VerticalLayout *vertical, u32 sensitivity);

// creates a new horizontal layout
// x and y for position, w and h for dimensions
CENGINE_PUBLIC VerticalLayout *ui_layout_vertical_create (i32 x, i32 y, u32 w, u32 h, struct _Renderer *renderer);

// updates ALL the layout group's elements positions
// called automatically every time a new element is added or removed
CENGINE_PUBLIC void ui_layout_vertical_update (VerticalLayout *vertical);

// adds a new elemenet in the specified pos of the vertical layout group
// returns 0 on success, 1 on error
CENGINE_PUBLIC u8 ui_layout_vertical_add_at_pos (VerticalLayout *vertical, struct _UIElement *ui_element, u32 pos);

// adds a new element to the end of the vertical layout group
// this method avoids updating all the other elements positions as well
// returns 0 on success, 1 on error
CENGINE_PUBLIC u8 ui_layout_vertical_add_at_end (VerticalLayout *vertical, struct _UIElement *ui_element);

// returns the ui element that is at the required position in the layout
CENGINE_PUBLIC struct _UIElement *ui_layout_vertical_get_element_at (VerticalLayout *vertical, unsigned int pos);

// removes an element from the vertical layout group
CENGINE_PUBLIC u8 ui_layout_vertical_remove (VerticalLayout *vertical, struct _UIElement *ui_element);

#endif