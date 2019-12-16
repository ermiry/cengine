#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/renderer.h"

#include "cengine/ui/components/transform.h"
#include "cengine/ui/layout/grid.h"
#include "cengine/ui/image.h"

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

static void ui_layout_grid_reset_pos_values (GridLayout *grid) {

    if (grid) {
        grid->curr_n_ui_elements = 0;
        grid->next_x = 0;
        grid->next_y = 0;
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

// sets the number of columns and rows available for the grid
void ui_layout_grid_set_grid (GridLayout *grid, u32 cols, u32 rows) {

    if (grid) {
        grid->cols = cols;
        grid->rows = rows;

        grid->elements = dlist_init (grid_element_delete, NULL);

        grid->max_n_ui_elements = grid->cols * grid->rows;
        grid->curr_n_ui_elements = 0;
        grid->next_x = grid->next_y = 0;

        grid->cell_width = grid->transform->rect.w / grid->cols;
        grid->cell_height = grid->transform->rect.h / grid->rows;
        // printf ("cell size: %d x %d\n", grid->cell_width, grid->cell_height);
        grid->cell_padding_x = grid->cell_width * 0.1;
        grid->cell_padding_y = grid->cell_height * 0.1;
    }

}

// TODO: update layout
// sets the preferred size of the cells, the layout must be of the correct size
void ui_layout_grid_set_cell_size (GridLayout *grid, u32 width, u32 height) {

    if (grid) {
        grid->cell_width = width;
        grid->cell_height = height;
        // printf ("cell size: %d x %d\n", grid->cell_width, grid->cell_height);

        grid->cell_padding_x = grid->cell_width * 0.1;
        grid->cell_padding_y = grid->cell_height * 0.1;
    }

}

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

static void ui_layout_grid_update_element_pos (GridLayout *grid, UITransform *ui_element_trans) {

    if (grid && ui_element_trans) {
        UIRect cell = { .x = (grid->transform->rect.x + (grid->cell_width * grid->next_x)), 
            .y = (grid->transform->rect.y + (grid->cell_height * grid->next_y)),
            .w = grid->cell_width, .h = grid->cell_height };
        // printf ("x: %d - y: %d - w: %d - h: %d\n", cell.x, cell.y, cell.w, cell.h);
        ui_transform_component_set_pos (ui_element_trans, NULL, &cell, UI_POS_MIDDLE_CENTER, false);
    }

}

// FIXME: we need to update the panel size
// updates the grid with a new size
// returns 0 on success update, 1 on failure
int ui_layout_grid_update_dimensions (GridLayout *grid, u32 cols, u32 rows) {

    int retval = 1;

    if (grid) {
        if (grid->elements) {
            if (cols > 0 && rows > 0) {
                grid->cols = cols;
                grid->rows = rows;
                grid->curr_n_ui_elements = 0;
                grid->next_x = 0;
                grid->next_y = 0;
                grid->max_n_ui_elements = grid->cols * grid->rows;

                // grid->cell_width = (cols * grid->cell_width) / grid->cols;
                // grid->cell_height = (cols * grid->cell_height) / grid->rows;

                grid->cell_width = grid->transform->rect.w / cols;
                grid->cell_height = grid->transform->rect.h / rows;
                // printf ("cell height: %d / %d\n", grid->transform->rect.h, rows);
                // printf ("cell size: %d x %d\n", grid->cell_width, grid->cell_height);
                grid->cell_padding_x = grid->cell_width * 0.1;
                grid->cell_padding_y = grid->cell_height * 0.1;

                GridElement *grid_element = NULL;
                for (ListElement *le = dlist_start (grid->elements); le; le = le->next) {
                    grid_element = (GridElement *) le->data;

                    grid_element->x = grid->next_x;
                    grid_element->y = grid->next_y;

                    ui_layout_grid_update_element_size (grid, grid_element);
                    ui_layout_grid_update_element_pos (grid, grid_element->ui_element->transform);

                    grid->curr_n_ui_elements += 1;

                    if (grid->next_x < (grid->cols - 1)) grid->next_x += 1;
                    else {
                        grid->next_x = 0;
                        grid->next_y += 1;
                    } 
                }

                retval = 0;
            }
        }
    }

    return retval;

}

// creates a new grid layout
// x and y for position, w and h for dimensions
GridLayout *ui_layout_grid_create (i32 x, i32 y, u32 w, u32 h) {

    GridLayout *grid = ui_layout_grid_new ();

    grid->transform = ui_transform_component_create (x, y, w, h);

    return grid;

}

// adds a new element to the grid
// returns 0 on success, 1 if failed to add
u8 ui_layout_grid_add_element (GridLayout *grid, UIElement *ui_element) {

    u8 retval = 1;

    if (grid && ui_element) {
        if (grid->elements && (grid->curr_n_ui_elements < grid->max_n_ui_elements)) {
            // add element in next available idx -> at the end for now
            GridElement *grid_element = grid_element_new (ui_element, 
                ui_element->transform->rect.w, ui_element->transform->rect.h);
            grid_element->x = grid->next_x;
            grid_element->y = grid->next_y;
            dlist_insert_after (grid->elements, dlist_end (grid->elements), grid_element);

            ui_layout_grid_update_element_size (grid, grid_element);
            ui_layout_grid_update_element_pos (grid, ui_element->transform);

            grid->curr_n_ui_elements += 1;

            if (grid->next_x < (grid->cols - 1)) grid->next_x += 1;
            else {
                grid->next_x = 0;
                grid->next_y += 1;
            } 

            retval = 0;
        }
    }

    return retval;

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
        ui_layout_grid_reset_pos_values (grid);
    }

}

// destroys the ui elements inside the grid
void ui_layout_grid_destroy_ui_elements (GridLayout *grid) {

    if (grid) {
        dlist_set_destroy (grid->elements, grid_element_delete_full);
        dlist_delete (grid->elements);

        grid->elements = dlist_init (grid_element_delete, NULL);
        ui_layout_grid_reset_pos_values (grid);
    }

}