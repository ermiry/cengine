#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"

#include "cengine/collections/dlist.h"

#include "cengine/events.h"
#include "cengine/input.h"
#include "cengine/renderer.h"

#include "cengine/ui/components/transform.h"
#include "cengine/ui/layout/vertical.h"

#include "cengine/ui/textbox.h"
#include "cengine/ui/panel.h"

static void ui_layout_vertical_scroll_up (void *event_data);
static void ui_layout_vertical_scroll_down (void *event_data);

static VerticalLayout *ui_layout_vertical_new (void) {

    VerticalLayout *vertical = (VerticalLayout *) malloc (sizeof (VerticalLayout));
    if (vertical) {
        memset (vertical, 0, sizeof (VerticalLayout));

        vertical->renderer = NULL;

        vertical->transform = NULL;
        vertical->ui_elements = NULL;

        vertical->event_scroll_up = NULL;
        vertical->event_scroll_down = NULL;
    }

    return vertical;

}

void ui_layout_vertical_delete (void *vertical_ptr) {

    if (vertical_ptr) {
        VerticalLayout *vertical = (VerticalLayout *) vertical_ptr;

        ui_transform_component_delete (vertical->transform);
        dlist_delete (vertical->ui_elements);

        if (vertical->event_scroll_up) {
            cengine_event_unregister (vertical->event_scroll_up);
        }

        if (vertical->event_scroll_down) {
            cengine_event_unregister (vertical->event_scroll_down);
        }

        free (vertical);
    }

}

// get the amount of elements that are inside the vertical layout
size_t ui_layout_vertical_get_elements_count (VerticalLayout *vertical) {

    return vertical ? vertical->ui_elements->size : 0;

}

// sets a preffered height for your elements
// if there are more elements than vertical layout height / element height,
// scrolling needs to be set, otherwise, remaining elements wont be displayed
// if this option is not set, the element height will be manage automatically
void ui_layout_vertical_set_element_height (VerticalLayout *vertical, u32 height) {

    if (vertical) {
        vertical->element_height = height;
    }

}

// sets the y padding between the elements
// the default padidng is 0, so no space between elements
void ui_layout_vertical_set_element_padding (VerticalLayout *vertical, u32 y_padding) {

    if (vertical) {
        vertical->y_padding = y_padding;
    }

}

// enables / disbale scrolling in the vertical layout
void ui_layout_vertical_toggle_scrolling (VerticalLayout *vertical, bool enable) {

    if (vertical) {
        // register this vertical layout to listen for the scroll event
        vertical->event_scroll_up = cengine_event_register (CENGINE_EVENT_SCROLL_UP, ui_layout_vertical_scroll_up, vertical);
        vertical->event_scroll_down = cengine_event_register (CENGINE_EVENT_SCROLL_DOWN, ui_layout_vertical_scroll_down, vertical);

        vertical->scroll_sensitivity = VERTICAL_LAYOUT_DEFAULT_SCROLL;
        vertical->scrolling = enable;
    }

}

// sets how fast the elements move when scrolling, the default is a value of 5
void ui_layout_vertical_set_scroll_sensitivity (VerticalLayout *vertical, u32 sensitivity) {

    if (vertical) vertical->scroll_sensitivity = sensitivity;

}

// creates a new horizontal layout
// x and y for position, w and h for dimensions
VerticalLayout *ui_layout_vertical_create (i32 x, i32 y, u32 w, u32 h, Renderer *renderer) {

    VerticalLayout *vertical = ui_layout_vertical_new ();

    if (vertical) {
        vertical->renderer = renderer;

        vertical->transform = ui_transform_component_create (x, y, w, h);
        // ui_transform_component_set_pos (vertical->transform, NULL, NULL, pos, false);
        vertical->ui_elements = dlist_init (ui_element_delete_dummy, ui_element_comparator);
    }

    return vertical;

}

// updates ALL the layout group's elements positions
// called automatically every time a new element is added or removed
void ui_layout_vertical_update (VerticalLayout *vertical) {

    if (vertical) {
        if (vertical->ui_elements->size > 0) {
            u32 padding = vertical->y_padding;
            u32 total_padding = padding * (vertical->ui_elements->size - 1);

            u32 vertical_height = vertical->transform->rect.h - total_padding;

            // get the height for every element
            vertical->curr_element_height = vertical->element_height == 0 ? 
                vertical_height / vertical->ui_elements->size :
                vertical->element_height;

            bool first;
            u32 offset = 0;

            UIElement *ui_element = NULL;
            UITransform *transform = NULL;
            for (ListElement *le = dlist_start (vertical->ui_elements); le; le = le->next) {
                ui_element = (UIElement *) le->data;
                transform = ui_element->transform;
                transform->rect.w = vertical->transform->rect.w;
                transform->rect.h = vertical->curr_element_height;

                // transform->rect.x = vertical->transform->rect.x;
                transform->rect.x = 0;
                if (first) {
                    // transform->rect.y = vertical->transform->rect.y;
                    transform->rect.y = 0;
                    offset += vertical->curr_element_height;
                    first = false;
                }

                else {
                    transform->rect.y = offset + padding;
                    offset += (vertical->curr_element_height + padding);
                } 

                switch (ui_element->type) {
                    case UI_INPUT:
                        ui_input_field_placeholder_text_pos_update ((InputField *) ui_element->element);
                        ui_input_field_text_pos_update ((InputField *) ui_element->element);
                        break;

                    case UI_TEXTBOX:
                        ui_textbox_update_text_pos ((TextBox *) ui_element->element);
                        break;

                    default: break;
                }
            }
        }
    }

}

