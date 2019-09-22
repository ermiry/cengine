#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/types/point2d.h"

#include "cengine/collections/dlist.h"

#include "cengine/math/plot.h"

#include "cengine/ui/ui.h"

#pragma region Plot Captions

static PlotCaption *math_plot_caption_new (void) {

    PlotCaption *caption = (PlotCaption *) malloc (sizeof (PlotCaption));
    if (caption) {
        memset (caption, 0, sizeof (PlotCaption));
        caption->name = NULL;
        caption->points = NULL;
    }

    return caption;

}

void math_plot_caption_delete (void *caption_ptr) {

    if (caption_ptr) {
        PlotCaption *caption = (PlotCaption *) caption_ptr;
        str_delete (caption->name);
        dlist_delete (caption->points);
        free (caption);
    }

}

int math_plot_caption_comparator (const void *one, const void *two) {

    return str_comparator (((PlotCaption *) one)->name, ((PlotCaption *) two)->name);

}

// creates a new caption to be added to a plot
PlotCaption *math_plot_caption_create (const char *name, RGBA_Color colour) {

    PlotCaption *caption = math_plot_caption_new ();
    if (caption) {
        caption->name = name ? str_new (name) : NULL;
        caption->colour = colour;
        caption->points = dlist_init (point_delete, point_x_comparator);
    }

    return caption;

}

// adds a new point to the caption
void math_plot_caption_point_add (PlotCaption *caption, int x, int y) {

    if (caption) {
        Point2D *point = point_create (x, y);
        dlist_insert_after (caption->points, dlist_end (caption->points), point);
    }

}

// removes a point from the caption
// returns 0 on success, 1 on error
int math_plot_caption_point_remove (PlotCaption *caption, int x, int y) {

    int retval = 1;

    if (caption) {
        Point2D *query = point_create (x, y);
        retval = dlist_remove (caption->points, query);
        point_delete (query);
    }

    return retval;

}

#pragma endregion

#pragma region Plot

static Plot *math_plot_new (void) {

    Plot *plot = (Plot *) malloc (sizeof (Plot));
    if (plot) {
        memset (plot, 0, sizeof (Plot));

        plot->title = NULL;
        plot->x_label = plot->y_label = NULL;
        plot->captions = NULL;
    }

    return plot;

}

void math_plot_delete (void *plot_ptr) {

    if (plot_ptr) {
        Plot *plot = (Plot *) plot_ptr;
        
        str_delete (plot->title);
        str_delete (plot->x_label);
        str_delete (plot->y_label);
        dlist_delete (plot->captions);

        free (plot);
    }

}

Plot *math_plot_create (PlotType type, 
    const char *title, const char *x_label, const char *y_label,
    u32 x_scale, u32 y_scale, i32 x_max, i32 y_max) {

    Plot *plot = math_plot_new ();
    if (plot) {
        plot->title = str_new (title);
        plot->x_label = str_new (x_label);
        plot->y_label = str_new (y_label);
        plot->captions = dlist_init (math_plot_caption_delete, math_plot_caption_comparator);

        plot->x_scale = x_scale;
        plot->y_scale = y_scale;

        plot->x_max = x_max;
        plot->y_max = y_max;
    }

    return plot;

}

// sets the plot's title
void math_plot_set_title (Plot *plot, const char *title) {

    if (plot) {
        str_delete (plot->title);
        plot->title = title ? str_new (title) : NULL;
    }

}

// sets the plot's labels' names
void math_plot_set_labels (Plot *plot, const char *x_label, const char *y_label) {

    if (plot) {
        str_delete (plot->x_label);
        plot->x_label = x_label? str_new (x_label) : NULL;

        str_delete (plot->y_label);
        plot->y_label = y_label? str_new (y_label) : NULL;
    }

}

// sets the plots constrain values
void math_plot_set_values (Plot *plot, u32 x_scale, u32 y_scale, i32 x_max, i32 y_max) {

    if (plot) {
        plot->x_scale = x_scale;
        plot->y_scale = y_scale;

        plot->x_max = x_max;
        plot->y_max = y_max;
    }

}

// gets a caption by name
PlotCaption *math_plot_get_caption (Plot *plot, const char *name) {

    if (plot) {
        PlotCaption *caption = NULL;
        for (ListElement *le = dlist_start (plot->captions); le; le = le->next) {
            caption = (PlotCaption *) le->data;
            if (!strcmp (caption->name->str, name)) return caption;
        }
    }

    return NULL;

}

// adds a new caption to the plot's captions
void math_plot_add_caption (Plot *plot, PlotCaption *caption) {

    if (plot && caption) dlist_insert_after (plot->captions, dlist_end (plot->captions), caption);

}

// removes a caption from the plot's captions
void math_plot_remove_caption (Plot *plot, const char *name) {

    if (plot && name) {
        for (ListElement *le = dlist_start (plot->captions); le; le = le->next) {
            if (!strcmp (((PlotCaption *) le->data)->name->str, name)) {
                math_plot_caption_delete (dlist_remove_element (plot->captions, le));
                break;
            }
        }
    }

}

#pragma endregion