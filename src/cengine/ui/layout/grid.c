#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/renderer.h"
#include "cengine/events.h"

#include "cengine/ui/components/transform.h"
#include "cengine/ui/layout/grid.h"
#include "cengine/ui/layout/align.h"
#include "cengine/ui/image.h"

// FIXME:!!!
#include "cengine/ui/textbox.h"
#include "cengine/ui/panel.h"

static void ui_layout_grid_scroll_up (void *event_data);
static void ui_layout_grid_scroll_down (void *event_data);

static GridElement *grid_element_new (UIElement *ui_element, u32 width, u32 height) {

    GridElement *element = (GridElement *) malloc (sizeof (GridElement));
    if (element) {
        element->ui_element = ui_element;
        element->original_width = width;
        element->original_height = height;
    }

    return element;

}

static void grid_element_delete (void *grid_element_ptr) {

    if (grid_element_ptr) free (grid_element_ptr);

}

static void grid_element_delete_full (void *grid_element_ptr) {

    if (grid_element_ptr) {
        ui_element_destroy (((GridElement *) grid_element_ptr)->ui_element);
        free (grid_element_ptr);
    } 

}

static GridLayout *ui_layout_grid_new (void) {

    GridLayout *grid = (GridLayout *) malloc (sizeof (GridLayout));
    if (grid) {
        memset (grid, 0, sizeof (GridLayout));

        grid->transform = NULL;
        grid->elements = NULL;
    }

    return grid;

}

static void ui_layout_grid_reset_values (GridLayout *grid) {

    if (grid) {
        grid->x_count = 0;
        grid->y_count = 0;

        grid->x_offset = 0;
        grid->y_offset = 0;
    }

}

void ui_layout_grid_delete (void *grid_ptr) {

    if (grid_ptr) {
        GridLayout *grid = (GridLayout *) grid_ptr;

        ui_transform_component_delete (grid->transform);

        dlist_delete (grid->elements);

        free (grid);
    }

}

// sets the preferred number of columns and rows available for the grid
void ui_layout_grid_set_grid (GridLayout *grid, u32 cols, u32 rows) {

    if (grid) {
        grid->cols = cols;
        grid->rows = rows;

        ui_layout_grid_reset_values (grid);
    }

}

// sets the number of elements x row
void ui_layout_grid_set_elements_x_row (GridLayout *grid, u32 n_elements_x_row) {

    if (grid) grid->n_elements_x_row = n_elements_x_row;

}

// sets the number of elements x col
void ui_layout_grid_set_elements_x_col (GridLayout *grid, u32 n_elements_x_col) {

    if (grid) grid->n_elements_x_col = n_elements_x_col;

}

// sets the preferred size of the cells, the layout must be of the correct size
void ui_layout_grid_set_cell_size (GridLayout *grid, u32 width, u32 height) {

    if (grid) {
        grid->cell_width = width;
        grid->cell_height = height;
        // printf ("cell size: %d x %d\n", grid->cell_width, grid->cell_height);
    }

}

// sets the cell padding - distance between cells (elements)
void ui_layout_grid_set_cell_padding (GridLayout *grid, u32 cell_padding_x, u32 cell_padding_y) {

    if (grid) {
        grid->cell_padding_x = cell_padding_x;
        grid->cell_padding_y = cell_padding_y;
    }

}

// sets the alignment to use in the x axis
void ui_layout_grid_set_x_axis_alignment (GridLayout *grid, AxisAlignment align) {

    if (grid) grid->align_x = align;

}

// sets the alignment to use in the y axis
void ui_layout_grid_set_y_axis_alignment (GridLayout *grid, AxisAlignment align) {

    if (grid) grid->align_y = align;

}

// enables / disbale scrolling in the grid layout
void ui_layout_grid_toggle_scrolling (GridLayout *grid, bool enable) {

    if (grid) {
        // register this grid layout to listen for the scroll event
        cengine_event_register (CENGINE_EVENT_SCROLL_UP, ui_layout_grid_scroll_up, grid);
        cengine_event_register (CENGINE_EVENT_SCROLL_DOWN, ui_layout_grid_scroll_down, grid);

        grid->scroll_sensitivity = GRID_LAYOUT_DEFAULT_SCROLL;
        grid->scrolling = enable;
    }

}

