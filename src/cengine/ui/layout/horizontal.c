#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"

#include "cengine/collections/dlist.h"

#include "cengine/events.h"

#include "cengine/ui/components/transform.h"
#include "cengine/ui/layout/horizontal.h"
#include "cengine/ui/panel.h"

static void ui_layout_horizontal_scroll_up (void *event_data);
static void ui_layout_horizontal_scroll_down (void *event_data);

static HorizontalLayout *ui_layout_horizontal_new (void) {

    HorizontalLayout *horizontal = (HorizontalLayout *) malloc (sizeof (HorizontalLayout));
    if (horizontal) {
        memset (horizontal, 0, sizeof (HorizontalLayout));

        horizontal->renderer = NULL;
        // horizontal->panel = NULL;

        horizontal->transform = NULL;
        horizontal->ui_elements = NULL;

        horizontal->event_scroll_up = NULL;
        horizontal->event_scroll_down = NULL;
    }

    return horizontal;

}

void ui_layout_horizontal_delete (void *horizontal_ptr) {

    if (horizontal_ptr) {
        HorizontalLayout *horizontal = (HorizontalLayout *) horizontal_ptr;

        ui_transform_component_delete (horizontal->transform);
        dlist_delete (horizontal->ui_elements);

        if (horizontal->event_scroll_up) {
            cengine_event_unregister (horizontal->event_scroll_up);
        }

        if (horizontal->event_scroll_down) {
            cengine_event_unregister (horizontal->event_scroll_down);
        }

        free (horizontal);
    }

}

// get the amount of elements that are inside the horizontal layout
size_t ui_layout_horizontal_get_elements_count (HorizontalLayout *horizontal) {

    return horizontal ? horizontal->ui_elements->size : 0;

}

// sets a preffered width for your elements
// if there are more elements than horizontal layout width / element width,
// scrolling needs to be set, otherwise, remaining elements wont be displayed
// if this option is not set, the element width will be manage automatically
void ui_layout_horizontal_set_element_width (HorizontalLayout *horizontal, u32 width) {

    if (horizontal) {
        horizontal->element_width = width;
    }

}

// sets the x padding between the elements
// the default padidng is 0, so no space between elements
void ui_layout_horizontal_set_element_padding (HorizontalLayout *horizontal, u32 x_padding) {

    if (horizontal) {
        horizontal->x_padding = x_padding;
    }

}

// enables / disbale scrolling in the horizontal layout
void ui_layout_horizontal_toggle_scrolling (HorizontalLayout *horizontal, bool enable) {

    if (horizontal) {
        // register this horizontal layout to listen for the scroll event
        horizontal->event_scroll_up = cengine_event_register (CENGINE_EVENT_SCROLL_UP, ui_layout_horizontal_scroll_up, horizontal);
        horizontal->event_scroll_down = cengine_event_register (CENGINE_EVENT_SCROLL_DOWN, ui_layout_horizontal_scroll_down, horizontal);

        horizontal->scroll_sensitivity = HORIZONTAL_LAYOUT_DEFAULT_SCROLL;
        horizontal->scrolling = enable;
    }

}

// sets how fast the elements move when scrolling, the default is a value of 5
void ui_layout_horizontal_set_scroll_sensitivity (HorizontalLayout *horizontal, u32 sensitivity) {

    if (horizontal) horizontal->scroll_sensitivity = sensitivity;

}

// creates a new horizontal layout
// x and y for position, w and h for dimensions
HorizontalLayout *ui_layout_horizontal_create (i32 x, i32 y, u32 w, u32 h, 
    Renderer *renderer) {

    HorizontalLayout *horizontal = ui_layout_horizontal_new ();
    if (horizontal) {
        horizontal->renderer = renderer;
        // horizontal->panel = panel;

        horizontal->transform = ui_transform_component_create (x, y, w, h);
        horizontal->ui_elements = dlist_init (ui_element_delete_dummy, ui_element_comparator);
    }

    return horizontal;

}

