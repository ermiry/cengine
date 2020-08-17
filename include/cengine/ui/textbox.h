#ifndef _CENGINE_UI_TEXTBOX_H_
#define _CENGINE_UI_TEXTBOX_H_

#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/config.h"
#include "cengine/graphics.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/components/text.h"

typedef struct TextBox {

	UIElement *ui_element;

	bool outline;
	RGBA_Color outline_colour;
	float outline_scale_x;
	float outline_scale_y;

	bool colour;
	RGBA_Color bg_colour;
	SDL_Texture *bg_texture;
	SDL_Rect bg_texture_rect;

	Text *text;

	// meida 
	u32 original_w, original_h;

} TextBox;

CENGINE_PUBLIC void ui_textbox_delete (void *textbox_ptr);

// returns the string representing the text in the textbox
CENGINE_EXPORT String *ui_textbox_get_text (TextBox *textbox);

// sets the textbox's UI position
CENGINE_EXPORT void ui_textbox_set_pos (TextBox *textbox, UIRect *ref_rect, UIPosition pos, Renderer *renderer);

// sets the textbox's position offset
CENGINE_EXPORT void ui_textbox_set_pos_offset (TextBox *textbox, int x_offset, int y_offset);

// sets the textbox's text
CENGINE_EXPORT void ui_textbox_set_text (TextBox *textbox, Renderer *renderer, const char *text, 
	Font *font, u32 size, RGBA_Color color, bool adjust_to_text);

// updates the textbox's text
CENGINE_EXPORT void ui_textbox_update_text (TextBox *textbox, Renderer *renderer, const char *text);

// sets the textbox's text position
CENGINE_EXPORT void ui_textbox_set_text_pos (TextBox *textbox, UIPosition pos);

// sets the textbox's text offset
CENGINE_EXPORT void ui_textbox_set_text_pos_offset (TextBox *textbox, int x_offset, int y_offset);

// 04/02/2020 -- 12:03 -- used when the parent component's position has been updated
// NOTE: to work properly, a text pos needs to be set before calling this method
CENGINE_EXPORT void ui_textbox_update_text_pos (TextBox *textbox);

// sets the textbox font
CENGINE_EXPORT void ui_textbox_set_font (TextBox *textbox, Renderer *renderer, Font *font);

// sets the textbox's text color
CENGINE_EXPORT void ui_textbox_set_text_color (TextBox *textbox, Renderer *renderer, RGBA_Color color);

// sets the textbox's outline colour
CENGINE_EXPORT void ui_textbox_set_ouline_colour (TextBox *textbox, RGBA_Color colour);

// sets the textbox's outline scale
CENGINE_EXPORT void ui_textbox_set_ouline_scale (TextBox *textbox, float x_scale, float y_scale);

// removes the ouline form the textbox
CENGINE_EXPORT void ui_textbox_remove_outline (TextBox *textbox);

// sets the textbox's background color
CENGINE_EXPORT void ui_textbox_set_bg_color (TextBox *textbox, Renderer *renderer, RGBA_Color color);

// removes the background from the textbox
CENGINE_EXPORT void ui_textbox_remove_background (TextBox *textbox, Renderer *renderer);

// creates a new textbox
CENGINE_EXPORT TextBox *ui_textbox_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos, Renderer *renderer);

// rezises the textbox based on window size
CENGINE_PUBLIC void ui_textbox_resize (TextBox *textbox, WindowSize window_original_size, WindowSize window_new_size);

// draws the textbox
CENGINE_PUBLIC void ui_textbox_draw (TextBox *textbox, Renderer *renderer);

#endif