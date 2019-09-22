#ifndef _CENGINE_MATH_PLOT_H_
#define _CENGINE_MATH_PLOT_H_

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/collections/dlist.h"

#include "cengine/ui/ui.h"

typedef enum PlotType {

    PLOT_LINE,
    PLOT_SCATTER,
    PLOT_LINE_SCATTER,
    PLOT_STEP,
    PLOT_STEP_FILLED

} PlotType;

typedef struct PlotCaption {

    String *name;
    RGBA_Color colour;
    DoubleList *points;

} PlotCaption;

extern void math_plot_caption_delete (void *caption_ptr);

extern int math_plot_caption_comparator (const void *one, const void *two);

// creates a new caption to be added to a plot
extern PlotCaption *math_plot_caption_create (const char *name, RGBA_Color colour);

// adds a new point to the caption
extern void math_plot_caption_point_add (PlotCaption *caption, int x, int y);

// removes a point from the caption
// returns 0 on success, 1 on error
extern int math_plot_caption_point_remove (PlotCaption *caption, int x, int y);

typedef struct Plot {

    PlotType type;

    String *title;
    String *x_label, *y_label;
    DoubleList *captions;

    u32 x_scale, y_scale;
    i32 x_max, y_max;

} Plot;

extern void math_plot_delete (void *plot_ptr);

extern Plot *math_plot_create (PlotType type, 
    const char *title, const char *x_label, const char *y_label,
    u32 x_scale, u32 y_scale, i32 x_max, i32 y_max);

// sets the plot's title
extern void math_plot_set_title (Plot *plot, const char *title);

// sets the plot's labels' names
extern void math_plot_set_labels (Plot *plot, const char *x_label, const char *y_label);

// sets the plots constrain values
extern void math_plot_set_values (Plot *plot, u32 x_scale, u32 y_scale, i32 x_max, i32 y_max);

// gets a caption by name
extern PlotCaption *math_plot_get_caption (Plot *plot, const char *name);

// adds a new caption to the plot's captions
extern void math_plot_add_caption (Plot *plot, PlotCaption *caption);

// removes a caption from the plot's captions
extern void math_plot_remove_caption (Plot *plot, const char *name);

#endif