// sets how fast the elements move when scrolling, the default is a value of 5
void ui_layout_grid_set_scroll_sensitivity (GridLayout *grid, u32 sensitivity) {

    if (grid) grid->scroll_sensitivity = sensitivity;

}

// FIXME: 22:35 -- 05/02/2020 - current method was designed to work for cimage images!
static void ui_layout_grid_update_element_size (GridLayout *grid, GridElement *element) {

    if (grid && element) {
        u32 max_width = grid->cell_width - grid->cell_padding_x;      // Max width for the image
        u32 max_height = grid->cell_height - grid->cell_padding_y;    // Max height for the image
        float ratio = 0;                            // Used for aspect ratio
        u32 width = element->original_width;        // Current image width
        u32 height = element->original_height;      // Current image height

        u32 new_width = element->original_width;
        u32 new_height = element->original_height;

        // Check if the current width is larger than the max
        if (width > max_width) {
            ratio = (float) max_width / width;      // get ratio for scaling image
            new_width = max_width;                  // Set new width
            new_height *= ratio;                    // Scale height based on ratio
            height = height * ratio;                // Reset height to match scaled image
            width = width * ratio;                  // Reset width to match scaled image
        }

        // Check if current height is larger than max
        if (height > max_height) {
            ratio = (float) max_height / height;    // get ratio for scaling image
            new_height = max_height;                // Set new height
            new_width *= ratio;                     // Scale width based on ratio
            width = width * ratio;                  // Reset width to match scaled image
            height = height * ratio;                // Reset height to match scaled image
        }

        // printf ("new: %d x %d\n", element->trans->rect.w, element->trans->rect.h);
        element->ui_element->transform->rect.w = new_width;
        element->ui_element->transform->rect.h = new_height;
    }

}

// FIXME: 06/02/2020 -- do we still need this?
// FIXME: we need to update the panel size
// updates the grid with a new size
// returns 0 on success update, 1 on failure
int ui_layout_grid_update_dimensions (GridLayout *grid, u32 cols, u32 rows) {

    int retval = 1;

    // if (grid) {
    //     if (grid->elements) {
    //         if (cols > 0 && rows > 0) {
    //             grid->cols = cols;
    //             grid->rows = rows;
    //             // grid->curr_n_ui_elements = 0;
    //             grid->next_x = 0;
    //             grid->next_y = 0;
    //             // grid->max_n_ui_elements = grid->cols * grid->rows;

    //             // grid->cell_width = (cols * grid->cell_width) / grid->cols;
    //             // grid->cell_height = (cols * grid->cell_height) / grid->rows;

    //             grid->cell_width = grid->transform->rect.w / cols;
    //             grid->cell_height = grid->transform->rect.h / rows;
    //             // printf ("cell height: %d / %d\n", grid->transform->rect.h, rows);
    //             // printf ("cell size: %d x %d\n", grid->cell_width, grid->cell_height);
    //             grid->cell_padding_x = grid->cell_width * 0.1;
    //             grid->cell_padding_y = grid->cell_height * 0.1;

    //             GridElement *grid_element = NULL;
    //             for (ListElement *le = dlist_start (grid->elements); le; le = le->next) {
    //                 grid_element = (GridElement *) le->data;

    //                 grid_element->x = grid->next_x;
    //                 grid_element->y = grid->next_y;

    //                 ui_layout_grid_update_element_size (grid, grid_element);
    //                 ui_layout_grid_update_element_pos (grid, grid_element->ui_element->transform);

    //                 // grid->curr_n_ui_elements += 1;

    //                 if (grid->next_x < (grid->cols - 1)) grid->next_x += 1;
    //                 else {
    //                     grid->next_x = 0;
    //                     grid->next_y += 1;
    //                 } 
    //             }

    //             retval = 0;
    //         }
    //     }
    // }

    return retval;

}

