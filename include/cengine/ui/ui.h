#ifndef _CENGINE_UI_H_
#define _CENGINE_UI_H_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/window.h"
#include "cengine/renderer.h"

#include "cengine/ui/components/transform.h"

struct _Window;
struct _Renderer;
struct _UI;
struct _UITransform;

typedef enum UIElementType {

    UI_NONE             = 0,

    UI_TEXTBOX          = 1,
    UI_IMAGE            = 2,
    UI_PANEL            = 3,
    UI_BUTTON           = 4,
    UI_INPUT            = 5,
    UI_CHECK            = 6,
    UI_NOTI_CENTER      = 7,
    UI_DROPDOWN         = 8

} UIElementType;

#define DEFAULT_MAX_UI_ELEMENTS         256

struct _UIElement {

    // 09/02/2020 - aux reference to the UI this element belongs to
    struct _UI *ui;

    i32 id;

    bool active;
    int layer_id;

    UIElementType type;
    void *element;
    struct _UITransform *transform;

    struct _UIElement *parent;

};

typedef struct _UIElement UIElement;

// ui element constructor
extern UIElement *ui_element_create (struct _UI *ui, UIElementType type);

// deletes the ui element directly
extern void ui_element_delete (void *ui_element_ptr);

extern void ui_element_delete_dummy (void *ui_element_ptr);

// removes the element from the ui and then deletes it
extern void ui_element_destroy (void *ui_element_ptr);

extern int ui_element_comparator (const void *one, const void *two);

// sets the render layer of the ui element
// removes it from the one it is now and adds it to the new one
// returns 0 on success, 1 on error
extern int ui_element_set_layer (struct _UI *ui, UIElement *ui_element, const char *layer_name);

extern void ui_element_toggle_active (UIElement *ui_element);

extern void ui_element_set_active (UIElement *ui_element, bool active);

extern void parent_ui_element_get_position (UIElement *parent, int *x, int *y);

struct _UI {

    // 08/02/2020 -- 21:17
    i32 new_ui_element_id;
    DoubleList *ui_elements;

    DoubleList *ui_elements_layers;

    UIElement *ui_element_hover;

};

typedef struct _UI UI;

extern void ui_delete (void *ui_ptr);

// init our ui elements structures
extern UI *ui_create (void);

extern UIElement *ui_element_hover_get (UI *ui);

// adds a new ui element back to the UI
// returns 0 on success, 1 on error
extern u8 ui_add_element (UI *ui, UIElement *ui_element);

// removes a ui element from the UI
extern UIElement *ui_remove_element (UI *ui, UIElement *ui_element);

/*** render ***/

// resize the ui elements to fit new window
extern void ui_resize (struct _Window *window);

// renders the ui element to the screen
extern void ui_render_element (struct _Renderer *renderer, UIElement *ui_element);

// renders all the current active ui to the screen
extern void ui_render (struct _Renderer *renderer);

/*** main ***/

// initializes cengine's ui capabilities
extern u8 ui_init (void);

// destroy common ui elements
extern u8 ui_end (void);

#endif