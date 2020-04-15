#ifndef _CENGINE_UI_BUTTON_H_
#define _CENGINE_UI_BUTTON_H_

#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/video.h"
#include "cengine/renderer.h"
#include "cengine/sprites.h"
#include "cengine/timer.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/components/text.h"

#define BUTTON_DEFAULT_DOUBLE_CLICK_DELAY           500

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

    bool active;

    // background
    RGBA_Color bg_colour;
    bool colour;
    SDL_Texture *bg_texture;
    SDL_Rect bg_texture_rect;

    bool outline;
    RGBA_Color outline_colour;
    float outline_scale_x;
    float outline_scale_y;

    Text *text;

    Sprite **sprites;
    bool *ref_sprites;

    // event listener
    bool pressed;
    Action action;
    void *args;

    // media
    u32 original_w, original_h;

    // double click
    bool one_click;
    Timer *double_click_timer;
    Action double_click_action;
    void *double_click_args;
    u32 double_click_delay;

} Button;

extern void ui_button_delete (void *button_ptr);

// sets the buttons's UI position
extern void ui_button_set_pos (Button *button, UIRect *ref_rect, UIPosition pos, Renderer *renderer);

// sets the button's position offset
extern void ui_button_set_pos_offset (Button *button, int x_offset, int y_offset);

// sets the button's render dimensions
extern void ui_button_set_dimensions (Button *button, unsigned int width, unsigned int height);

// sets the button's scale factor
extern void ui_button_set_scale (Button *button, int x_scale, int y_scale);

// sets the button to be active depending on values
extern void ui_button_set_active (Button *button, bool active);

// toggles the button to be active or not
extern void ui_button_toggle_active (Button *button);

// sets the button text
extern void ui_button_set_text (Button *button, Renderer *renderer, const char *text, 
    Font *font, u32 size, RGBA_Color text_color);

// updates the button's text
extern void ui_button_update_text (Button *button, Renderer *renderer, const char *text);

// sets the button's text position
extern void ui_button_set_text_pos (Button *button, UIPosition pos);

// sets the button's text offset
extern void ui_button_set_text_pos_offset (Button *button, int x_offset, int y_offset);

// 16/03/2020 -- used when the parent component's position has been updated
// NOTE: to work properly, a text pos needs to be set before calling this method
extern void ui_button_update_text_pos (Button *button);

// sets the button font
extern void ui_button_set_font (Button *button, Renderer *renderer, Font *font);

// sets the button's text color
extern void ui_button_set_text_color (Button *button, Renderer *renderer, RGBA_Color color);

// sets the button's outline colour
extern void ui_button_set_ouline_colour (Button *button, RGBA_Color colour);

// sets the button's outline scale
extern void ui_button_set_ouline_scale (Button *button, float x_scale, float y_scale);

// removes the ouline form the button
extern void ui_button_remove_outline (Button *button);

// sets the background color of the button
extern void ui_button_set_bg_color (Button *button, Renderer *renderer, RGBA_Color color);

// removes the background from the button
extern void ui_button_remove_background (Button *button);

// sets an sprite for each button state
// the sprite is loaded and deleted when the button gets deleted
extern void ui_button_set_sprite (Button *button, Renderer *renderer, ButtonState state, const char *filename);

// uses a refrence to the sprite and does not load or destroy it 
extern void ui_button_ref_sprite (Button *button, ButtonState state, Sprite *sprite);

// sets an action to be triggered when the button is clicked
extern void ui_button_set_action (Button *button, Action action, void *args);

// sets an action to be executed if double click is detected
extern void ui_button_set_double_click_action (Button *button, Action action, void *args);

// sets the max delay between two clicks to count as a double click (in mili secs), the default value is 500
extern void ui_button_set_double_click_delay (Button *button, u32 double_click_delay);

// creates a new button
extern Button *ui_button_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos, Renderer *renderer);

// rezises the button based on window size
extern void ui_button_resize (Button *button, WindowSize window_original_size, WindowSize window_new_size);

// draws a button
extern void ui_button_draw (Button *button, Renderer *renderer);

#endif