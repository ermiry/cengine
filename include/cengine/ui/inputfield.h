#ifndef _CENGINE_UI_INPUT_FIELD_H_
#define _CENGINE_UI_INPUT_FIELD_H_

#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"

typedef struct InputField {

    UIElement *ui_element;
    UITransform *transform;

    // background
    bool colour;
    RGBA_Color bg_colour;
    SDL_Texture *bg_texture;
    SDL_Rect bg_texture_rect;

    // outline
    bool outline;
    RGBA_Color outline_colour;
    bool draw_selected;
    RGBA_Color selected_color;

    // placeholder text
    Text *placeholder;
    bool empty_text;

    // text
    Text *text;
    bool is_password;
    String *password;

    bool pressed;
    bool active;

} InputField;

extern void ui_input_field_delete (void *input_ptr);

// sets the input field to be active depending on values
extern void ui_input_field_set_active (InputField *input, bool active);

// toggles the input field to be active or not
extern void ui_input_field_toggle_active (InputField *input);

// sets the input placeholder text
extern void ui_input_field_placeholder_text_set (InputField *input, const char *text,
    Font *font, u32 size, RGBA_Color text_color);

// updates the input's placeholder text
extern void ui_input_field_placeholder_text_update (InputField *input, const char *text);

// sets the input placeholder text position
extern void ui_input_field_placeholder_text_pos_set (InputField *input, UIPosition pos);

// sets the input field's text
extern void ui_input_field_text_set (InputField *input, const char *text,
    Font *font, u32 size, RGBA_Color text_color, bool is_password);

// updates the placeholder text (redraws the text component)
extern void ui_input_field_text_update (InputField *input, const char *update_text);

// sets the input field's text position
extern void ui_input_field_text_pos_set (InputField *input, UIPosition pos);

// sets the input field's text color
extern void ui_input_field_text_color_set (InputField *input, RGBA_Color color);

// returns the current input text
extern String *ui_input_field_input_get (InputField *input);

// returns the actual password value
extern String *ui_input_field_password_get (InputField *input);

// sets the input field's outline colour
extern void ui_input_field_ouline_set_colour (InputField *input, RGBA_Color colour);

// removes the ouline form the input field
extern void ui_input_field_outline_remove (InputField *input);

// sets the input field's background color
extern void ui_input_field_bg_color_set (InputField *input, RGBA_Color color);

// removes the background from the input field
extern void ui_input_field_bg_remove (InputField *input);

// sets the option to draw an outline rect when the input is selected
extern void ui_input_field_selected_set (InputField *input, RGBA_Color selected_color);

// creates a new input field
extern InputField *ui_input_field_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos);

// draws the input field
extern void ui_input_field_draw (InputField *input);

// updates the input field with the correct values
extern void ui_input_field_update (InputField *input);

#endif