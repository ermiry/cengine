#ifndef _CENGINE_UI_BUTTON_H_
#define _CENGINE_UI_BUTTON_H_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/sprites.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"

typedef enum ButtonState {

    BUTTON_STATE_MOUSE_OUT = 0,
    BUTTON_STATE_MOUSE_OVER_MOTION = 1,
    BUTTON_STATE_MOUSE_DOWN = 2,
    BUTTON_STATE_MOUSE_UP = 3,
    BUTTON_STATE_TOTAL = 4

} ButtonState;

typedef struct Button {

    // text
    Font *font;
    RGBA_Color text_color;
    String *text;
    bool is_volatile;

    // background
    UIRect bgrect;
    RGBA_Color bgcolor;

    Sprite **sprites;
    bool *ref_sprites;

    SDL_Texture *texture;       // rendered text

    // event listener
    Action event;

} Button;

extern void button_delete (void *button_ptr);

// sets the button text
extern void ui_button_set_text (Button *button, const char *text);

// sets the button's text color
extern void ui_button_set_text_color (Button *button, RGBA_Color color);

// sets the background color of the button
extern void ui_button_set_bg_color (Button *button, RGBA_Color color);

// sets an sprite for each button state
// the sprite is loaded and deleted when the button gets deleted
extern void ui_button_set_sprite (Button *button, ButtonState state, const char *filename);

// uses a refrence to the sprite and does not load or destroy it 
extern void ui_button_ref_sprite (Button *button, ButtonState state, Sprite *sprite);

// sets an event to be triggered when the button is clicked
extern void ui_button_set_event (Button *button, Action event);

// draws a volatile button text
extern void ui_button_text_draw (Button *button);

// draws a button
extern void ui_button_draw (Button *button);

#endif