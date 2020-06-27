#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL_rect.h>

#include "cengine/collections/dlist.h"

#include "cengine/renderer.h"
#include "cengine/textures.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/components/transform.h"
#include "cengine/ui/tooltip.h"

// used for children
// #include "cengine/ui/inputfield.h"
#include "cengine/ui/button.h"
// #include "cengine/ui/image.h"
#include "cengine/ui/textbox.h"

#include "cengine/ui/layout/layout.h"
#include "cengine/ui/layout/vertical.h"

static Tooltip *ui_tooltip_new (void) {

	Tooltip *tooltip = (Tooltip *) malloc (sizeof (Tooltip));
	if (tooltip) {
		tooltip->renderer = NULL;

		tooltip->ui_element = NULL;

		tooltip->colour = false;
		memset (&tooltip->bg_colour, 0, sizeof (RGBA_Color));
		tooltip->bg_texture = NULL;
		memset (&tooltip->bg_texture_rect, 0, sizeof (SDL_Rect));

		tooltip->vertical = NULL;

		tooltip->children = NULL;
	}

	return tooltip;

}

void ui_tooltip_delete (void *tooltip_ptr) {

	if (tooltip_ptr) {
		Tooltip *tooltip = (Tooltip *) tooltip_ptr;

		tooltip->ui_element = NULL;

		texture_destroy (tooltip->renderer, tooltip->bg_texture);

		ui_layout_vertical_delete (tooltip->vertical);

		dlist_delete (tooltip->children);

        free (tooltip);
	}

}

// sets the background colour of the tooltip
void ui_tooltip_set_bg_colour (Tooltip *tooltip, Renderer *renderer, RGBA_Color colour) {

    if (tooltip) {
        tooltip->colour = true;
        tooltip->bg_colour = colour;
        if (colour.a < 255) {
            render_complex_transparent_rect (renderer, &tooltip->bg_texture, &tooltip->ui_element->transform->rect, colour);
            tooltip->bg_texture_rect.w = tooltip->ui_element->transform->rect.w;
            tooltip->bg_texture_rect.h = tooltip->ui_element->transform->rect.h;
        }
    } 

}

// removes the background from the tooltip
void ui_tooltip_remove_background (Tooltip *tooltip) {

    if (tooltip) {
        if (tooltip->bg_texture) {
            texture_destroy (tooltip->renderer, tooltip->bg_texture);
            tooltip->bg_texture = NULL;
        }

        memset (&tooltip->bg_colour, 0, sizeof (RGBA_Color));
        tooltip->colour = false;
    }

}

// 15/05/2020 -- we want only buttons and textboxs in the tooltip
// updates one tooltip's child position
static void ui_tooltip_child_update_pos (Tooltip *tooltip, UIElement *child) {

    if (tooltip && child) {
        UITransform *trans = ui_transform_component_create (
            0, 0, 
            tooltip->ui_element->transform->rect.w, tooltip->ui_element->transform->rect.h
        );

        if (trans) {
            // update the element's transform values
            switch (child->type) {
                case UI_BUTTON: {
                    ui_button_set_pos ((Button *) child->element,
                        // &tooltip->ui_element->transform->rect,
                        &trans->rect,
                        child->transform->pos,
                        NULL);
                } break;

                case UI_TEXTBOX: {
                    TextBox *textbox = (TextBox *) child->element;
                    ui_textbox_set_pos (textbox,
                        // &tooltip->ui_element->transform->rect, 
                        &trans->rect,
                        child->transform->pos, 
                        NULL);
                } break;

                default: break;
            }

            ui_transform_component_delete (trans);
        }
    }

}

// updates the tooltip's children positions
void ui_tooltip_children_update_pos (Tooltip *tooltip) {

    if (tooltip) {
        for (ListElement *le = dlist_start (tooltip->children); le; le = le->next) {
            UIElement *child = (UIElement *) le->data;

            ui_tooltip_child_update_pos (tooltip, child);
        }
    }

}

// adds a new child to the tooltip
// the element's position will be managed by the tooltip
// when the tooltip gets destroyed, all of its children get destroyed as well 
static void ui_tooltip_child_add (Tooltip *tooltip, UIElement *ui_element) {

    if (tooltip && ui_element) {
        // remove the element from the ui elements
        ui_remove_element (tooltip->renderer->ui, ui_element);

        // remove element from its layers to let the tooltip manage how the element gets renderer
        Layer *layer = layer_get_by_pos (tooltip->renderer->ui->ui_elements_layers, 
            ui_element->layer_id);
        layer_remove_element (layer, ui_element);

        dlist_insert_after (tooltip->children, dlist_end (tooltip->children), ui_element);

        ui_tooltip_child_update_pos (tooltip, ui_element);

        ui_element->abs_offset_x = tooltip->ui_element->transform->rect.x;
        ui_element->abs_offset_y = tooltip->ui_element->transform->rect.y;

        ui_element->parent = tooltip->ui_element;
    }

}

