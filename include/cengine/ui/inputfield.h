#ifndef _CENGINE_UI_INPUT_FIELD_H_
#define _CENGINE_UI_INPUT_FIELD_H_

#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/config.h"
#include "cengine/renderer.h"

#include "cengine/ui/types/types.h"
#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"

struct _Renderer;
struct _UIElement;

typedef struct InputField {

	struct _UIElement *ui_element;

	// background
	bool colour;
	RGBA_Color bg_colour;
	SDL_Texture *bg_texture;
	SDL_Rect bg_texture_rect;

	// outline
	bool outline;
	RGBA_Color outline_colour;
	float outline_scale_x;
	float outline_scale_y;

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

	// action to be triggered every key input
	Action on_key_input;
	void *on_key_input_args;

} InputField;

CENGINE_PUBLIC void ui_input_field_delete (void *input_ptr);

// sets the input field to be active depending on values
CENGINE_EXPORT void ui_input_field_set_active (InputField *input, bool active);

// toggles the input field to be active or not
CENGINE_EXPORT void ui_input_field_toggle_active (InputField *input);

// sets the inputs's UI position
CENGINE_EXPORT void ui_input_field_set_pos (InputField *input, UIRect *ref_rect, UIPosition pos, struct _Renderer *renderer);

// sets the input's position offset
CENGINE_EXPORT void ui_input_set_pos_offset (InputField *input, int x_offset, int y_offset);

// sets the input placeholder text
CENGINE_EXPORT void ui_input_field_placeholder_text_set (InputField *input, struct _Renderer *renderer, const char *text,
	Font *font, u32 size, RGBA_Color text_color);

// updates the input's placeholder text
CENGINE_EXPORT void ui_input_field_placeholder_text_update (InputField *input, struct _Renderer *renderer, const char *text);

// sets the input placeholder text position
CENGINE_EXPORT void ui_input_field_placeholder_text_pos_set (InputField *input, UIPosition pos);

// sets the input placeholder text position offset
CENGINE_EXPORT void ui_input_field_placeholder_text_pos_offset_set (InputField *input, int x_offset, int y_offset);

// updates the input's placeholder text position
CENGINE_EXPORT void ui_input_field_placeholder_text_pos_update (InputField *input);

// sets the input field's text
CENGINE_EXPORT void ui_input_field_text_set (InputField *input, struct _Renderer *renderer, const char *text,
	Font *font, u32 size, RGBA_Color text_color, bool is_password);

// updates the placeholder text (redraws the text component)
CENGINE_EXPORT void ui_input_field_text_update (InputField *input, struct _Renderer *renderer, const char *update_text);

// sets the input field's text position
CENGINE_EXPORT void ui_input_field_text_pos_set (InputField *input, UIPosition pos);

// sets the input text position offset
CENGINE_EXPORT void ui_input_field_text_pos_offset_set (InputField *input, int x_offset, int y_offset);

// updates the input's text position
CENGINE_EXPORT void ui_input_field_text_pos_update (InputField *input);

// sets the input field's text color
CENGINE_EXPORT void ui_input_field_text_color_set (InputField *input, struct _Renderer *renderer, RGBA_Color color);

// returns the current input text
CENGINE_EXPORT String *ui_input_field_input_get (InputField *input);

// returns the actual password value
CENGINE_EXPORT String *ui_input_field_password_get (InputField *input);

// sets the input field's outline colour
CENGINE_EXPORT void ui_input_field_ouline_set_colour (InputField *input, RGBA_Color colour);

// sets the input field's outline scale
CENGINE_EXPORT void ui_input_field_ouline_set_scale (InputField *input, float x_scale, float y_scale);

// removes the ouline form the input field
CENGINE_EXPORT void ui_input_field_outline_remove (InputField *input);

// sets the input field's background color
CENGINE_EXPORT void ui_input_field_bg_color_set (InputField *input, struct _Renderer *renderer, RGBA_Color color);

// removes the background from the input field
CENGINE_EXPORT void ui_input_field_bg_remove (InputField *input);

// sets the option to draw an outline rect when the input is selected
CENGINE_EXPORT void ui_input_field_selected_set (InputField *input, RGBA_Color selected_color);

// sets an action to be triggered every input
// works for every keystroke, paste, delete
CENGINE_EXPORT void ui_input_field_set_on_key_input (InputField *input, Action on_key_input, void *on_key_input_args);

// creates a new input field
CENGINE_EXPORT InputField *ui_input_field_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos, struct _Renderer *renderer);

// draws the input field
CENGINE_PRIVATE void ui_input_field_draw (InputField *input, struct _Renderer *renderer);

// updates the input field with the correct values
CENGINE_PRIVATE void ui_input_field_update (InputField *input, struct _Renderer *renderer);

#endif