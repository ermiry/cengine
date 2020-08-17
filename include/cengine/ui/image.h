#ifndef _CENGINE_UI_IMAGE_H_
#define _CENGINE_UI_IMAGE_H_

#include <stdbool.h>

#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"

#include "cengine/config.h"
#include "cengine/renderer.h"
#include "cengine/sprites.h"
#include "cengine/timer.h"

#include "cengine/ui/components/transform.h"

#define IMAGE_DEFAULT_DOUBLE_CLICK_DELAY           500

typedef struct Image {

	UIElement *ui_element;

	bool texture_reference;
	SDL_Texture *texture;
	SDL_Rect *texture_src_rect;

	Sprite *sprite;
	SpriteSheet *sprite_sheet;
	u32 x_sprite_offset, y_sprite_offset;
	
	bool ref_sprite;
	Flip flip;

	bool outline;
	RGBA_Color outline_colour;
	float outline_scale_x;
	float outline_scale_y;

	// event listener
	bool active;
	bool pressed;
	bool selected;
	Action action;
	void *args;

	// double click
	bool one_click;
	Timer *double_click_timer;
	Action double_click_action;
	void *double_click_args;
	u32 double_click_delay;

	bool overlay_reference;
	SDL_Texture *overlay_texture;

	bool selected_reference;
	SDL_Texture *selected_texture;

} Image;

CENGINE_PUBLIC void ui_image_delete (void *image_ptr);

// sets the image's UI position
CENGINE_EXPORT void ui_image_set_pos (Image *image, UIRect *ref_rect, UIPosition pos, Renderer *renderer);

// directly sets the image's texture
CENGINE_EXPORT void ui_image_set_texture (Image *image, SDL_Texture *texture);

// sets the image's texture using a refrence to another texture; when the image gets destroyted,
// the texture won't be deleted
CENGINE_EXPORT void ui_image_set_texture_ref (Image *image, SDL_Texture *texture_ref);

// sets the image's texture's source rect (used to give an offset to the texture)
CENGINE_EXPORT void ui_image_set_texture_src_rect (Image *image,
    int x, int y, int w, int h);

// sets the image's render dimensions
CENGINE_EXPORT void ui_image_set_dimensions (Image *image, unsigned int width, unsigned int height);

// sets the image's scale factor
CENGINE_EXPORT void ui_image_set_scale (Image *image, int x_scale, int y_scale);

// sets the image's sprite to be rendered and loads its
// returns 0 on success loading sprite, 1 on error
CENGINE_EXPORT u8 ui_image_set_sprite (Image *image, Renderer *renderer, const char *filename);

// sets the image's sprite sheet to be rendered and loads it
// returns 0 on success loading sprite sheet, 1 on error
CENGINE_EXPORT u8 ui_image_set_sprite_sheet (Image *image, Renderer *renderer, const char *filename);

// sets the image's sprite from a refrence from an already loaded sprite
CENGINE_EXPORT void ui_image_ref_sprite (Image *image, Sprite *sprite);

// sets the image's sprite sheet from a refrence from an already loaded sprite sheet
CENGINE_EXPORT void ui_image_ref_sprite_sheet (Image *image, SpriteSheet *sprite_sheet);

// sets the image's sprite sheet offset
CENGINE_EXPORT void ui_image_set_sprite_sheet_offset (Image *image, u32 x_offset, u32 y_offset);

// sets the image's outline colour
CENGINE_EXPORT void ui_image_set_ouline_colour (Image *image, RGBA_Color colour);

// sets the image's outline scale
CENGINE_EXPORT void ui_image_set_outline_scale (Image *image, float x_scale, float y_scale);

// removes the ouline form the image
CENGINE_EXPORT void ui_image_remove_outline (Image *image);

// sets the image to be active depending on value
// action listerner working
CENGINE_EXPORT void ui_image_set_active (Image *image, bool active);

// toggles the image to be active or not
// action listerner working
CENGINE_EXPORT void ui_image_toggle_active (Image *image);

// sets an action to be triggered when the image is clicked
CENGINE_EXPORT void ui_image_set_action (Image *image, Action action, void *args);

// sets an action to be executed if double click is dected
CENGINE_EXPORT void ui_image_set_double_click_action (Image *image, Action action, void *args);

// sets the max delay between two clicks to count as a double click (in mili secs), the default value is 500
CENGINE_EXPORT void ui_image_set_double_click_delay (Image *image, u32 double_click_delay);

// sets an overlay to the image that only renders when you hover the image
CENGINE_EXPORT void ui_image_set_overlay (Image *image, Renderer *renderer, RGBA_Color color);

// sets an overlay to the image that only renders when you hover the image
// you need to pass a refrence to the texture
CENGINE_EXPORT void ui_image_set_overlay_ref (Image *image, SDL_Texture *overlay_ref);

// removes the overlay from the image
CENGINE_EXPORT void ui_image_remove_overlay (Image *image);

// sets an overlay to the image that only renders when you select the image (1 left click)
CENGINE_EXPORT void ui_image_set_selected (Image *image, Renderer *renderer, RGBA_Color color);

// sets an overlay to the image that only renders when you select the image
// you need to pass a reference to the texture
CENGINE_EXPORT void ui_image_set_selected_ref (Image *image, SDL_Texture *selected_ref);

// removes the select overlay from the image
CENGINE_EXPORT void ui_image_remove_selected (Image *image);

// creates a new image to be displayed from a constant source, like using a sprite loaded from a file
// x and y for position
// w and h for dimensions
CENGINE_EXPORT Image *ui_image_create (u32 x, u32 y, u32 w, u32 h, Renderer *renderer);

CENGINE_PUBLIC u8 ui_image_update_texture_from_mem (Image *image, Renderer *renderer, void *mem, int mem_size);

// manually creates a streaming access texture, usefull for constant updates
CENGINE_PUBLIC u8 ui_image_create_streaming_texture (Image *image, Renderer *renderer, Uint32 sdl_pixel_format);

// updates the streaming texture using an in memory buffer representing an image
// NOTE: buffer is not freed
CENGINE_PUBLIC u8 ui_image_update_streaming_texture_mem (Image *image, Renderer *renderer, void *mem, int mem_size);

// creates an image that is ment to be updated directly and constantly using its texture
// usefull for streaming video
// x and y for position
// w and h for dimensions
CENGINE_EXPORT Image *ui_image_create_dynamic (u32 x, u32 y, u32 w, u32 h, Renderer *renderer);

// draws the image to the screen
CENGINE_PRIVATE void ui_image_draw (Image *image, Renderer *renderer);

#endif