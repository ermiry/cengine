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

typedef struct GlyphData {

    UIRect rect;
    int cacheLevel;

} GlyphData;

typedef struct FontMapNode {

    u32 key;
    GlyphData value;
    struct FontMapNode *next;

} FontMapNode;

#define DEFAULT_FONT_MAP_N_BUCKETS      300

typedef struct FontMap {

    i32 n_buckets;
    FontMapNode **buckets;

} FontMap;

typedef struct Font {

    String *name;
    String *filename;

    TTF_Font *ttf_source;
    u8 owns_ttf_source;

    FilterEnum filter;

    RGBA_Color default_color;
    u16 height;
    u16 maxWidth;
    u16 baseline;
    i32 ascent;
    i32 descent;

    i32 lineSpacing;
    i32 letterSpacing;

    FontMap *glyphs;
    GlyphData last_glyph;
    i32 glyph_cache_size;
    i32 glyph_cache_count;
    FontImage **glyph_cache;
    char *loading_string;

} Font;

// inits cengine ui font capabilities
extern u8 ui_fonts_init (void);
// destroys any cengine font data left
extern void ui_font_end (void);

// creates a new font data structure
extern Font *ui_font_new (const char *font_name, const char *font_filename);
// loads the font with the specified values
extern u8 ui_font_load (Font *font, u32 pointSize, RGBA_Color color, int style);
// destroys a font data structure
extern void ui_font_delete (void *font_ptr);

// gets a refrence to a ui font by its name -> it should be ready to use
extern Font *ui_font_get_by_name (const char *name);

/*** Glyph ***/

extern u8 glyph_get_data (Font *font, GlyphData *result, u32 codepoint);
extern SDL_Texture *glyph_get_cache_level (Font *font, int cacheLevel);

/*** Misc ***/

extern u32 get_code_point_from_UTF8 (const char **c, u8 advancePtr);

#endif