// creates a new grid layout
// x and y for position, w and h for dimensions
GridLayout *ui_layout_grid_create (i32 x, i32 y, u32 w, u32 h, Renderer *renderer) {

    GridLayout *grid = ui_layout_grid_new ();

    if (grid) {
        grid->renderer = renderer;

        grid->transform = ui_transform_component_create (x, y, w, h);

        grid->elements = dlist_init (grid_element_delete, NULL);
    }

    return grid;

}

static void ui_layout_grid_get_layout_values (GridLayout *grid,
    u32 *x_size, u32 *y_size,
    u32 *x_padding, u32 *y_padding) {

    switch (grid->align_x) {
        case ALIGN_KEEP_SIZE: {
            // elements must be set to be the size of grid / n elements
            *x_size = grid->cell_width;

            // padding must be 0
            *x_padding = 0;
            *y_padding = 0;
        } break;

        // element size * element count must be smaller than grid x size
        case ALIGN_PADDING_ALL: {
            *x_size = grid->cell_width;

            u32 temp = grid->transform->rect.w - (*x_size * grid->n_elements_x_row);
            *x_padding = temp / (grid->n_elements_x_row + 1);
            *y_padding = 0;
        } break;

        // element size * element count must be smaller than grid x size
        case ALIGN_PADDING_BETWEEN: {
            *x_size = grid->cell_width;

            u32 temp = grid->transform->rect.w - (*x_size * grid->n_elements_x_row);
            *x_padding = temp / (grid->n_elements_x_row - 1);
            *y_padding = 0;
        } break;

        default: break;
    }

    switch (grid->align_y) {
        case ALIGN_KEEP_SIZE: {
            *y_size = grid->cell_height;
        } break;

        // TODO: option to set custom padding values
        case ALIGN_PADDING_ALL: {
            *y_size = grid->cell_height;

            u32 temp = grid->transform->rect.h - (*y_size * grid->n_elements_x_col);
            *y_padding = temp / (grid->n_elements_x_row + 1);
        } break;

        // TODO: option to set custom padding values
        case ALIGN_PADDING_BETWEEN: {
            *y_size = grid->cell_height;

            u32 temp = grid->transform->rect.h - (*y_size * grid->n_elements_x_col);
            *y_padding = temp / (grid->n_elements_x_row - 1);
        } break;

        default: break;
    }

}

static void ui_layout_grid_set_element_pos (GridLayout *grid,
    GridElement *grid_element,
    u32 x_size, u32 y_size,
    u32 x_count, u32 y_count,
    u32 x_padding, u32 y_padding) {

    if (grid && grid_element) {
        UITransform *transform = grid_element->ui_element->transform;

        switch (grid->align_x) {
            case ALIGN_KEEP_SIZE: {
                // transform->rect.x = grid->transform->rect.x + (x_size * grid->x_count);
                transform->rect.x = 0 + (x_size * grid->x_count);
            } break;

            case ALIGN_PADDING_ALL: {
                // transform->rect.x = grid->transform->rect.x + (x_size * grid->x_count) + (x_padding * (grid->x_count + 1));
                transform->rect.x = 0 + (x_size * grid->x_count) + (x_padding * (grid->x_count + 1));
            } break;

            case ALIGN_PADDING_BETWEEN: {
                // transform->rect.x = grid->transform->rect.x + (x_size * grid->x_count) + (x_padding * grid->x_count);
                transform->rect.x = 0 + (x_size * grid->x_count) + (x_padding * grid->x_count);
            } break;

            default: break;
        }

        switch (grid->align_y) {
            case ALIGN_KEEP_SIZE: {
                // transform->rect.y = grid->transform->rect.y + (y_size * grid->y_count);
                transform->rect.y = 0 + (y_size * grid->y_count);
            } break;

            case ALIGN_PADDING_ALL: {
                // transform->rect.y = grid->transform->rect.y + (y_size * grid->y_count) + (y_padding * (grid->y_count + 1));
                transform->rect.y = 0 + (y_size * grid->y_count) + (y_padding * (grid->y_count + 1));
            } break;

            case ALIGN_PADDING_BETWEEN: {
                // transform->rect.y = grid->transform->rect.y + (y_size * grid->y_count) + (y_padding * grid->y_count);
                transform->rect.y = 0 + (y_size * grid->y_count) + (y_padding * grid->y_count);
            } break;

            default: break;
        }
    }

}

