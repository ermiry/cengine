#ifndef _CENGINE_UI_H_
#define _CENGINE_UI_H_

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"

/*** Common HEX colors ***/

#define HEX_NO_COLOR        0x00000000
#define HEX_WHITE           0xFFFFFFFF
#define HEX_BLACK           0x000000FF

#define HEX_FULL_RED        0xFF0000FF
#define HEX_FULL_GREEN      0x00FF00FF
#define HEX_FULL_BLUE       0x0000FFFF

#define HEX_YELLOW          0xFFD32AFF
#define HEX_SAPPHIRE        0x1E3799FF

#define HEX_SILVER          0xBDC3C7FF

/*** Common RGBA Colors ***/

typedef SDL_Color RGBA_Color;

extern RGBA_Color RGBA_NO_COLOR;
extern RGBA_Color RGBA_WHITE;
extern RGBA_Color RGBA_BLACK;
extern RGBA_Color RGBA_RED;
extern RGBA_Color RGBA_GREEN;
extern RGBA_Color RGBA_BLUE;

/*** UI Elements ***/

typedef enum UIElementType {

    UI_TEXTBOX,
    UI_IMAGE,
    UI_PANEL,
    UI_BUTTON,
    UI_INPUT,
    UI_CHECK,
    UI_NOTI_CENTER,
    UI_DROPDOWN

} UIElementType;

#define DEFAULT_MAX_UI_ELEMENTS     10

typedef struct UIElement {

    i32 id;
    bool active;
    UIElementType type;
    int layer_id;
    void *element;

} UIElement;

// ui element constructor
extern UIElement *ui_element_new (UIElementType type);

// deactivates the ui element and destroys its component (this is what the user should call)
extern void ui_element_destroy (UIElement *ui_element);

// completely deletes the UI element (only called by dengine functions)
extern void ui_element_delete (UIElement *ui_element);

extern void ui_element_delete (UIElement *ui_element);

extern void ui_element_delete_dummy (void *ui_element_ptr);

extern int ui_element_comparator (const void *one, const void *two);

// sets the render layer of the ui element
// removes it from the one it is now and adds it to the new one
// returns 0 on success, 1 on error
extern int ui_element_set_layer (UIElement *ui_element, const char *layer_name);

extern void ui_element_toggle_active (UIElement *ui_element);

typedef SDL_Rect UIRect;

extern UIRect ui_rect_create (u32 x, u32 y, u32 w, u32 h);
extern UIRect ui_rect_union (UIRect a, UIRect b);

extern RGBA_Color ui_rgba_color_create (u8 r, u8 g, u8 b, u8 a);

/*** Public ui funcs ***/

// renders all the current active ui to the screen
extern void ui_render (void);

// initializes cengine's ui capabilities
extern u8 ui_init (void);

// sets the location of cengine's default ui assets
extern void ui_default_assets_set_path (const char *pathname);

// loads cengine's default ui assets
extern u8 ui_default_assets_load (void);

// destroys any cengine ui element left and deallocates memory
extern u8 ui_destroy (void);

#endif