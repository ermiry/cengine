#ifndef _CENGINE_UI_TEXTBOX_H_
#define _CENGINE_UI_TEXTBOX_H_

#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"

typedef struct TextBox {

    UIElement *ui_element;

    UITransform *transform;

    bool outline;
    RGBA_Color outline_colour;

    bool colour;
    RGBA_Color bg_colour;
    SDL_Texture *bg_texture;
    SDL_Rect bg_texture_rect;

    Text *text;

} TextBox;

extern void ui_textbox_delete (void *textbox_ptr);

// returns the string representing the text in the textbox
extern String *ui_textbox_get_text (TextBox *textbox);

// sets the textbox's text
extern void ui_textbox_set_text (TextBox *textbox, const char *text, 
    Font *font, u32 size, RGBA_Color color, bool adjust_to_text);

// updates the textbox's text
extern void ui_textbox_update_text (TextBox *textbox, const char *text);

// sets the textbox's text position
extern void ui_textbox_set_text_pos (TextBox *textbox, UIPosition pos);

// sets the textbox font
extern void ui_textbox_set_font (TextBox *textbox, Font *font);

// sets the textbox's text color
extern void ui_textbox_set_text_color (TextBox *textbox, RGBA_Color color);

// sets the textbox's outline colour
extern void ui_textbox_set_ouline_colour (TextBox *textbox, RGBA_Color colour);

// removes the ouline form the textbox
extern void ui_textbox_remove_outline (TextBox *textbox);

// sets the textbox's background color
extern void ui_textbox_set_bg_color (TextBox *textbox, RGBA_Color color);

// removes the background from the textbox
extern void ui_textbox_remove_background (TextBox *textbox);

// creates a new textbox
extern TextBox *ui_textbox_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos);

// draws the textbox
extern void ui_textbox_draw (TextBox *textbox);

#endif