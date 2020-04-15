#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL_rect.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/assets.h"
#include "cengine/renderer.h"
#include "cengine/sprites.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/check.h"
#include "cengine/ui/components/transform.h"

#pragma region internal

static SpriteSheet *checks_spritesheet = NULL;

u8 ui_default_assets_load_checks (void) {

    u8 retval = 1;

    if (ui_default_assets_path) {
        String *checks_spritesheet_filename = str_create ("%schecks.png", ui_default_assets_path->str);
        if (checks_spritesheet_filename) {
            Renderer *renderer = renderer_get_by_name ("main");
            checks_spritesheet = sprite_sheet_load (checks_spritesheet_filename->str, renderer);
            if (checks_spritesheet) {
                sprite_sheet_set_sprite_size (checks_spritesheet, 64, 64);
                sprite_sheet_set_scale_factor (checks_spritesheet, 1);
                sprite_sheet_crop (checks_spritesheet);
            }
            
            str_delete (checks_spritesheet_filename);
        }
    }

    return retval;

}

static Check *ui_check_new (void) {

    Check *check = (Check *) malloc (sizeof (Check));
    if (check) {
        memset (check, 0, sizeof (Check));
        check->ui_element = NULL;
    }

    return check;

}

#pragma endregion

void ui_check_delete (void *check_ptr) {

    if (check_ptr) {
        Check *check = (Check *) check_ptr;

        check->ui_element = NULL;

        free (check);
    }

}

// creates a new check
// x and y for position
Check *ui_check_create (i32 x, i32 y, Renderer *renderer) {

    Check *check = NULL;

    UIElement *ui_element = ui_element_create (renderer->ui, UI_CHECK);
    if (ui_element) {
        check = ui_check_new ();
        if (check) {
            check->ui_element = ui_element;

            ui_transform_component_set_values (check->ui_element->transform, x, y, 0, 0);

            ui_element->element = check;
        }
    }

    return check;

}

// draws the check to the screen
void ui_check_draw (Check *check, Renderer *renderer) {

    if (check && renderer) {
        if (SDL_HasIntersection (&check->ui_element->transform->rect, &renderer->window->screen_rect)) {

            renderer->render_count += 1;
        }
    }

}