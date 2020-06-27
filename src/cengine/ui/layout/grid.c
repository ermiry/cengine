#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/renderer.h"
#include "cengine/events.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/layout/grid.h"
#include "cengine/ui/layout/align.h"
#include "cengine/ui/image.h"

#include "cengine/ui/textbox.h"
#include "cengine/ui/panel.h"

static void ui_layout_grid_update_all_elements_pos (GridLayout *grid);

static void ui_layout_grid_scroll_up_event (void *event_data);
static void ui_layout_grid_scroll_down_event (void *event_data);

#pragma region grid element

static GridElement *grid_element_new (void) {

    GridElement *grid_element = (GridElement *) malloc (sizeof (GridElement));
    if (grid_element) {
        grid_element->transform = NULL;

        grid_element->x = grid_element->y = 0;

        grid_element->ui_element = NULL;
        grid_element->ui_element_original_width = 0;
        grid_element->ui_element_original_height = 0;
    }

    return grid_element;

}

static void grid_element_delete (void *grid_element_ptr) {

    if (grid_element_ptr) {
        GridElement *grid_element = (GridElement *) grid_element_ptr;

        ui_transform_component_delete (grid_element->transform);

        free (grid_element_ptr);
    } 

}

static void grid_element_delete_full (void *grid_element_ptr) {

    if (grid_element_ptr) {
        GridElement *grid_element = (GridElement *) grid_element_ptr;

        ui_element_destroy (grid_element->ui_element);

        ui_transform_component_delete (grid_element->transform);

        free (grid_element_ptr);
    } 

}

static int grid_element_comparator_by_ui_element (const void *a, const void *b) {

    if (a && b) {
        GridElement *grid_element_a = (GridElement *) a;
        GridElement *grid_element_b = (GridElement *) b;

        if (grid_element_a->ui_element->id < grid_element_b->ui_element->id) return -1;
        else if (grid_element_a->ui_element->id == grid_element_b->ui_element->id) return 0;
        else return 1;
    }

    if (a && !b) return -1;
    else if (!a && b) return 1;
    return 0;

}

static GridElement *grid_element_create (GridLayout *grid, 
    UIElement *ui_element, u32 ui_element_width, u32 ui_element_height) {

    GridElement *grid_element = grid_element_new ();
    if (grid_element) {
        grid_element->transform = ui_transform_component_create (
            0, 0,                                       // pos
            grid->cell_width, grid->cell_height         // size
        );

        grid_element->ui_element = ui_element;
        grid_element->ui_element_original_width = ui_element_width;
        grid_element->ui_element_original_height = ui_element_height;
    }

    return grid_element;

}

// updates the grid element's ui element's position
static void grid_element_update_ui_elements_pos (GridLayout *grid, GridElement *grid_element) {

    if (grid_element) {
        UIElement *ui_element = grid_element->ui_element;

        if (ui_element) {
            // ui_element->transform->rect.x = grid_element->transform->rect.x;
            // ui_element->transform->rect.y = grid_element->transform->rect.y;

            // printf ("BEFORE: ui element: x %d - y %d\n", ui_element->transform->rect.x, ui_element->transform->rect.y);

            UITransform *trans = ui_transform_component_create (
                0, 0, 
                ui_element->transform->rect.w, ui_element->transform->rect.h
            );

            if (trans) {
                ui_transform_component_set_pos (
                    // ((Image *) grid_element->ui_element)->ui_element->transform, 
                    trans,
                    NULL, 
                    &grid_element->transform->rect, 
                    grid->cell_pos, 
                    false
                );

                ui_element->transform->rect.x = trans->rect.x;
                ui_element->transform->rect.y = trans->rect.y;

                ui_transform_component_delete (trans);
            }

            // printf ("AFTER: ui element: x %d - y %d\n", ui_element->transform->rect.x, ui_element->transform->rect.y);
        }
    }

}

#pragma endregion

#pragma region grid

static GridLayout *ui_layout_grid_new (void) {

    GridLayout *grid = (GridLayout *) malloc (sizeof (GridLayout));
    if (grid) {
        memset (grid, 0, sizeof (GridLayout));

        grid->renderer = NULL;
        grid->transform = NULL;
        grid->elements = NULL;

        grid->event_scroll_up = NULL;
        grid->event_scroll_down = NULL;
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

        if (grid->event_scroll_up) {
            cengine_event_unregister (grid->event_scroll_up);
        }

        if (grid->event_scroll_down) {
            cengine_event_unregister (grid->event_scroll_down);
        }

        free (grid);
    }

}

// get the amount of elements that are inside the grid
size_t ui_layout_grid_get_elements_count (GridLayout *grid) {

    return grid ? grid->elements->size : 0;

}

