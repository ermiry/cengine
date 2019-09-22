#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/renderer.h"
#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/utils/log.h"
#include "cengine/utils/utils.h"

#include "cengine/collections/dlist.h"

static DoubleList *fonts = NULL;

static u8 has_render_target_support = 0;

/*** Misc ***/

#pragma region Misc

static int u8_charsize (const char *character) {

    if (!character) return 0;

    if ((unsigned char) *character <= 0x7F) return 1;
    else if ((unsigned char) *character < 0xE0) return 2;
    else if ((unsigned char) *character < 0xF0) return 3;
    else return 4;

    return 1;
}

static inline const char *u8_next (const char *string) {

    return string + u8_charsize (string);

}

static int u8_charcpy (char *buffer, const char *source, int buffer_size) {

    if(buffer == NULL || source == NULL || buffer_size < 1)
        return 0;

    int charsize = u8_charsize (source);
    if (charsize > buffer_size)
        return 0;

    memcpy (buffer, source, charsize);

    return charsize;

}

u32 get_code_point_from_UTF8 (const char **c, u8 advancePtr) {

    if (c && *c) {
        u32 retval = 0;
        const char *str = *c;

        if((unsigned char)*str <= 0x7F) retval = *str;
        else if((unsigned char)*str < 0xE0) {
            retval |= (unsigned char)(*str) << 8;
            retval |= (unsigned char)(*(str + 1));
            if (advancePtr) *c += 1;
        }

        else if((unsigned char)*str < 0xF0) {
            retval |= (unsigned char)(*str) << 16;
            retval |= (unsigned char)(*(str + 1)) << 8;
            retval |= (unsigned char)(*(str + 2));
            if (advancePtr) *c += 2;
        }
        else {
            retval |= (unsigned char)(*str) << 24;
            retval |= (unsigned char)(*(str + 1)) << 16;
            retval |= (unsigned char)(*(str + 2)) << 8;
            retval |= (unsigned char)(*(str + 3));
            if (advancePtr) *c += 3;
        }

        return retval;
    }

    return 0;

}

static char *font_get_string_ASCII (void) {

    static char *buffer = NULL;
    if (!buffer) {
        int i;
        char c;
        buffer = (char *) calloc (512, sizeof (char));
        i = 0;
        c = 32;
        while (1) {
            buffer[i] = c;
            if (c == 126)
                break;
            ++i;
            ++c;
        }
    }

    char *retval = (char*) calloc (strlen (buffer) + 1, sizeof (char));
    if (retval) strcpy (retval, buffer);

    return retval;

}

