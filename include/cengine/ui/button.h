#ifndef _CENGINE_UI_BUTTON_H_
#define _CENGINE_UI_BUTTON_H_

#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/sprites.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"

typedef enum ButtonState {

    BUTTON_STATE_MOUSE_OUT              = 0,
    BUTTON_STATE_MOUSE_OVER_MOTION      = 1,
    BUTTON_STATE_MOUSE_DOWN             = 2,
    BUTTON_STATE_MOUSE_UP               = 3,
    BUTTON_STATE_DISABLE                = 4,
    BUTTON_STATE_TOTAL                  = 5

} ButtonState;

typedef struct Button {

    UIElement *ui_element;

    UITransform *transform;

    bool active;

    // background
    RGBA_Color bg_colour;
    bool colour;
    SDL_Texture *bg_texture;
    SDL_Rect bg_texture_rect;

    bool outline;
    RGBA_Color outline_colour;

    Text *text;

    Sprite **sprites;
    bool *ref_sprites;

    // event listener
    bool pressed;
    Action action;
    void *args;

} Button;

extern void ui_button_delete (void *button_ptr);

// sets the button to be active depending on values
extern void ui_button_set_active (Button *button, bool active);

// toggles the button to be active or not
extern void ui_button_toggle_active (Button *button);

// sets the button text
extern void ui_button_set_text (Button *button, const char *text, 
    Font *font, u32 size, RGBA_Color text_color);

// sets the button's text position
extern void ui_button_set_text_pos (Button *button, UIPosition pos);

// sets the button's text color
extern void ui_button_set_text_color (Button *button, RGBA_Color color);

// sets the button's outline colour
extern void ui_button_set_ouline_colour (Button *button, RGBA_Color colour);

// removes the ouline form the button
extern void ui_button_remove_outline (Button *button);

// sets the background color of the button
extern void ui_button_set_bg_color (Button *button, RGBA_Color color);

// removes the background from the button
extern void ui_button_remove_background (Button *button);

// sets an sprite for each button state
// the sprite is loaded and deleted when the button gets deleted
extern void ui_button_set_sprite (Button *button, ButtonState state, const char *filename);

// uses a refrence to the sprite and does not load or destroy it 
extern void ui_button_ref_sprite (Button *button, ButtonState state, Sprite *sprite);

// sets an action to be triggered when the button is clicked
extern void ui_button_set_action (Button *button, Action action, void *args);

// creates a new button
extern Button *ui_button_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos);

// draws a button
extern void ui_button_draw (Button *button);

#endif