// get the current grid's size, cols and rows
void ui_layout_grid_get_size (GridLayout *grid, u32 *cols, u32 *rows) {

    if (grid) {
        *cols = grid->x_count;
        *rows = grid->y_count;
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

// sets the cell's inner padding percentage
void ui_layout_grid_set_cell_inner_padding (GridLayout *grid, 
    float cell_x_inner_padding_percentage, float cell_y_inner_padding_percentage) {

    if (grid) {
        grid->cell_x_inner_padding_percentage = cell_x_inner_padding_percentage;
        grid->cell_y_inner_padding_percentage = cell_y_inner_padding_percentage;
    }

}

// sets the ui position to be used for grid element's, by the default it is UI_POS_MIDDLE_CENTER
void ui_layout_grid_set_cell_position (GridLayout *grid, UIPosition pos) {

    if (grid) grid->cell_pos = pos;

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
        grid->event_scroll_up = cengine_event_register (CENGINE_EVENT_SCROLL_UP, ui_layout_grid_scroll_up_event, grid);
        grid->event_scroll_down = cengine_event_register (CENGINE_EVENT_SCROLL_DOWN, ui_layout_grid_scroll_down_event, grid);

        grid->scroll_sensitivity = GRID_LAYOUT_DEFAULT_SCROLL;
        grid->scrolling = enable;
    }

}

// sets how fast the elements move when scrolling, the default is a value of 5
void ui_layout_grid_set_scroll_sensitivity (GridLayout *grid, u32 sensitivity) {

    if (grid) grid->scroll_sensitivity = sensitivity;

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
        UITransform *transform = grid_element->transform;
        // UITransform *transform = grid_element->ui_element->transform;

        if (transform) {
            switch (grid->align_x) {
                case ALIGN_KEEP_SIZE: {
                    // transform->rect.x = grid->transform->rect.x + (x_size * grid->x_count);
                    // transform->rect.x = 0 + (x_size * grid->x_count);
                    transform->rect.x = 0 + (x_size * x_count);
                } break;

                case ALIGN_PADDING_ALL: {
                    // transform->rect.x = grid->transform->rect.x + (x_size * grid->x_count) + (x_padding * (grid->x_count + 1));
                    // transform->rect.x = 0 + (x_size * grid->x_count) + (x_padding * (grid->x_count + 1));
                    transform->rect.x = 0 + (x_size * x_count) + (x_padding * (x_count + 1));
                } break;

                case ALIGN_PADDING_BETWEEN: {
                    // transform->rect.x = grid->transform->rect.x + (x_size * grid->x_count) + (x_padding * grid->x_count);
                    // transform->rect.x = 0 + (x_size * grid->x_count) + (x_padding * grid->x_count);
                    transform->rect.x = 0 + (x_size * x_count) + (x_padding * x_count);
                } break;

                default: break;
            }

            switch (grid->align_y) {
                case ALIGN_KEEP_SIZE: {
                    // transform->rect.y = grid->transform->rect.y + (y_size * grid->y_count);
                    // transform->rect.y = 0 + (y_size * grid->y_count);
                    transform->rect.y = 0 + (y_size * y_count);
                } break;

                case ALIGN_PADDING_ALL: {
                    // transform->rect.y = grid->transform->rect.y + (y_size * grid->y_count) + (y_padding * (grid->y_count + 1));
                    // transform->rect.y = 0 + (y_size * grid->y_count) + (y_padding * (grid->y_count + 1));
                    transform->rect.y = 0 + (y_size * y_count) + (y_padding * (y_count + 1));
                } break;

                case ALIGN_PADDING_BETWEEN: {
                    // transform->rect.y = grid->transform->rect.y + (y_size * grid->y_count) + (y_padding * grid->y_count);
                    // transform->rect.y = 0 + (y_size * grid->y_count) + (y_padding * grid->y_count);
                    transform->rect.y = 0 + (y_size * y_count) + (y_padding * y_count);
                } break;

                default: break;
            }
        }
    }

}

// updates all the grid elements positions
static void ui_layout_grid_update_all_elements_pos (GridLayout *grid) {

    if (grid) {
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

        GridElement *grid_element = NULL;
        for (ListElement *le = dlist_start (grid->elements); le; le = le->next) {
            grid_element = (GridElement *) le->data;

            // printf ("x_count: %d, y_count: %d\n", x_count, y_count);
            // printf ("x_size: %d, y_size: %d\n", x_size, y_size);
            // printf ("x_padding: %d, y_padding: %d\n", x_padding, y_padding);
            
            ui_layout_grid_set_element_pos (grid, 
                grid_element,
                x_size, y_size,
                x_count, y_count,
                x_padding, y_padding);

            // updates the grid element's ui element's position
            grid_element_update_ui_elements_pos (grid, grid_element);

            // printf ("x %d - y %d\n", grid_element->ui_element->transform->rect.x, grid_element->ui_element->transform->rect.y);
            // printf ("x_count %d < %d\n", x_count, grid->cols -1);

            if (x_count < (grid->cols - 1)) x_count += 1;
            else {
                x_count = 0;
                y_count += 1;
            }
        }
    }

}

