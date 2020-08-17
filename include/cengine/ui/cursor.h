#ifndef _CENGINE_UI_CURSOR_H_
#define _CENGINE_UI_CURSOR_H_

#include <stdbool.h>

#include "cengine/types/types.h"

#include "cengine/config.h"
#include "cengine/renderer.h"
#include "cengine/sprites.h"

#define CURSOR_DEFAULT_WIDTH            8
#define CURSOR_DEFAULT_HEIGHT           8
#define CURSOR_DEFAULT_HOT_X            4
#define CURSOR_DEFAULT_HOT_Y            4

typedef struct Cursor {

	SDL_Cursor *cursor; 
	int w, h;
	int hot_x, hot_y;
	
	Sprite *sprite;
	SpriteSheet *sprite_sheet;
	bool refSprite;

	u32 x_sprite_offset, y_sprite_offset;
	bool multipleSprites;
	Flip flip;

} Cursor;

extern Cursor *main_cursor;

CENGINE_PUBLIC Cursor *ui_cursor_new (void);

CENGINE_PUBLIC void ui_cursor_delete (Cursor *cursor);

// create a new ui cursor with a custom sprite or pass a spirtesheet to load different images
// when performing different actions
CENGINE_EXPORT Cursor *ui_cursor_create (Renderer *renderer, const char *sprite, const char *sprite_sheet, 
	int w, int h, int hot_x, int hot_y);

CENGINE_PRIVATE void ui_cursor_draw (Cursor *cursor, Renderer *renderer);

// update cursor with new values
CENGINE_PRIVATE int ui_cursor_update (Cursor *c, int w, int h, int hot_x, int hot_y);

CENGINE_PUBLIC int ui_cursor_get_width (Cursor *c);

CENGINE_PUBLIC int ui_cursor_set_width (Cursor *c, int new_width);

CENGINE_PUBLIC int ui_cursor_get_height (Cursor *c);

CENGINE_PUBLIC int ui_cursor_set_height (Cursor *c, int new_height);

CENGINE_PUBLIC int ui_cursor_get_hot_x (Cursor *c);

CENGINE_PUBLIC int ui_cursor_get_hot_y (Cursor *c);

#endif