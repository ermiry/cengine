#ifndef _CENGINE_UI_CHECK_H_
#define _CENGINE_UI_CHECK_H_

#include "cengine/types/types.h"

#include "cengine/renderer.h"
#include "cengine/sprites.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"

extern u8 ui_default_assets_load_checks (void);

typedef enum CheckShape {

	UI_CHECK_SHAPE_NONE			= 0,
	UI_CHECK_SHAPE_ROUND      	= 1,
	UI_CHECK_SHAPE_SQUARE     	= 2,

} CheckShape;

typedef enum CheckColour {

	UI_CHECK_COLOUR_NONE		= 0,
	UI_CHECK_COLOUR_RED			= 1,
	UI_CHECK_COLOUR_GREEN		= 2,
	UI_CHECK_COLOUR_BLUE		= 3

} CheckColour;

typedef struct Check {

	UIElement *ui_element;

	// 2/01/2020 -- checks can only use the default assets check.png spritesheet
	SpriteSheet *sprite_sheet;
	u32 x_sprite_offset, y_sprite_offset;

	CheckShape shape;
	CheckColour colour;

} Check;

extern void ui_check_delete (void *check_ptr);

// creates a new check
// x and y for position
extern Check *ui_check_create (i32 x, i32 y, Renderer *renderer);

// draws the check to the screen
extern void ui_check_draw (Check *check, Renderer *renderer);

#endif