static void ui_layout_grid_update_image_size (GridLayout *grid, GridElement *element) {

    if (grid && element) {
        float x_inner_padding = grid->cell_width * grid->cell_x_inner_padding_percentage;
        float y_inner_padding = grid->cell_height * grid->cell_y_inner_padding_percentage;

        u32 max_width = grid->cell_width - x_inner_padding;      // Max width for the image
        u32 max_height = grid->cell_height - y_inner_padding;    // Max height for the image
        float ratio = 0;                            // Used for aspect ratio
        u32 width = element->ui_element_original_width;        // Current image width
        u32 height = element->ui_element_original_height;      // Current image height

        u32 new_width = element->ui_element_original_width;
        u32 new_height = element->ui_element_original_height;

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

static void ui_layout_grid_update_element_size (GridLayout *grid, GridElement *grid_element) {

    if (grid && grid_element) {
        switch (grid_element->ui_element->type) {
            case UI_IMAGE: 
                ui_layout_grid_update_image_size (grid, grid_element); 
                break;

            default: break;
        }
    }

}

static void ui_layout_grid_update_all_elements_size (GridLayout *grid) {

    if (grid) {
        // GridElement *grid_element = NULL;
        for (ListElement *le = dlist_start (grid->elements); le; le = le->next) {
            ui_layout_grid_update_element_size (grid, (GridElement *) le->data);
        }
    }

}

// creates a new grid layout
// x and y for position, w and h for dimensions
GridLayout *ui_layout_grid_create (i32 x, i32 y, u32 w, u32 h, Renderer *renderer) {

    GridLayout *grid = ui_layout_grid_new ();

    if (grid) {
        grid->renderer = renderer;

        grid->transform = ui_transform_component_create (x, y, w, h);

        grid->elements = dlist_init (grid_element_delete, grid_element_comparator_by_ui_element);

        grid->cell_pos = UI_POS_MIDDLE_CENTER;
    }

    return grid;

}

// 14/05/2020 -- 13:44- working for images in cimage
// updates the grid with a new size (cols and rows)
// returns 0 on success update, 1 on failure
int ui_layout_grid_update_size (GridLayout *grid, u32 cols, u32 cell_width, u32 cell_height) {

    int retval = 1;

    if (grid) {
        if (grid->elements) {
            // printf ("\nBEFORE: cols %d - rows %d\n", grid->cols, grid->rows);
            // printf ("\nBEFORE: cell_width %d - cell_height %d\n", grid->cell_width, grid->cell_height);

            grid->cols = cols;
            // grid->rows = rows;       // we dont care about rows

            // printf ("\n\ncols %d - rows %d\n\n", grid->cols, grid->rows);

            grid->n_elements_x_row = cols;
            // grid->n_elements_x_col = rows;

            ui_layout_grid_set_cell_size (grid, cell_width, cell_height);
            // printf ("\ncell_width %d - cell_height %d\n", grid->cell_width, grid->cell_height);

            GridElement *grid_element = NULL;
            for (ListElement *le = dlist_start (grid->elements); le; le = le->next) {
                grid_element = (GridElement *) le->data;

                grid_element->transform->rect.w = cell_width;
                grid_element->transform->rect.h = cell_height;
            }

            ui_layout_grid_update_all_elements_size (grid);

            ui_layout_grid_update_all_elements_pos (grid);
        }
    }

    return retval;

}

// adds a new element to the grid in the specified pos (0 indexed)
// if its greater than the current size, it will be added at the end
// returns 0 on success, 1 if failed to add
u8 ui_layout_grid_add_element_at_pos (GridLayout *grid, UIElement *ui_element, u32 pos) {

    u8 retval = 1;

    if (grid && ui_element) {
        // GridElement *grid_element = grid_element_new (ui_element, 
        //     ui_element->transform->rect.w, ui_element->transform->rect.h);
        GridElement *grid_element = grid_element_create (grid,
            ui_element, ui_element->transform->rect.w, ui_element->transform->rect.h);

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

                // update element size if necesary
                ui_layout_grid_update_element_size (grid, grid_element);

                // updated in ui_layout_grid_update_all_elements_pos () called before this
                // updates the grid element's ui element's position
                // grid_element_update_ui_elements_pos (grid_element);

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
        // GridElement *grid_element = grid_element_new (ui_element, 
        //     ui_element->transform->rect.w, ui_element->transform->rect.h);
        GridElement *grid_element = grid_element_create (grid,
            ui_element, ui_element->transform->rect.w, ui_element->transform->rect.h);

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

                // printf ("x_size: %d, y_size: %d\n", x_size, y_size);
                // printf ("x_padding: %d, y_padding: %d\n", x_padding, y_padding);

                ui_layout_grid_set_element_pos (grid, 
                    grid_element,
                    x_size, y_size,
                    grid->x_count, grid->y_count,
                    x_padding, y_padding);

                // printf ("grid element: x %d - y %d\n", grid_element->transform->rect.x, grid_element->transform->rect.y);

                if (grid->x_count < (grid->cols - 1)) grid->x_count += 1;
                else {
                    grid->x_count = 0;
                    grid->y_count += 1;
                }

                // update element size if necesary
                ui_layout_grid_update_element_size (grid, grid_element);

                // updates the grid element's ui element's position
                grid_element_update_ui_elements_pos (grid, grid_element);

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
u8 ui_layout_grid_remove_element (GridLayout *grid, UIElement *ui_element) {

    u8 retval = 1;

    if (grid && ui_element) {
        GridElement *grid_element = (GridElement *) dlist_remove (grid->elements, ui_element, NULL);
        if (grid_element) {
            if (grid->x_count == 0) {
                grid->x_count = (grid->cols - 1);
                grid->y_count -= 1;
            }

            else {
                grid->x_count -= 1;
            }

            ui_layout_grid_update_all_elements_pos (grid);

            grid_element_delete (grid_element);

            retval = 0;
        }
    }

    return retval;

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

#pragma endregion

#pragma region scrolling

static void ui_layout_grid_scroll_up_actual (GridLayout *grid, int amount) {

    if (grid) {
        // printf ("up!\n");
        // printf ("+%d\n", amount);

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

                        grid_element->ui_element->transform->rect.y += (amount * grid->scroll_sensitivity);

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

                            default: break;
                        }
                    }
                }
            }
        }
    }

}

