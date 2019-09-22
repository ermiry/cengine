#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"

#include "cengine/ui/check.h"
#include "cengine/ui/components/transform.h"

static Check *ui_check_new (void) {

    Check *check = (Check *) malloc (sizeof (Check));
    if (check) {
        memset (check, 0, sizeof (Check));
        check->ui_element = NULL;
        check->transform = NULL; 
    }

    return check;

}

void ui_check_delete (void *check_ptr) {

    if (check_ptr) {
        Check *check = (Check *) check_ptr;

        check->ui_element = NULL;
        ui_transform_component_delete (check->transform);

        free (check);
    }

}

// creates a new check
// x and y for position
Check *ui_check_create (u32 x, u32 y) {

    Check *check = NULL;

    UIElement *ui_element = ui_element_new (UI_CHECK);
    if (ui_element) {
        check = ui_check_new ();
        if (check) {
            check->ui_element = ui_element;
            check->transform = ui_transform_component_create (x, y, 0, 0);
            ui_element->element = check;
        }
    }

    return check;

}

// draws the check to the screen
void ui_check_draw (Check *check) {

    if (check) {
        
    }

}