// updates all the grid elements positions
static void ui_layout_grid_update_all_elements_pos (GridLayout *grid) {

    if (grid) {
        u32 n_elements_x_row = grid->n_elements_x_row;
        u32 n_elements_x_col = grid->n_elements_x_col;
        
        // cell size
        u32 x_size = 0;
        u32 y_size = 0;

        u32 x_padding = 0;
        u32 y_padding = 0;

        ui_layout_grid_get_layout_values (grid, 
            &x_size, &y_size, 
            &x_padding, &y_padding);

        u32 x_count = 0;
        u32 y_count = 0;

        // u32 x_offset = 0;
        // u32 y_offset = 0;

        GridElement *grid_element = NULL;
        UITransform *transform = NULL;
        for (ListElement *le = dlist_start (grid->elements); le; le = le->next) {
            grid_element = (GridElement *) le->data;
            transform = grid_element->ui_element->transform;

            if (x_count >= n_elements_x_row) {
                x_count = 0;
                y_count += 1;
            }
            
            ui_layout_grid_set_element_pos (grid,
                grid_element,
                x_size, y_size,
                x_count, y_count,
                x_padding, y_padding);

            x_count += 1;
        }
    }

}

// adds a new element to the grid in the specified pos (0 indexed)
// if its greater than the current size, it will be added at the end
// returns 0 on success, 1 if failed to add
u8 ui_layout_grid_add_element_at_pos (GridLayout *grid, UIElement *ui_element, u32 pos) {

    u8 retval = 1;

    if (grid && ui_element) {
        GridElement *grid_element = grid_element_new (ui_element, 
            ui_element->transform->rect.w, ui_element->transform->rect.h);

        if (grid_element) {
            grid_element->x = grid->x_count;
            grid_element->y = grid->y_count;

            if (!dlist_insert_at (grid->elements, grid_element, pos)) {
                ui_layout_grid_update_all_elements_pos (grid);

                if (grid->x_count < (grid->cols - 1)) grid->x_count += 1;
                else {
                    grid->x_count = 0;
                    grid->y_count += 1;
                } 

                retval = 0;
            }

            else grid_element_delete (grid_element);    // failed to insert
        }
    }

    return retval;

}

// adds a new element at the end of the grid
// this method avoids updating all the other elements positions as well
// returns 0 on success, 1 on error
u8 ui_layout_grid_add_element_at_end (GridLayout *grid, UIElement *ui_element) {

    u8 retval = 1;

    if (grid && ui_element) {
        // add element in next available idx -> at the end for now
        GridElement *grid_element = grid_element_new (ui_element, 
            ui_element->transform->rect.w, ui_element->transform->rect.h);

        if (grid_element) {
            grid_element->x = grid->x_count;
            grid_element->y = grid->y_count;

            if (!dlist_insert_after (grid->elements, dlist_end (grid->elements), grid_element)) {
                if (grid->x_count >= grid->n_elements_x_row) {
                    grid->x_count = 0;
                    grid->y_count += 1;
                }

                u32 x_size = 0;
                u32 y_size = 0;

                u32 x_padding = 0;
                u32 y_padding = 0;

                ui_layout_grid_get_layout_values (grid,
                    &x_size, &y_size,
                    &x_padding, & y_padding);

                ui_layout_grid_set_element_pos (grid, 
                    grid_element,
                    x_size, y_size,
                    grid->x_count, grid->y_count,
                    x_padding, y_padding);

                grid->x_count += 1;

                retval = 0;
            }
        }
    }

    return retval;

}

