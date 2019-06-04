#include <stdlib.h>
#include <string.h>

#include "cengine/types/types.h"
#include "cengine/renderer.h"
#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/textbox.h"

void ui_textBox_set_text (TextBox *textBox, const char *newText) {

    if (textBox) {
        if (textBox->text) free (textBox->text);
        if (textBox->pswd) free (textBox->pswd);

        if (newText) {
            if (textBox->ispassword) {
                textBox->pswd = (char *) calloc (strlen (newText) + 1, sizeof (char));
                strcpy (textBox->pswd, newText);
                u32 len = strlen (newText);
                for (u8 i = 0; i < len; i++) textBox->text[i] = '*';
            }

            else {
                textBox->text = (char *) calloc (strlen (newText) + 1, sizeof (char));
                strcpy (textBox->text, newText);
                textBox->pswd = NULL;
            }
        }

        else textBox->text = textBox->pswd = NULL;

        if (!textBox->isVolatile) {
            if (textBox->texture) SDL_DestroyTexture (textBox->texture);
            SDL_Surface *surface = TTF_RenderText_Solid (textBox->font->ttf_source, newText, textBox->textColor);
            textBox->texture = SDL_CreateTextureFromSurface (main_renderer->renderer, surface);
       
            textBox->bgrect.w = surface->w;
            textBox->bgrect.h = surface->h;

            SDL_FreeSurface(surface);
        }

    }

}

void ui_textBox_set_text_color (TextBox *textBox, RGBA_Color newColor) {

    if (textBox) {
        textBox->textColor = newColor;

        if (!textBox->isVolatile) {
            SDL_Surface *surface = TTF_RenderText_Solid (textBox->font->ttf_source, 
                textBox->text, textBox->textColor);
            textBox->texture = SDL_CreateTextureFromSurface (main_renderer->renderer, surface);
    
            textBox->bgrect.w = surface->w;
            textBox->bgrect.h = surface->h;

            SDL_FreeSurface(surface);
        }
    }

}

void ui_textBox_set_bg_color (TextBox *textBox, RGBA_Color newColor) {

    if (textBox) textBox->bgcolor = newColor;

}

static TextBox *ui_textBox_new (u32 x, u32 y, RGBA_Color bgColor,
    const char *text, RGBA_Color textColor, Font *font, bool isPassword) {

    UIElement *ui_element = ui_element_new (UI_TEXTBOX);
    if (ui_element) {
        TextBox *textBox = (TextBox *) malloc (sizeof (TextBox));
        if (textBox) {
            textBox->texture = NULL;
            textBox->font = font;

            textBox->bgrect.x = x;
            textBox->bgrect.y = y;
            textBox->bgrect.w = textBox->bgrect.h = 0;
            textBox->bgcolor = bgColor;

            textBox->textColor = textColor;

            textBox->ispassword = isPassword;

            if (text) {
                if (isPassword) {
                    textBox->pswd = (char *) calloc (strlen (text) + 1, sizeof (char));
                    strcpy (textBox->pswd, text);
                    u32 len = strlen (text);
                    for (u8 i = 0; i < len; i++) textBox->text[i] = '*';
                }

                else {
                    textBox->text = (char *) calloc (strlen (text) + 1, sizeof (char));
                    strcpy (textBox->text, text);
                    textBox->pswd = NULL;
                }
            }

            else textBox->text = textBox->pswd = NULL;

            ui_element->element = textBox;

            return textBox;
        }
    }

    return NULL;

}

// TODO: generate a smoother text like with the volatile ones
// TODO: handle password logic
TextBox *ui_textBox_create_static (u32 x, u32 y, RGBA_Color bgColor,
    const char *text, RGBA_Color textColor, Font *font, bool isPassword) {

    TextBox *textBox = ui_textBox_new (x, y, bgColor, text, textColor, font, isPassword);
    if (textBox) {
        SDL_Surface *surface = TTF_RenderText_Solid (textBox->font->ttf_source, text, textColor);
        textBox->texture = SDL_CreateTextureFromSurface (main_renderer->renderer, surface);

        textBox->bgrect.x = x;
        textBox->bgrect.y = y;        
        textBox->bgrect.w = surface->w;
        textBox->bgrect.h = surface->h;

        SDL_FreeSurface(surface);

        textBox->isVolatile = false;
    }

    return textBox;

}

TextBox *ui_textBox_create_volatile (u32 x, u32 y, RGBA_Color bgColor,
    const char *text, RGBA_Color textColor, Font *font, bool isPassword) {

    TextBox *textBox = ui_textBox_new (x, y, bgColor, text, textColor, font, isPassword);
    if (textBox) {
        textBox->texture = NULL;
        textBox->isVolatile = true;
    }

    return textBox;

}

void ui_textBox_destroy (TextBox *textbox) {

    if (textbox) {
        textbox->font = NULL;
        if (textbox->texture) SDL_DestroyTexture (textbox->texture);
        if (textbox->text) free (textbox->text);
        if (textbox->pswd) free (textbox->pswd);

        free (textbox);
    }

}

// FIXME: move this form here!
// FC_Default_RenderCallback
UIRect ui_rect_render (SDL_Texture *srcTexture, UIRect *srcRect, u32 x, u32 y) {

    UIRect retval;

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    UIRect r = *srcRect;
    UIRect dr = { x, y, r.w, r.h };
    SDL_RenderCopyEx (main_renderer->renderer, srcTexture, &r, &dr, 0, NULL, flip);

    retval.x = x;
    retval.y = y;
    retval.w = srcRect->w;
    retval.h = srcRect->h;

    return retval;

}

// FIXME: handle text color change
// FIXME: handle password logic
// TODO: maybe add scale
// TODO: we need a better way for selecting font sizes!!
// this was FC_RenderLeft...
void ui_textbox_draw (TextBox *textbox) {

    if (textbox) {
        const char *c = textbox->text;

        // TODO: do we want this inside the textbox?
        UIRect srcRect, destRect, 
            dirtyRect = ui_rect_create (textbox->bgrect.x, textbox->bgrect.y, 0, 0);

        GlyphData glyph;
        u32 codepoint;

        u32 destX = textbox->bgrect.x;
        u32 destY = textbox->bgrect.y;
        float destH, destLineSpacing, destLetterSpacing;

        // TODO: add scale here!
        destH = textbox->font->height;
        destLineSpacing = textbox->font->lineSpacing;
        destLetterSpacing = textbox->font->letterSpacing;

        int newLineX = textbox->bgrect.x;

        for (; *c != '\0'; c++) {
            if (*c == '\n') {
                destX = newLineX;
                destY += destH + destLineSpacing;
                continue;
            }

            codepoint = get_code_point_from_UTF8 (&c, 1);
            if (glyph_get_data (textbox->font, &glyph, codepoint)) {
                // FIXME: handle bas caharcters
            }

            if (codepoint == ' ') {
                destX += glyph.rect.w + destLetterSpacing;
                continue;
            }

            srcRect = glyph.rect;

            destRect = ui_rect_render (glyph_get_cache_level (textbox->font, glyph.cacheLevel),
                &srcRect, destX, destY);

            if (dirtyRect.w == 0 || dirtyRect.h == 0) dirtyRect = destRect;
            else dirtyRect = ui_rect_union (dirtyRect, destRect);

            destX += glyph.rect.w + destLetterSpacing;
        }

        // FIXME:
        // return dirtyRect;
    }

}