// adds a new elemenet in the specified pos of the vertical layout group
// returns 0 on success, 1 on error
u8 ui_layout_vertical_add_at_pos (VerticalLayout *vertical, UIElement *ui_element, u32 pos) {
    
    u8 retval = 1;

    if (vertical && ui_element) {
        if (!dlist_insert_at (vertical->ui_elements, ui_element, pos)) {
            ui_layout_vertical_update (vertical);
            retval = 0;
        }
    }

    return retval;

}

// adds a new element to the end of the vertical layout group
// this method avoids updating all the other elements positions as well
// returns 0 on success, 1 on error
u8 ui_layout_vertical_add_at_end (VerticalLayout *vertical, UIElement *ui_element) {
    
    u8 retval = 1;

    if (vertical && ui_element) {
        if (!dlist_insert_after (vertical->ui_elements, dlist_end (vertical->ui_elements), ui_element)) {
            // FIXME: 06/02/2020 -- we dont have to update all the elements when adding at the end
            ui_layout_vertical_update (vertical);
            retval = 0;
        }
    }

    return retval;

}

// returns the ui element that is at the required position in the layout
UIElement *ui_layout_vertical_get_element_at (VerticalLayout *vertical, unsigned int pos) {

    return vertical ? (UIElement *) dlist_get_at (vertical->ui_elements, pos) : NULL;

}

// removes an element from the vertical layout group
u8 ui_layout_vertical_remove (VerticalLayout *vertical, UIElement *ui_element) {

    u8 retval = 1;

    if (vertical && ui_element) {
        if (dlist_remove (vertical->ui_elements, ui_element, NULL)) {
            ui_layout_vertical_update (vertical);
            retval = 0;
        }
    }

    return retval;

}

#pragma region scrolling

static void ui_layout_vertical_scroll_up (void *event_data) {

    if (event_data) {
        EventActionData *event_action_data = (EventActionData *) event_data;

        int *amount = (int *) event_action_data->event_data;
        VerticalLayout *vertical = (VerticalLayout *) event_action_data->action_args;

        if (vertical->renderer->window->mouse) {
            // check if the mouse is in the button
            if (mousePos.x >= vertical->transform->rect.x && mousePos.x <= (vertical->transform->rect.x + vertical->transform->rect.w) && 
                mousePos.y >= vertical->transform->rect.y && mousePos.y <= (vertical->transform->rect.y + vertical->transform->rect.h)) {
                // printf ("+%d\n", *amount);

                if (dlist_size (vertical->ui_elements) > 0) {
                    // check if the elements fill all the panel to even allow scrolling
                    int total_elements_height = vertical->curr_element_height * dlist_size (vertical->ui_elements);
                    if (total_elements_height > vertical->transform->rect.h) {
                        // check for the first element position
                        UIElement *first_element = (UIElement *) (dlist_start (vertical->ui_elements)->data);
                        if (first_element->transform->rect.y < 0) {
                            // move all the elements downwards
                            UIElement *ui_element = NULL;
                            UITransform *transform = NULL;
                            for (ListElement *le = dlist_start (vertical->ui_elements); le; le = le->next) {
                                ui_element = (UIElement *) le->data;
                                transform = ui_element->transform;

                                transform->rect.y += (*amount * vertical->scroll_sensitivity);

                                switch (ui_element->type) {
                                    case UI_INPUT:
                                        ui_input_field_placeholder_text_pos_update ((InputField *) ui_element->element);
                                        ui_input_field_text_pos_update ((InputField *) ui_element->element);
                                        break;

                                    case UI_PANEL: 
                                        ui_panel_children_update_pos ((Panel *) ui_element->element); 
                                        break;

                                    case UI_TEXTBOX:
                                        ui_textbox_update_text_pos ((TextBox *) ui_element->element);
                                        break;

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

static void ui_layout_vertical_scroll_down (void *event_data) {

    if (event_data) {
        EventActionData *event_action_data = (EventActionData *) event_data;

        int *amount = (int *) event_action_data->event_data;
        VerticalLayout *vertical = (VerticalLayout *) event_action_data->action_args;

        if (vertical->renderer->window->mouse) {
            // check if the mouse is in the button
            if (mousePos.x >= vertical->transform->rect.x && mousePos.x <= (vertical->transform->rect.x + vertical->transform->rect.w) && 
                mousePos.y >= vertical->transform->rect.y && mousePos.y <= (vertical->transform->rect.y + vertical->transform->rect.h)) {
                // printf ("%d\n", *amount);

                if (dlist_size (vertical->ui_elements) > 0) {
                    // check the pos of the last element
                    UIElement *last_element = (UIElement *) (dlist_end (vertical->ui_elements)->data);
                    int edge = vertical->transform->rect.h - vertical->curr_element_height;
                    if (last_element->transform->rect.y > edge) {
                        // move all the elements upwards
                        UIElement *ui_element = NULL;
                        UITransform *transform = NULL;
                        for (ListElement *le = dlist_start (vertical->ui_elements); le; le = le->next) {
                            ui_element = (UIElement *) le->data;
                            transform = ui_element->transform;

                            transform->rect.y += (*amount * vertical->scroll_sensitivity);

                            switch (ui_element->type) {
                                case UI_INPUT:
                                    ui_input_field_placeholder_text_pos_update ((InputField *) ui_element->element);
                                    ui_input_field_text_pos_update ((InputField *) ui_element->element);
                                    break;

                                case UI_PANEL: 
                                    ui_panel_children_update_pos ((Panel *) ui_element->element); 
                                    break;

                                case UI_TEXTBOX:
                                    ui_textbox_update_text_pos ((TextBox *) ui_element->element);
                                    break;

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