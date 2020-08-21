#ifndef _CENGINE_UI_LAYOUT_HORIZONTAL_H_
#define _CENGINE_UI_LAYOUT_HORIZONTAL_H_

#include "cengine/types/types.h"

#include "cengine/collections/dlist.h"

#include "cengine/renderer.h"
#include "cengine/events.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/panel.h"

#define HORIZONTAL_LAYOUT_DEFAULT_SCROLL          5

struct _Renderer;
struct _Panel;

typedef struct HorizontalLayout {

    // usefull reference to the window's renderer
    struct _Renderer *renderer;

    // usefull reference to the parent panel
    // struct _Panel *panel;

    UITransform *transform;

    DoubleList *ui_elements;
    u32 curr_element_width;

    // constraints
    u32 element_width;
    u32 x_padding;

    bool scrolling;             // enable / disable scrolling
    u32 scroll_sensitivity;     // how fast the elements move
    EventAction *event_scroll_up;
    EventAction *event_scroll_down;

} HorizontalLayout;

CENGINE_PRIVATE void ui_layout_horizontal_delete (void *horizontal_ptr);

// get the amount of elements that are inside the horizontal layout
CENGINE_PUBLIC size_t ui_layout_horizontal_get_elements_count (HorizontalLayout *horizontal);

// sets a preffered width for your elements
// if there are more elements than horizontal layout width / element width,
// scrolling needs to be set, otherwise, remaining elements wont be displayed
// if this option is not set, the element width will be manage automatically
CENGINE_PUBLIC void ui_layout_horizontal_set_element_width (HorizontalLayout *horizontal, u32 width);

// sets the x padding between the elements
// the default padidng is 0, so no space between elements
CENGINE_PUBLIC void ui_layout_horizontal_set_element_padding (HorizontalLayout *horizontal, u32 x_padding);

// enables / disbale scrolling in the horizontal layout
CENGINE_PUBLIC void ui_layout_horizontal_toggle_scrolling (HorizontalLayout *horizontal, bool enable);

// sets how fast the elements move when scrolling, the default is a value of 5
CENGINE_PUBLIC void ui_layout_horizontal_set_scroll_sensitivity (HorizontalLayout *horizontal, u32 sensitivity);

// creates a new horizontal layout
// x and y for position, w and h for dimensions
CENGINE_PUBLIC HorizontalLayout *ui_layout_horizontal_create (i32 x, i32 y, u32 w, u32 h, 
    struct _Renderer *renderer);

// adds a new elemenet in the specified pos of the horizontal layout group
// returns 0 on success, 1 on error
CENGINE_PUBLIC u8 ui_layout_horizontal_add_at_pos (HorizontalLayout *horizontal, UIElement *ui_element, u32 pos);

// adds a new element to the end of the horizontal layout group
// this method avoids updating all the other elements positions as well
// returns 0 on success, 1 on error
CENGINE_PUBLIC u8 ui_layout_horizontal_add_at_end (HorizontalLayout *horizontal, UIElement *ui_element);

// returns the ui element that is at the required position in the layout
CENGINE_PUBLIC UIElement *ui_layout_horizontal_get_element_at (HorizontalLayout *horizontal, unsigned int pos);

// removes an element from the horizontal layout group
CENGINE_PUBLIC u8 ui_layout_horizontal_remove (HorizontalLayout *horizontal, UIElement *ui_element);

#endif