#ifndef _CENGINE_UI_TEXT_BOX_H_
#define _CENGINE_UI_TEXT_BOX_H_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"

// TODO: handle input logic
typedef struct TextBox {

    Font *font;
    SDL_Texture *texture;
    UIRect bgrect;
    RGBA_Color bgcolor;

    RGBA_Color textColor;
    char *text;
    bool ispassword;
    char *pswd;

    bool isVolatile;

} TextBox;

extern TextBox *ui_textBox_create_static (u32 x, u32 y, RGBA_Color bgColor,
    const char *text, RGBA_Color textColor, Font *font, bool isPassword);
extern TextBox *ui_textBox_create_volatile (u32 x, u32 y, RGBA_Color bgColor,
    const char *text, RGBA_Color textColor, Font *font, bool isPassword);

extern void ui_textbox_draw (TextBox *textbox);

extern void ui_textBox_set_text (TextBox *textBox, const char *newText);
extern void ui_textBox_set_text_color (TextBox *textBox, RGBA_Color newColor);
extern void ui_textBox_set_bg_color (TextBox *textBox, RGBA_Color newColor);

extern void ui_textBox_destroy (TextBox *textbox);

#endif