// returns the ui element that is at the required position in the layout
UIElement *ui_layout_grid_get_element_at (GridLayout *grid, unsigned int pos) {

    if (grid) {
        void *found = dlist_get_at (grid->elements, pos);
        return found ? ((GridElement *) found)->ui_element : NULL;
    }

    return NULL;

}

// removes an element from the grid
void ui_layout_grid_remove_element (GridLayout *grid, UIElement *ui_element) {

    if (grid) {
        // FIXME:
    }

}

// removes all ui elements from the grid layout without destroying them
void ui_layout_grid_remove_ui_elements (GridLayout *grid) {

    if (grid) {
        dlist_set_destroy (grid->elements, grid_element_delete);
        dlist_delete (grid->elements);

        grid->elements = dlist_init (grid_element_delete, NULL);
        ui_layout_grid_reset_values (grid);
    }

}

// destroys the ui elements inside the grid
void ui_layout_grid_destroy_ui_elements (GridLayout *grid) {

    if (grid) {
        dlist_set_destroy (grid->elements, grid_element_delete_full);
        dlist_delete (grid->elements);

        grid->elements = dlist_init (grid_element_delete, NULL);
        ui_layout_grid_reset_values (grid);
    }

}

#pragma region scrolling

static void ui_layout_grid_scroll_up (void *event_data) {

    if (event_data) {
        EventActionData *event_action_data = (EventActionData *) event_data;

        int *amount = (int *) event_action_data->event_data;
        GridLayout *grid = (GridLayout *) event_action_data->action_args;

        if (grid->renderer->window->mouse) {
            // check if the mouse is in the button
            if (mousePos.x >= grid->transform->rect.x && mousePos.x <= (grid->transform->rect.x + grid->transform->rect.w) && 
                mousePos.y >= grid->transform->rect.y && mousePos.y <= (grid->transform->rect.y + grid->transform->rect.h)) {
                // printf ("+%d\n", *amount);

                if (dlist_size (grid->elements) > 0) {
                    // check if the elements fill all the panel to even allow scrolling
                    u32 total_elements_height = grid->cell_height * dlist_size (grid->elements);
                    if (total_elements_height > grid->transform->rect.h) {
                        // check for the first element position
                        GridElement *first_element = (GridElement *) (dlist_start (grid->elements)->data);
                        if (first_element->ui_element->transform->rect.y < 0) {
                            UIElement *ui_element = NULL;
                            GridElement *grid_element = NULL;
                            for (ListElement *le = dlist_start (grid->elements); le; le = le->next) {
                                grid_element = (GridElement *) le->data;

                                grid_element->ui_element->transform->rect.y += (*amount * grid->scroll_sensitivity);

                                ui_element = grid_element->ui_element;
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
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

static void ui_layout_grid_scroll_down (void *event_data) {

    if (event_data) {
        EventActionData *event_action_data = (EventActionData *) event_data;

        int *amount = (int *) event_action_data->event_data;
        GridLayout *grid = (GridLayout *) event_action_data->action_args;

        if (grid->renderer->window->mouse) {
            // check if the mouse is in the button
            if (mousePos.x >= grid->transform->rect.x && mousePos.x <= (grid->transform->rect.x + grid->transform->rect.w) && 
                mousePos.y >= grid->transform->rect.y && mousePos.y <= (grid->transform->rect.y + grid->transform->rect.h)) {
                // printf ("+%d\n", *amount);

                if (dlist_size (grid->elements) > 0) {
                    // check the pos of the last element
                    GridElement *last_element = (GridElement *) (dlist_end (grid->elements)->data);
                    u32 edge = grid->transform->rect.h - grid->cell_height;
                    if (last_element->ui_element->transform->rect.y > edge) {
                        UIElement *ui_element = NULL;
                        GridElement *grid_element = NULL;
                        for (ListElement *le = dlist_start (grid->elements); le; le = le->next) {
                            grid_element = (GridElement *) le->data;

                            grid_element->ui_element->transform->rect.y += (*amount * grid->scroll_sensitivity);

                            ui_element = grid_element->ui_element;
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
                            }
                        }
                    }
                }
            }
        }
    }

}

#pragma endregion