// removes a child from the tooltip (the dlist uses a ui element ids comparator)
// returns the ui element that was removed
static UIElement *ui_tooltip_child_remove (Tooltip *tooltip, UIElement *ui_element) {

    UIElement *retval = NULL;

    if (tooltip && ui_element) {
        retval = (UIElement *) dlist_remove (tooltip->children, ui_element, NULL);

        // add the element back to its original layer
        layer_add_element (layer_get_by_pos (tooltip->renderer->ui->ui_elements_layers,
            ui_element->layer_id), ui_element);

        // add the element back to the UI
        ui_add_element (tooltip->renderer->ui, ui_element);

        ui_element->parent = NULL;
    }

    return retval;

}

// adds a new ui element to the tooltip's layout's in the specified position (0 indexed)
void ui_tooltip_add_element_at_pos (Tooltip *tooltip, UIElement *ui_element, u32 pos) {

	if (tooltip && ui_element) {
		if (tooltip->vertical) {
			// add the element to the tooltip's children
            ui_tooltip_child_add (tooltip, ui_element);

			ui_layout_vertical_add_at_pos (tooltip->vertical, ui_element, pos);
		}
	}

}

// adds a new ui element to the tooltip's layout's END
void ui_tooltip_add_element_at_end (Tooltip *tooltip, UIElement *ui_element) {

	if (tooltip && ui_element) {
		if (tooltip->vertical) {
			// add the element to the tooltip's children
            ui_tooltip_child_add (tooltip, ui_element);

			ui_layout_vertical_add_at_end (tooltip->vertical, ui_element);
		}
	}

}

// returns the ui element that is at the required position in the tooltip's layout
UIElement *ui_tooltip_get_element_at (Tooltip *tooltip, unsigned int pos) {

	UIElement *retval = NULL;

	if (tooltip) {
		if (tooltip->vertical) {
			retval = ui_layout_vertical_get_element_at (tooltip->vertical, pos);
		}
	}

	return retval;

}

// removes a ui element form the tooltip's layout
u8 ui_tooltip_remove_element (Tooltip *tooltip, UIElement *ui_element) {

	u8 retval = 1;

	if (tooltip && ui_element) {
		if (tooltip->vertical) {
			retval = ui_layout_vertical_remove (tooltip->vertical, ui_element);

			// remove the element from the tooltip's children
            ui_tooltip_child_remove (tooltip, ui_element);
		}
	}

	return retval;

}

Tooltip *ui_tooltip_create (u32 w, u32 h, Renderer *renderer) {

	Tooltip *tooltip = NULL;

	UIElement *ui_element = ui_element_create (renderer->ui, UI_TOOLTIP);
	if (ui_element) {
		tooltip = ui_tooltip_new ();
		if (tooltip) {
			tooltip->renderer = renderer;

			tooltip->ui_element	= ui_element;
			ui_transform_component_set_values (
				tooltip->ui_element->transform, 
				0, 0,
				w, h
			);

			ui_element->element = tooltip;

			tooltip->vertical = ui_layout_vertical_create (
				// tooltip->ui_element->transform->rect.x, tooltip->ui_element->transform->rect.y, 
				// tooltip->ui_element->transform->rect.w, tooltip->ui_element->transform->rect.h,
				0, 0,
				w, h,
				renderer
			);

			tooltip->children = dlist_init (ui_element_delete, ui_element_comparator);
		}
	}

	return tooltip;

}

// draws the tooltip to the screen
void ui_tooltip_draw (Tooltip *tooltip, Renderer *renderer) {

	if (tooltip && renderer) {
		if (SDL_HasIntersection (&tooltip->ui_element->transform->rect, &renderer->window->screen_rect)) {
            // render the background
            if (tooltip->colour) {
                if (tooltip->bg_texture) {
                    SDL_RenderCopyEx (renderer->renderer, tooltip->bg_texture, 
                        &tooltip->bg_texture_rect, &tooltip->ui_element->transform->rect, 
                        0, 0, SDL_FLIP_NONE);
                }

                else {
                    render_basic_filled_rect (renderer, &tooltip->ui_element->transform->rect, tooltip->bg_colour);
                }
            }

            // render the tooltip's children
            if (tooltip->children) {
                if (tooltip->vertical) SDL_RenderSetViewport (renderer->renderer, &tooltip->ui_element->transform->rect);
                for (ListElement *le = dlist_start (tooltip->children); le; le = le->next)
                    ui_render_element (renderer, (UIElement *) le->data);
                
                if (tooltip->vertical) renderer_set_viewport_to_window_size (renderer);
            }

            // render the outline
            // if (tooltip->outline) {
            //     render_basic_outline_rect (renderer, &tooltip->ui_element->transform->rect, tooltip->outline_colour,
            //         tooltip->outline_scale_x, tooltip->outline_scale_y);
            // }

            renderer->render_count += 1;
        }
	}

}