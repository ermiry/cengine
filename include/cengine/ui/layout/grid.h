#ifndef _CENGINE_UI_LAYOUT_GRID_H_
#define _CENGINE_UI_LAYOUT_GRID_H_

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/renderer.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/layout/align.h"

#define GRID_LAYOUT_DEFAULT_SCROLL          5

struct _Renderer;

typedef struct GridElement {

    UIElement *ui_element;
    u32 original_width, original_height;
    u32 x, y;

} GridElement;

typedef struct GridLayout {

    // usefull reference to the window's renderer
    struct _Renderer *renderer;

    UITransform *transform;

    DoubleList *elements;
    u32 cols, rows;
    
    u32 x_count;
    u32 y_count;

    u32 x_offset;
    u32 y_offset;

    u32 n_elements_x_row;
    u32 n_elements_x_col;

    u32 cell_width;
    u32 cell_height;
    u32 cell_padding_x;
    u32 cell_padding_y;

    AxisAlignment align_x;
    AxisAlignment align_y;

    bool scrolling;             // enable / disable scrolling
    u32 scroll_sensitivity;     // how fast the elements move

} GridLayout;

extern void ui_layout_grid_delete (void *grid_ptr);

// sets the number of columns and rows available for the grid
extern void ui_layout_grid_set_grid (GridLayout *grid, u32 cols, u32 rows);

// sets the number of elements x row
extern void ui_layout_grid_set_elements_x_row (GridLayout *grid, u32 n_elements_x_row);

// sets the number of elements x col
extern void ui_layout_grid_set_elements_x_col (GridLayout *grid, u32 n_elements_x_col);

// sets the preferred size of the cells, the layout must be of the correct size
extern void ui_layout_grid_set_cell_size (GridLayout *grid, u32 width, u32 height);

// sets the cell padding - distance between cells
extern void ui_layout_grid_set_cell_padding (GridLayout *grid, u32 cell_padding_x, u32 cell_padding_y);

// sets the alignment to use in the x axis
extern void ui_layout_grid_set_x_axis_alignment (GridLayout *grid, AxisAlignment align);

// sets the alignment to use in the y axis
extern void ui_layout_grid_set_y_axis_alignment (GridLayout *grid, AxisAlignment align);

// enables / disbale scrolling in the grid layout
extern void ui_layout_grid_toggle_scrolling (GridLayout *grid, bool enable);

// sets how fast the elements move when scrolling, the default is a value of 5
extern void ui_layout_grid_set_scroll_sensitivity (GridLayout *grid, u32 sensitivity);

// updates the grid with a new size
// returns 0 on success update, 1 on failure
extern int ui_layout_grid_update_dimensions (GridLayout *grid, u32 cols, u32 rows);

// creates a new grid layout
// x and y for position, w and h for dimensions
extern GridLayout *ui_layout_grid_create (i32 x, i32 y, u32 w, u32 h, struct _Renderer *renderer);

// adds a new element to the grid in the specified pos (0 indexed)
// if its greater than the current size, it will be added at the end
// returns 0 on success, 1 if failed to add
extern u8 ui_layout_grid_add_element_at_pos (GridLayout *grid, UIElement *ui_element, u32 pos);

// adds a new element at the end of the grid
// this method avoids updating all the other elements positions as well
// returns 0 on success, 1 on error
extern u8 ui_layout_grid_add_element_at_end (GridLayout *grid, UIElement *ui_element);

// returns the ui element that is at the required position in the layout
extern UIElement *ui_layout_grid_get_element_at (GridLayout *grid, unsigned int pos);

// removes an element from the grid
extern void ui_layout_grid_remove_element (GridLayout *grid, UIElement *ui_element);

// removes all ui elements from the grid layout without destroying them
extern void ui_layout_grid_remove_ui_elements (GridLayout *grid);

// destroys the ui elements inside the grid
extern void ui_layout_grid_destroy_ui_elements (GridLayout *grid);

#endif