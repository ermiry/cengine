#include <stdlib.h>

#include "cengine/types/point2d.h"

Point2D *point_new (void) {

    Point2D *point = (Point2D *) malloc (sizeof (Point2D));
    if (point) point->x = point->y = 0;
    return point;

}

Point2D *point_create (int x, int y) {

    Point2D *point = (Point2D *) malloc (sizeof (Point2D));
    if (point) {
        point->x = x;
        point->y = y;
    }

    return point;

}

void point_delete (void *point_ptr) { if (point_ptr) free (point_ptr); }

// compares the two point's x and y values
// retunrs 0 if they are exactly the same
// uses x value to check for lower and higher values
int point_x_comparator (const void *one, const void *two) {

    if (one && two) {
        Point2D *point_one = (Point2D *) one;
        Point2D *point_two = (Point2D *) two;

        if ((point_one->x == point_two->x) && (point_one->y == point_two->y)) return 0;
        else if (point_one->x < point_two->x) return -1;
        else return 1;
    }

    else if (one) return -1;
    else return 1;

}

// compares the two point's x and y values
// retunrs 0 if they are exactly the same
// uses y value to check for lower and higher values
int point_y_comparator (const void *one, const void *two) {

    if (one && two) {
        Point2D *point_one = (Point2D *) one;
        Point2D *point_two = (Point2D *) two;

        if ((point_one->x == point_two->x) && (point_one->y == point_two->y)) return 0;
        else if (point_one->y < point_two->y) return -1;
        else return 1;
    }

    else if (one) return -1;
    else return 1;

}