// updates ALL the layout group's elements positions
// called automatically every time a new element is added or removed
static void ui_layout_horizontal_update (HorizontalLayout *horizontal) {

    if (horizontal) {
        if (horizontal->ui_elements->size > 0) {
            u32 padding = horizontal->x_padding;
            u32 total_padding = padding * (horizontal->ui_elements->size - 1);

            u32 horizontal_width = horizontal->transform->rect.w - total_padding;

            // get the height for every element
            horizontal->curr_element_width = horizontal->element_width == 0 ? 
                horizontal_width / horizontal->ui_elements->size :
                horizontal->element_width;

            bool first;
            u32 offset = 0;

            UIElement *ui_element = NULL;
            UITransform *transform = NULL;
            for (ListElement *le = dlist_start (horizontal->ui_elements); le; le = le->next) {
                ui_element = (UIElement *) le->data;
                transform = ui_element->transform;
                transform->rect.w = horizontal->curr_element_width;
                transform->rect.h = horizontal->transform->rect.h;

                // transform->rect.y = horizontal->transform->rect.y;
                transform->rect.y = 0;
                if (first) {
                    // transform->rect.x = horizontal->transform->rect.x;
                    transform->rect.x = 0;
                    offset += horizontal->curr_element_width;
                    first = false;
                }

                else {
                    // transform->rect.x = horizontal->transform->rect.x + offset + padding;
                    transform->rect.x = offset + padding;
                    offset += (horizontal->curr_element_width + padding);
                } 

                // FIXME: 16/04/2020 -- 02:43 -- handle all ui elements
                switch (ui_element->type) {
                    case UI_PANEL: ui_panel_children_update_pos ((Panel *) ui_element->element); break;

                    case UI_TEXTBOX:
                        // ui_textbox_update_text_pos ((TextBox *) ui_element->element);
                        break;

                    // case UI_IMAGE: ui_image_update break;

                    default: break;
                }
            }
        }
    }

}

// adds a new elemenet in the specified pos of the horizontal layout group
// returns 0 on success, 1 on error
u8 ui_layout_horizontal_add_at_pos (HorizontalLayout *horizontal, UIElement *ui_element, u32 pos) {
    
    u8 retval = 1;

    if (horizontal && ui_element) {
        if (!dlist_insert_at (horizontal->ui_elements, ui_element, pos)) {
            ui_layout_horizontal_update (horizontal);
            retval = 0;
        }
    }

    return retval;

}

// adds a new element to the end of the horizontal layout group
// this method avoids updating all the other elements positions as well
// returns 0 on success, 1 on error
u8 ui_layout_horizontal_add_at_end (HorizontalLayout *horizontal, UIElement *ui_element) {
    
    u8 retval = 1;

    if (horizontal && ui_element) {
        if (!dlist_insert_after (horizontal->ui_elements, dlist_end (horizontal->ui_elements), ui_element)) {
            ui_layout_horizontal_update (horizontal);
            retval = 0;
        }
    }

    return retval;

}

// returns the ui element that is at the required position in the layout
UIElement *ui_layout_horizontal_get_element_at (HorizontalLayout *horizontal, unsigned int pos) {

    return horizontal ? (UIElement *) dlist_get_at (horizontal->ui_elements, pos) : NULL;

}

// removes an element from the horizontal layout group
u8 ui_layout_horizontal_remove (HorizontalLayout *horizontal, UIElement *ui_element) {

    u8 retval = 1;

    if (horizontal && ui_element) {
        if (dlist_remove (horizontal->ui_elements, ui_element, NULL)) {
            ui_layout_horizontal_update (horizontal);
            retval = 0;
        }
    }

    return retval;

}

#pragma region scrolling

