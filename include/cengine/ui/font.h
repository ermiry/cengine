#ifndef _CENGINE_UI_FONT_H_
#define _CENGINE_UI_FONT_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/ui/ui.h"

typedef SDL_Texture FontImage;

#define DEFAULT_FONT_SIZE           50

#define TTF_STYLE_OUTLINE	        16
#define FONT_LOAD_MAX_SURFACES      10

typedef enum FilterEnum {

    FILTER_NEAREST,
    FILTER_LINEAR,

} FilterEnum;

typedef struct FontSource {

    TTF_Font *ttf_source;
    u8 owns_ttf_source;

    u16 height;
    u16 maxWidth;
    u16 baseline;
    i32 ascent;
    i32 descent;

    unsigned int size;

} FontSource;

typedef struct Font {

    String *name;
    String *filename;

    unsigned int n_sizes;
    unsigned int *sizes;

    FontSource **sources;

    FilterEnum filter;

} Font;

extern FontSource *font_source_get_by_size (Font *font, unsigned int size);

// inits cengine ui font capabilities
extern u8 ui_fonts_init (void);

// destroys any cengine font data left
extern void ui_font_end (void);

// creates a new font structure that requires a font to be loaded
extern Font *ui_font_create (const char *font_name, const char *font_filename);

// sets the font sizes to be loaded
// returns 0 on success, 1 on error
extern u8 ui_font_set_sizes (Font *font, u8 n_sizes, ...);

// loads the font with the specified values
extern u8 ui_font_load (Font *font, int style);

// destroys a font data structure
extern void ui_font_delete (void *font_ptr);

// get a refrence to the default font --> the first one that was added
extern Font *ui_font_get_default (void);

// gets a refrence to a ui font by its name -> it should be ready to use
extern Font *ui_font_get_by_name (const char *name);

/*** Misc ***/

extern u32 get_code_point_from_UTF8 (const char **c, u8 advancePtr);

#endif