static inline SDL_Surface *font_create_surface (u32 width, u32 height) {

    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        return SDL_CreateRGBSurface (SDL_SWSURFACE, width, height, 32, 
            0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    #else
        return SDL_CreateRGBSurface (SDL_SWSURFACE, width, height, 32, 
            0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    #endif

}

#pragma endregion

/*** Font Source ***/

static FontSource *font_source_new (void) {

    FontSource *font_source = (FontSource *) malloc (sizeof (FontSource));
    if (font_source) {
        memset (font_source, 0, sizeof (font_source));

        font_source->ttf_source = NULL;
    }

    return font_source;

}

static void font_source_delete (void *font_source_ptr) {

    if (font_source_ptr) {
        FontSource *source = (FontSource *) font_source_ptr;
        // if (source->owns_ttf_source) TTF_CloseFont (source->ttf_source);
        free (source);
    }

}

FontSource *font_source_get_by_size (Font *font, unsigned int size) {

    FontSource *source = NULL;
    
    if (font) {
        int small_diff = 1000;
        int small_diff_idx;

        for (unsigned int i = 0; i < font->n_sizes; i++) {
            if (font->sources[i]->size == size) {
                source = font->sources[i];
                break;
            }

            else {
                int diff = size - font->sources[i]->size;
                if (diff < small_diff) {
                    small_diff = diff;
                    small_diff_idx = i;
                }
            }
        }

        if (!source) source = font->sources[small_diff_idx];
    }

    return source;

}

/*** Font ***/

#pragma region Font

static Font *ui_font_new (void) {

    Font *font = (Font *) malloc (sizeof (Font));
    if (font) {
        memset (font, 0, sizeof (Font));

        font->filename = font->name = NULL;

        font->sizes = NULL;
        font->sources = NULL;

        font->filter = FILTER_NEAREST;
    }

    return font;

}

// creates a new font structure that requires a font to be loaded
Font *ui_font_create (const char *font_name, const char *font_filename) {

    Font *font = ui_font_new ();
    if (font) {
        font->name = str_new (font_name);
        font->filename = str_new (font_filename);

        dlist_insert_after (fonts, dlist_end (fonts), font);
    } 

    return font;

}

void ui_font_delete (void *font_ptr) {

    if (font_ptr) {
        Font *font = (Font *) font_ptr;
        str_delete (font->name);
        str_delete (font->filename);

        if (font->sizes) free (font->sizes);
        if (font->sources) {
            for (unsigned int i = 0; i < font->n_sizes; i++)
                font_source_delete (font->sources[i]);

            free (font->sources);
        }

        free (font);
    }

}

// sets the font sizes to be loaded
// returns 0 on success, 1 on error
u8 ui_font_set_sizes (Font *font, u8 n_sizes, ...) {

    u8 retval = 1;

    if (font) {
        va_list valist;
        va_start (valist, n_sizes);

        font->n_sizes = n_sizes;

        font->sizes = (int *) calloc (n_sizes, sizeof (int));
        if (font->sizes) {
            for (u8 i = 0; i < n_sizes; i++)
                font->sizes[i] = va_arg (valist, int);

            retval = 0;
        }

        va_end (valist);
    }

    return retval;

}

static void ui_font_source_load_from_ttf (FontSource *source, TTF_Font *ttf) {

    SDL_RendererInfo info;
    SDL_GetRendererInfo (main_renderer->renderer, &info);
    has_render_target_support = (info.flags & SDL_RENDERER_TARGETTEXTURE);

    source->ttf_source = ttf;

    source->height = TTF_FontHeight (ttf);
    source->ascent = TTF_FontAscent (ttf);
    source->descent = -TTF_FontDescent (ttf);
    
    // Some bug for certain fonts can result in an incorrect height.
    if (source->height < source->ascent - source->descent)
        source->height = source->ascent - source->descent;

    source->baseline = source->height - source->descent;

}

static FontSource *ui_font_load_source (Font *font, SDL_RWops *file_rwops_ttf,
    u8 own_rwops, u32 pointSize, int style) {

    FontSource *source = font_source_new ();
    if (source) {
        TTF_Font *ttf = TTF_OpenFontRW (file_rwops_ttf, own_rwops, pointSize);
        if (ttf) {
            source->size = pointSize;

            u8 outline = (style & TTF_STYLE_OUTLINE);
            if (outline) {
                style &= ~TTF_STYLE_OUTLINE;
                TTF_SetFontOutline (ttf, 1);
            }

            TTF_SetFontStyle (ttf, style);

            source->owns_ttf_source = own_rwops;
            if (!own_rwops) {
                TTF_CloseFont (source->ttf_source);
                source->ttf_source = NULL;
            }
            
            ui_font_source_load_from_ttf (source, ttf);
        }

        else {
            cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, 
                c_string_create ("Failed to load font %s ttf!",
                font->name->str));
        }
    }

    return source;

}

u8 ui_font_load (Font *font, int style) {

    u8 errors = 0;

    if (font) {
        SDL_RWops *rwops = SDL_RWFromFile (font->filename->str, "rb");
        if (rwops) {
            font->sources = (FontSource **) calloc (font->n_sizes, sizeof (FontSource *));

            // load the font for each set size
            for (unsigned int i = 0; i < font->n_sizes; i++) {
                font->sources[i] = ui_font_load_source (font, rwops, 1, font->sizes[i], style);
                if (!font->sources[i]) {
                    cengine_log_msg (stderr, LOG_WARNING, LOG_NO_TYPE,
                        c_string_create ("Failed to load size: %d for font: %s",
                        font->sizes[i], font->name->str));
                    errors = 1;
                }
            }
        }
        
        else {
            cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, 
                c_string_create ("Failed to open font file: %s", font->filename));
        } 
    }

    return errors;

}

// get a refrence to the default font --> the first one that was added
Font *ui_font_get_default (void) {

    void *font_data = (dlist_start (fonts))->data;
    if (font_data) return (Font *) font_data;
    else return NULL;

}

// gets a refrence to a ui font by its name -> it should be ready to use
Font *ui_font_get_by_name (const char *name) {

    Font *retval = NULL;

    if (name) {
        Font *font = NULL;
        for (ListElement *le = dlist_start (fonts); le; le = le->next) {
            font = (Font *) le->data;
            if (!strcmp (font->name->str, name)) {
                retval = font;
                break;
            }
        }
    }

    return retval;

}

#pragma endregion

#pragma region Font Main

u8 ui_fonts_init (void) {

    int errors = 0;

    errors = TTF_Init ();
    errors = (fonts = dlist_init (ui_font_delete, NULL)) ? 0 : 1;

    return errors;

}

void ui_font_end (void) {

    dlist_delete (fonts);
    TTF_Quit ();

}

#pragma endregion