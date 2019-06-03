#ifndef _CENGINE_UI_H_
#define _CENGINE_UI_H_

#include <SDL2/SDL.h>

#include "cengine/types/types.h"

/*** COMMON HEX COLORS ***/

#define HEX_NO_COLOR        0x00000000
#define HEX_WHITE           0xFFFFFFFF
#define HEX_BLACK           0x000000FF

#define HEX_FULL_GREEN      0x00FF00FF
#define HEX_FULL_RED        0xFF0000FF

#define HEX_YELLOW          0xFFD32AFF
#define HEX_SAPPHIRE        0x1E3799FF

#define HEX_SILVER          0xBDC3C7FF

/*** COMMON RGBA COLORS ***/

typedef SDL_Color RGBA_Color;

extern RGBA_Color RGBA_NO_COLOR;
extern RGBA_Color RGBA_WHITE;
extern RGBA_Color RGBA_BLACK;

/*** UI ELEMENTS ***/

typedef enum UIElementType {

    UI_TEXTBOX,
    UI_BUTTON,

} UIElementType;

#define DEFAULT_MAX_UI_ELEMENTS     10

typedef struct UIElement {

    i32 id;
    UIElementType type;
    void *element;

} UIElement;

// ui element constructor
extern UIElement *ui_element_new (UIElementType type);

typedef SDL_Rect UIRect;

extern UIRect ui_rect_create (u32 x, u32 y, u32 w, u32 h);
extern UIRect ui_rect_union (UIRect a, UIRect b);

extern RGBA_Color ui_rgba_color_create (u8 r, u8 g, u8 b, u8 a);

/*** Public ui funcs ***/

extern void ui_render (void);

extern u8 ui_init (void);
extern u8 ui_destroy (void);

#endif