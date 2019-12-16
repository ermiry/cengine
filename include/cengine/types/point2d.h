#ifndef _CENGINE_TYPES_POINT2D_H_
#define _CENGINE_TYPES_POINT2D_H_

typedef struct Point2D {

    int x, y;

} Point2D;

extern Point2D *point_new (void);

extern Point2D *point_create (int x, int y);

extern void point_delete (void *point_ptr);

// compares the two point's x and y values
// retunrs 0 if they are exactly the same
// uses x value to check for lower and higher values
extern int point_x_comparator (const void *one, const void *two);

// compares the two point's x and y values
// retunrs 0 if they are exactly the same
// uses y value to check for lower and higher values
extern int point_y_comparator (const void *one, const void *two);

#endif