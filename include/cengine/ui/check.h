#ifndef _CENGINE_UI_CHECK_H_
#define _CENGINE_UI_CHECK_H_

#include "cengine/ui/components/transform.h"

typedef struct Check {

    UIElement *ui_element;
    UITransform *transform;

} Check;

extern void ui_check_delete (void *check_ptr);

// creates a new check
// x and y for position
extern Check *ui_check_create (u32 x, u32 y);

// draws the check to the screen
extern void ui_check_draw (Check *check);

#endif