static void ui_layout_horizontal_scroll_up (void *event_data) {

    if (event_data) {
        EventActionData *event_action_data = (EventActionData *) event_data;

        int *amount = (int *) event_action_data->event_data;
        HorizontalLayout *horizontal = (HorizontalLayout *) event_action_data->action_args;

        if (horizontal->renderer->window->mouse) {
            // check if the mouse is in the button
            if (mousePos.x >= horizontal->transform->rect.x && mousePos.x <= (horizontal->transform->rect.x + horizontal->transform->rect.w) && 
                mousePos.y >= horizontal->transform->rect.y && mousePos.y <= (horizontal->transform->rect.y + horizontal->transform->rect.h)) {
                // printf ("+%d\n", *amount);

                if (dlist_size (horizontal->ui_elements) > 0) {
                    // check if the elements fill all the panel to even allow scrolling
                    u32 total_elements_width = horizontal->curr_element_width * dlist_size (horizontal->ui_elements);
                    if (total_elements_width > horizontal->transform->rect.w) {
                        // check for the first element position
                        UIElement *first_element = (UIElement *) (dlist_start (horizontal->ui_elements)->data);
                        if (first_element->transform->rect.x < 0) {
                            u32 real_scroll = (*amount * horizontal->scroll_sensitivity);
                            i32 new_first_element_pos = first_element->transform->rect.x + (*amount * horizontal->scroll_sensitivity);
                            if (new_first_element_pos > 0) real_scroll = 0 - first_element->transform->rect.x;

                            UIElement *ui_element = NULL;
                            UITransform *transform = NULL;
                            for (ListElement *le = dlist_start (horizontal->ui_elements); le; le = le->next) {
                                ui_element = (UIElement *) le->data;
                                transform = ui_element->transform;

                                // transform->rect.x += (*amount * horizontal->scroll_sensitivity);
                                transform->rect.x += real_scroll;

                                switch (ui_element->type) {
                                    case UI_PANEL: ui_panel_children_update_pos ((Panel *) ui_element->element); break;
                                    
                                    default: break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

static void ui_layout_horizontal_scroll_down (void *event_data) {

    if (event_data) {
        EventActionData *event_action_data = (EventActionData *) event_data;

        int *amount = (int *) event_action_data->event_data;
        HorizontalLayout *horizontal = (HorizontalLayout *) event_action_data->action_args;

        if (horizontal->renderer->window->mouse) {
            // check if the mouse is in the button
            if (mousePos.x >= horizontal->transform->rect.x && mousePos.x <= (horizontal->transform->rect.x + horizontal->transform->rect.w) && 
                mousePos.y >= horizontal->transform->rect.y && mousePos.y <= (horizontal->transform->rect.y + horizontal->transform->rect.h)) {
                // printf ("+%d\n", *amount);

                if (dlist_size (horizontal->ui_elements) > 0) {
                    // check the pos of the last element
                    UIElement *last_element = (UIElement *) (dlist_end (horizontal->ui_elements)->data);
                    u32 edge = (0 + horizontal->transform->rect.w) - horizontal->curr_element_width;
                    if (last_element->transform->rect.x > edge) {
                        u32 real_scroll = (*amount * horizontal->scroll_sensitivity);
                        u32 new_last_element_pos = last_element->transform->rect.x + (*amount * horizontal->scroll_sensitivity);
                        if (new_last_element_pos < edge) real_scroll = edge - last_element->transform->rect.x;

                        UIElement *ui_element = NULL;
                        UITransform *transform = NULL;
                        for (ListElement *le = dlist_start (horizontal->ui_elements); le; le = le->next) {
                            ui_element = (UIElement *) le->data;
                            transform = ui_element->transform;

                            // transform->rect.x += (*amount * horizontal->scroll_sensitivity);
                            transform->rect.x += real_scroll;

                            switch (ui_element->type) {
                                case UI_PANEL: ui_panel_children_update_pos ((Panel *) ui_element->element); break;
                                
                                default: break;
                            }
                        }
                    }
                }
            }
        }
    }

}

#pragma endregion