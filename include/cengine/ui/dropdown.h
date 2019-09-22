#ifndef _CENGINE_UI_DROPDOWN_H_
#define _CENGINE_UI_DROPDOWN_H_

#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/collections/dlist.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/panel.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/components/text.h"
#include "cengine/ui/components/layout.h"

typedef struct DropdownOption {

    UITransform *transform;

    Text *option;

    bool pressed;

    // background
    bool colour;
    RGBA_Color bg_colour;
    SDL_Texture *bg_texture;
    SDL_Rect bg_texture_rect;

    // outline
    bool outline;
    RGBA_Color outline_colour;
    bool draw_selected;
    RGBA_Color selected_color;

} DropdownOption;

extern void ui_dropdown_option_delete (void *dropdown_option_ptr);

// dropdown_option_comparator by option's name
extern int ui_dropdown_option_comparator (const void *one, const void *two);

// creates a new dropdown option to be added to a dropdown
// options to pass text modifiers
extern DropdownOption *ui_dropdown_option_create (const char *option_text,
    Font *font, u32 size, RGBA_Color color);

// sets the option's text for the dropdown option element
extern void ui_dropdown_option_set_text (DropdownOption *option, const char *option_text,
    Font *font, u32 size, RGBA_Color color);

// sets the option's outline colour
extern void ui_dropdown_option_set_ouline_colour (DropdownOption *option, RGBA_Color colour);

// removes the ouline form the option
extern void ui_dropdown_option_remove_outline (DropdownOption *option);

// sets the option's background color
extern void ui_dropdown_option_set_bg_color (DropdownOption *option, RGBA_Color color);

// removes the background from the option
extern void ui_dropdown_option_remove_background (DropdownOption *option);

typedef struct Dropdown {

    UIElement *ui_element;
    UITransform *transform;

    bool active;
    bool pressed;

    // background
    bool colour;
    RGBA_Color bg_colour;
    SDL_Texture *bg_texture;
    SDL_Rect bg_texture_rect;

    // outline
    bool outline;
    RGBA_Color outline_colour;
    bool draw_selected;
    RGBA_Color selected_color;

    // placeholder text
    Text *placeholder;

    // extended
    bool extended;
    Panel *extended_panel;

    // options
    RGBA_Color option_hover_colour;
    SDL_Texture *option_hover_texture;
    DropdownOption *option_selected;
    DoubleList *options;
    LayoutGroup *layout;

} Dropdown;

extern void ui_dropdown_delete (void *dropdown_ptr);

// creates a new dropdown menu
extern Dropdown *ui_dropdown_create (i32 x, i32 y, u32 w, u32 h, UIPosition pos);

// sets the main options for the dropdown (this is required to correctly create the dropdown)
// x, y: set the position of the dropdown's extended panel
// with, height: set the dimensions of the dropdown's extended panel
extern void ui_dropdown_set_options (Dropdown *dropdown, i32 x, i32 y,
    u32 options_width, u32 options_max_height, UIPosition pos);

// sets the dropdown to be active depending on values
extern void ui_dropdown_set_active (Dropdown *dropdown, bool active);

// toggles the dropdown field to be active or not
extern void ui_dropdown_toggle_active (Dropdown *dropdown);

// sets the dropdown's outline colour
extern void ui_dropdown_set_ouline_colour (Dropdown *dropdown, RGBA_Color colour);

// removes the ouline form the dropdown
extern void ui_dropdown_remove_outline (Dropdown *dropdown);

// sets the dropdown's background color
extern void ui_dropdown_set_bg_color (Dropdown *dropdown, RGBA_Color color);

// removes the background from the dropdown
extern void ui_dropdown_remove_background (Dropdown *dropdown);

// sets the dropdown's placeholder text
extern void ui_dropdown_set_placeholder (Dropdown *dropdown,
    const char *text, Font *font, u32 size, RGBA_Color colour);

// sets the dropdown's placeholder position
extern void ui_dropdown_set_placeholder_pos (Dropdown *dropdown, UIPosition pos);

// sets the dropdown's extened panel colour
extern void ui_dropdown_extened_set_bg_colour (Dropdown *dropdown, RGBA_Color colour);

// adds a new dropdown option to the dropdown
extern void ui_dropdown_option_add (Dropdown *dropdown, DropdownOption *option);

// removes a dropdown option from the dropdown
extern void ui_dropdown_option_remove (Dropdown *dropdown, DropdownOption *option);

// gets the value of the selected option in the dropdown
extern String *ui_dropdown_option_selected (Dropdown *dropdown);

// sets the hovering colour for the dropdown options
// IMPORTANT: set the colour after you have added all the options to the dropdwon!
extern void ui_dropdown_option_set_hover_color (Dropdown *dropdown, RGBA_Color color);

// render the dropdown to the screen
extern void ui_dropdown_render (Dropdown *dropdown);

#endif