static void ui_layout_grid_scroll_down_actual (GridLayout *grid, int amount) {

    if (grid) {
        // printf ("down!\n");
        // printf ("+%d\n", amount);

        if (dlist_size (grid->elements) > 0) {
            // check the pos of the last element
            GridElement *last_element = (GridElement *) (dlist_end (grid->elements)->data);
            u32 edge = grid->transform->rect.h - grid->cell_height;
            if (last_element->ui_element->transform->rect.y > edge) {
                UIElement *ui_element = NULL;
                GridElement *grid_element = NULL;
                for (ListElement *le = dlist_start (grid->elements); le; le = le->next) {
                    grid_element = (GridElement *) le->data;

                    grid_element->ui_element->transform->rect.y += (amount * grid->scroll_sensitivity);

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

                        default: break;
                    }
                }
            }
        }
    }

}

// triggered on mouse scroll up
static void ui_layout_grid_scroll_up_event (void *event_data) {

    if (event_data) {
        EventActionData *event_action_data = (EventActionData *) event_data;

        int *amount = (int *) event_action_data->event_data;
        GridLayout *grid = (GridLayout *) event_action_data->action_args;

        if (grid->renderer->window->mouse) {
            // check if the mouse is hovering the panel
            if (mousePos.x >= grid->transform->rect.x && mousePos.x <= (grid->transform->rect.x + grid->transform->rect.w) && 
                mousePos.y >= grid->transform->rect.y && mousePos.y <= (grid->transform->rect.y + grid->transform->rect.h)) {
                ui_layout_grid_scroll_up_actual (grid, *amount);
            }
        }
    }

}

// triggered on mouse scroll down
static void ui_layout_grid_scroll_down_event (void *event_data) {

    if (event_data) {
        EventActionData *event_action_data = (EventActionData *) event_data;

        int *amount = (int *) event_action_data->event_data;
        GridLayout *grid = (GridLayout *) event_action_data->action_args;

        if (grid->renderer->window->mouse) {
            // check if the mouse is in the button
            if (mousePos.x >= grid->transform->rect.x && mousePos.x <= (grid->transform->rect.x + grid->transform->rect.w) && 
                mousePos.y >= grid->transform->rect.y && mousePos.y <= (grid->transform->rect.y + grid->transform->rect.h)) {
                ui_layout_grid_scroll_down_actual (grid, *amount);
            }
        }
    }

}

// public method to be used by user,s like to register a key to scroll up the grid
void ui_layout_grid_scroll_up (GridLayout *grid, int amount) {

    if (grid) {
        ui_layout_grid_scroll_up_actual (grid, amount);
    }

}

// public method to be used by user,s like to register a key to scroll down the grid
void ui_layout_grid_scroll_down (GridLayout *grid, int amount) {

    if (grid) {
        ui_layout_grid_scroll_down_actual (grid, amount);
    }

}

#pragma endregion