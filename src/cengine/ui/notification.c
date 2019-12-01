#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/collections/dlist.h"

#include "cengine/renderer.h"
#include "cengine/timer.h"

#include "cengine/ui/types/types.h"
#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/notification.h"
#include "cengine/ui/components/text.h"
#include "cengine/ui/components/transform.h"

#pragma region Notification

static Notification *ui_notification_new (void) {

    Notification *noti = (Notification *) malloc (sizeof (Notification));
    if (noti) {
        memset (noti, 0, sizeof (Notification));

        noti->timestamp = NULL;
        noti->timestamp_text = NULL;

        noti->title = NULL;
        noti->msg = NULL;

        noti->transform = NULL;

        noti->life = timer_new ();
    }

    return noti;

}

void ui_notification_delete (void *noti_ptr) {

    if (noti_ptr) {
        Notification *noti = (Notification *) noti_ptr;

        ui_text_component_delete (noti->timestamp_text);

        ui_text_component_delete (noti->title);
        ui_text_component_delete (noti->msg);

        ui_transform_component_delete (noti->transform);

        timer_destroy (noti->life);

        free (noti);
    }

}

// set notification bg color based on its type
static void ui_notification_get_color (Notification *noti) {

    if (noti) {
        switch (noti->type) {
            case NOTI_TYPE_INFO: {
                RGBA_Color color = { 34, 167, 240, 1 };
                noti->bgcolor = color;
            } break;
            case NOTI_TYPE_SUCCESS: {
                RGBA_Color color = { 38, 166, 91, 1 };
                noti->bgcolor = color;
            } break;
            case NOTI_TYPE_WARNING: {
                RGBA_Color color = { 247, 202, 24, 1 };
                noti->bgcolor = color;
            } break;
            case NOTI_TYPE_ERROR: {
                RGBA_Color color = { 150, 40, 27, 1 };
                noti->bgcolor = color; 
            } break;

            case NOTI_TYPE_UNDEFINED:
            case NOTI_TYPE_CUSTOM:
            default: {
                RGBA_Color color = { 149, 165, 166, 1 };
                noti->bgcolor = color;
            } break;
        }
    }

}

// sets the timestamp to be displayed in the notification
void ui_notification_set_timestmap (Notification *noti, struct tm *timestamp) {

    if (noti) {
        if (timestamp) {
            noti->timestamp = timestamp;

            // update the notification's timestamp text
            if (noti->timestamp_text) ui_text_component_delete (noti->timestamp_text);

            noti->timestamp_text = ui_text_component_new ();
            if (noti->timestamp_text) {
                String *time_str = timer_time_to_string (noti->timestamp);
                ui_text_component_init (noti->timestamp_text,
                    ui_font_get_default (), NOTI_TIMESTAMP_DEFAULT_SIZE, RGBA_WHITE, 
                    time_str->str);
                str_delete (time_str);
            }
        }
    }

}

// sets the notification's timestamp display options
void ui_notification_set_timestamp_options (Notification *noti, 
    Font *font, u32 size, RGBA_Color color) {

    if (noti) {
        if (noti->timestamp_text) {
            noti->timestamp_text->font = font;
            noti->timestamp_text->size = size;
            noti->timestamp_text->text_color = color;
        }
    }

}

// sets the color to be used in the notification's timestamp
void ui_notification_set_timestamp_color (Notification *noti, RGBA_Color color) {

    if (noti) {
        if (noti->timestamp_text) {
            noti->timestamp_text->text_color = color;
        }
    }

}

// sets the font to be used in the notification's timestamp
void ui_notification_set_timestamp_font (Notification *noti, Font *font) {

    if (noti) {
        if (noti->timestamp_text) {
            noti->timestamp_text->font = font;
        }
    }

}

// sets the title of the notification with some options
void ui_notification_set_title (Notification *noti, const char *text, 
    Font *font, u32 size, RGBA_Color color) {

    if (noti) {
        if (noti->title) ui_text_component_delete (noti->title);

        noti->title = ui_text_component_new ();
        if (noti->title) {
            ui_text_component_init (noti->title,
                font, size, color, text);
        }
    }

}

// sets some options for the notification's title
void ui_notification_set_title_options (Notification *noti, 
    Font *font, u32 size, RGBA_Color color) {

    if (noti) {
        if (noti->title) {
            noti->title->font = font;
            noti->title->size = size;
            noti->title->text_color = color;
        }
    }

}

// sets the color of the notification's title
void ui_notification_set_title_color (Notification *noti, RGBA_Color color) {

    if (noti) {
        if (noti->title) noti->title->text_color = color;
    } 

}

// sets the font to be used in the notification's title
void ui_notification_set_title_font (Notification *noti, Font *font) {

    if (noti) {
        if (noti->title) noti->title->font = font;
    }

}

// sets the msg of the notification with some options
void ui_notification_set_msg (Notification *noti, const char *text, 
    Font *font, u32 size, RGBA_Color color) {

    if (noti) {
        if (noti->msg) ui_text_component_delete (noti->msg);

        noti->msg = ui_text_component_new ();
        if (noti->msg) {
            ui_text_component_init (noti->msg,
                font, size, color, text);
        }
    }

}

// sets some options for the notification's msg
void ui_notification_set_msg_options (Notification *noti, 
    Font *font, u32 size, RGBA_Color color) {

    if (noti) {
        if (noti->msg) {
            noti->msg->font = font;
            noti->msg->size = size;
            noti->msg->text_color = color;
        }
    }

}

// sets the color of the notification's message
void ui_notification_set_msg_color (Notification *noti, RGBA_Color color) {

    if (noti) {
        if (noti->msg) noti->msg->text_color = color;
    }

}

// sets the font to be used in the notification's msg
void ui_notification_set_msg_font (Notification *noti, Font *font) {

    if (noti) {
        if (noti->msg) noti->msg->font = font;
    }

}

// sets a custom background color for the notification
void ui_notification_set_bg_color (Notification *noti, RGBA_Color color) {

    if (noti) noti->bgcolor = color;

}

// creates a new notification
// type: default types have already created designs, select custom to pass your own
// lifetime: secs to display
// display_timestamp: option to display the timestamp of the notification
// title: the title of the notification (NULL for empty)
// msg: the message to be displayed (NULL for empty)
// uses the first set font for the texts with default values for sizes
Notification *ui_notification_create (NotificationType type, float lifetime, bool display_timestamp,
    const char *title, const char *msg) {

    Notification *noti = ui_notification_new ();
    if (noti) {
        noti->type = type;
        ui_notification_get_color (noti);
        noti->lifetime = lifetime;
        noti->transform = ui_transform_component_create (0, 0, 0, 0);

        Font *default_font = ui_font_get_default ();

        if (display_timestamp) {
            // get the current local time the notification has been created
            noti->timestamp = timer_get_local_time ();

            noti->timestamp_text = ui_text_component_new ();
            if (noti->timestamp_text) {
                String *time_str = timer_time_to_string (noti->timestamp);
                ui_text_component_init (noti->timestamp_text, default_font,
                    NOTI_TIMESTAMP_DEFAULT_SIZE, RGBA_WHITE, 
                    time_str->str);
                str_delete (time_str);
            }
        }

        if (title) {
            noti->title = ui_text_component_new ();
            if (noti->title) {
                ui_text_component_init (noti->title, default_font, 
                    NOTI_TITLE_DEFAULT_SIZE, RGBA_WHITE, title);
            } 
        } 

        if (msg) {
            noti->msg = ui_text_component_new ();
            if (noti->msg) {
                ui_text_component_init (noti->msg, default_font, 
                    NOTI_MSG_DEFAULT_SIZE, RGBA_WHITE, msg);
            }
        }
    }

    return noti;

}

// updates the position of the notification;s elements
static void ui_notification_update_pos (Notification *noti) {

    if (noti) {
        u32 y_pos = noti->transform->rect.y;

        if (noti->timestamp_text) {
            noti->timestamp_text->transform->rect.y = noti->transform->rect.y;
            y_pos += noti->timestamp_text->transform->rect.h;
        }

        if (noti->title) {
            noti->title->transform->rect.y = noti->timestamp_text ? 
                y_pos : noti->transform->rect.y;
                y_pos += noti->title->transform->rect.h;
        } 

        if (noti->msg) {
            noti->msg->transform->rect.y = y_pos;
        } 
    }

}

// prepare the notification to be displayed in an upper notification center
static void ui_notification_prepare_upper (NotiCenter *noti_center, Notification *noti, Renderer *renderer) {

    if (noti) {
        // prepare notification layout
        noti->transform->rect.w = noti_center->ui_element->transform->rect.w - 10;
        noti->transform->rect.x = noti_center->ui_element->transform->rect.x + 5;
        noti->transform->rect.y = noti_center->ui_element->transform->rect.y + 5;

        if (noti->timestamp_text) {
            ui_text_component_set_wrap (noti->timestamp_text, noti->transform->rect.w);
            ui_text_component_draw (noti->timestamp_text, renderer);

            noti->timestamp_text->transform->rect.x = noti->transform->rect.x;
            noti->timestamp_text->transform->rect.y = noti->transform->rect.y;

            noti->transform->rect.h += noti->timestamp_text->transform->rect.h;
        }

        if (noti->title) {
            ui_text_component_set_wrap (noti->title, noti->transform->rect.w);
            ui_text_component_draw (noti->title, renderer);

            noti->title->transform->rect.x = noti->transform->rect.x;
            noti->title->transform->rect.y = noti->timestamp_text ? noti->transform->rect.y + (noti->transform->rect.h) : noti->transform->rect.y;

            noti->transform->rect.h += noti->title->transform->rect.h;
        } 

        if (noti->msg) {
            ui_text_component_set_wrap (noti->msg, noti->transform->rect.w);
            ui_text_component_draw (noti->msg, renderer);

            noti->msg->transform->rect.x = noti->transform->rect.x;
            noti->msg->transform->rect.y = noti->title ? noti->transform->rect.y + (noti->transform->rect.h) : noti->transform->rect.y;

            noti->transform->rect.h += noti->msg->transform->rect.h;
        } 
    }

}

// prepare the notification to be displayed in a bottom notification center
static void ui_notification_prepare_bottom (NotiCenter *noti_center, Notification *noti, Renderer *renderer) {

    if (noti) {
        // prepare notification layout
        noti->transform->rect.w = noti_center->ui_element->transform->rect.w - 10;
        noti->transform->rect.x = noti_center->ui_element->transform->rect.x + 5;
        // noti->transform->rect.y = (noti_center->transform->rect.y + noti_center->transform->rect.h) - noti->transform->rect.h;
        noti->transform->rect.y = noti_center->ui_element->transform->rect.y;

        if (noti->timestamp_text) {
            ui_text_component_set_wrap (noti->timestamp_text, noti->transform->rect.w);
            ui_text_component_draw (noti->timestamp_text, renderer);

            noti->timestamp_text->transform->rect.x = noti->transform->rect.x;
            noti->timestamp_text->transform->rect.y = noti->transform->rect.y + 5;

            noti->transform->rect.h += noti->timestamp_text->transform->rect.h;
        }

        if (noti->title) {
            ui_text_component_set_wrap (noti->title, noti->transform->rect.w);
            ui_text_component_draw (noti->title, renderer);

            noti->title->transform->rect.x = noti->transform->rect.x;
            noti->title->transform->rect.y = noti->timestamp_text ? noti->transform->rect.y + (noti->transform->rect.h) : noti->transform->rect.y;

            noti->transform->rect.h += noti->title->transform->rect.h;
        } 

        if (noti->msg) {
            ui_text_component_set_wrap (noti->msg, noti->transform->rect.w);
            ui_text_component_draw (noti->msg, renderer);

            noti->msg->transform->rect.x = noti->transform->rect.x;
            noti->msg->transform->rect.y = noti->title ? noti->transform->rect.y + (noti->transform->rect.h) : noti->transform->rect.y;

            noti->transform->rect.h += noti->msg->transform->rect.h;
        } 
    }

}

// adds the notification to the notification center where you want to display it
// and displays the notification for x seconds (lifetime)
void ui_notification_display (NotiCenter *noti_center, Notification *notification, Renderer *renderer) {

    if (noti_center && notification) {
        // prepare the notification to be displayed in the notification center
        noti_center->bottom ? ui_notification_prepare_bottom (noti_center, notification, renderer) : ui_notification_prepare_upper (noti_center, notification, renderer);

        // add the notification to the notification center for display
        dlist_insert_after (noti_center->notifications, dlist_end (noti_center->notifications), notification);
    }

}

// creates the notification with the passed values and default options by notification type
// and then displays it in the notification center
void ui_notification_create_and_display (NotiCenter *noti_center, Renderer *renderer, NotificationType type, 
    float lifetime, bool display_timestamp,
    const char *title, const char *msg) {

    if (noti_center) {
        // create the notification
        Notification *noti = ui_notification_create (type, lifetime, display_timestamp, title, msg);
        if (noti) ui_notification_display (noti_center, noti, renderer);
    }

}

// draws the notification to the screen
static void ui_notification_draw (Notification *noti, Renderer *renderer) {

    if (noti) {
        render_basic_filled_rect (renderer, &noti->transform->rect, noti->bgcolor);

        if (noti->timestamp) ui_text_component_render (noti->timestamp_text, renderer);
        if (noti->title) ui_text_component_render (noti->title, renderer);
        if (noti->msg) ui_text_component_render (noti->msg, renderer);
    }

}

#pragma endregion

#pragma region NotiCenter

static NotiCenter *ui_noti_center_new (void) {

    NotiCenter *noti_center = (NotiCenter *) malloc (sizeof (NotiCenter));
    if (noti_center) {
        memset (noti_center, 0, sizeof (NotiCenter));
        noti_center->ui_element = NULL;
        noti_center->bg_texture = NULL;
        noti_center->colour = false;
        noti_center->outline = false;
        noti_center->notifications = dlist_init (ui_notification_delete, NULL);
        noti_center->active_notifications = dlist_init (ui_notification_delete, NULL);
    }

    return noti_center;

}

void ui_noti_center_delete (void *noti_center_ptr) {

    if (noti_center_ptr) {
        NotiCenter *noti_center = (NotiCenter *) noti_center_ptr;

        noti_center->ui_element = NULL;
        if (noti_center->bg_texture) SDL_DestroyTexture (noti_center->bg_texture);
        dlist_delete (noti_center->notifications);
        dlist_delete (noti_center->active_notifications);

        free (noti_center);
    }

}

// adjust noti center postion values
static void ui_noti_center_update_pos (NotiCenter *noti_center) {

    if (noti_center) {
        switch (noti_center->ui_element->transform->pos) {
            case UI_POS_FREE: 
            case UI_POS_MIDDLE_CENTER: 
                noti_center->bottom = false;
                break;

            case UI_POS_UPPER_CENTER: noti_center->bottom = false; break;
            case UI_POS_RIGHT_UPPER_CORNER: noti_center->bottom = false; break;
            case UI_POS_RIGHT_CENTER: noti_center->bottom = false; break;
            case UI_POS_RIGHT_BOTTOM_CORNER: noti_center->bottom = true; break;
            case UI_POS_BOTTOM_CENTER: noti_center->bottom = true; break;
            case UI_POS_LEFT_BOTTOM_CORNER: noti_center->bottom = true; break;
            case UI_POS_LEFT_CENTER: noti_center->bottom = false; break;
            case UI_POS_LEFT_UPPER_CORNER: noti_center->bottom = false; break;

            default: noti_center->bottom = false; break;
        }
    }

}

// creates a new notification center
// max_display: max number of notifications to display at once
// position: where do you want the notification center to be
NotiCenter *ui_noti_center_create (UI *ui, u8 max_display, UIPosition pos, Renderer *renderer) {

    NotiCenter *noti_center = NULL;

    UIElement *ui_element = ui_element_create (ui, UI_NOTI_CENTER);

    if (ui_element) {
        noti_center = ui_noti_center_new ();
        if (noti_center) {
            noti_center->ui_element = ui_element;
            noti_center->max_display = max_display;
            ui_transform_component_set_values (ui_element->transform, 0, 0,
                NOTI_CENTER_DEFAULT_WIDTH, NOTI_CENTER_DEFAULT_HEIGHT);
            ui_transform_component_set_pos (noti_center->ui_element->transform, renderer, NULL, pos, true);

            // adjust noti center postion values
            ui_noti_center_update_pos (noti_center);

            noti_center->offset = noti_center->bottom ? noti_center->ui_element->transform->rect.h : 0;

            ui_element->element = noti_center;

            noti_center->outline_scale_x = 1;
            noti_center->outline_scale_y = 1;
        }

        else ui_element_delete (ui_element);
    }

    return noti_center;

}

// sets the notification center position in the screen
void ui_noti_center_set_position (NotiCenter *noti_center, UIPosition pos) {

    if (noti_center) {
        noti_center->ui_element->transform->pos = pos;
        ui_noti_center_update_pos (noti_center);
    }

}

// sets the notification center dimensions
void ui_noti_center_set_dimensions (NotiCenter *noti_center, u32 width, u32 height) {

    if (noti_center) {
        noti_center->ui_element->transform->rect.w = width;
        noti_center->ui_element->transform->rect.h = height;

        ui_noti_center_update_pos (noti_center);
    }

}

// sets the noti center's outline colour
void ui_noti_center_set_ouline_colour (NotiCenter *noti_center, RGBA_Color colour) {

    if (noti_center) {
        noti_center->outline = true;
        noti_center->outline_colour = colour;
    }

}

// sets the noti center's outline scale
void ui_noti_center_set_ouline_scale (NotiCenter *noti_center, float x_scale, float y_scale) {

    if (noti_center) {
        noti_center->outline_scale_x = x_scale;
        noti_center->outline_scale_y = y_scale;
    }

}

// removes the ouline form the noti center
void ui_noti_center_remove_outline (NotiCenter *noti_center) {

    if (noti_center) {
        memset (&noti_center->outline_colour, 0, sizeof (RGBA_Color));
        noti_center->outline = false;
    }

}

// sets the notification center background color
void ui_noti_center_set_bg_color (NotiCenter *noti_center, Renderer *renderer, RGBA_Color color) {

    if (noti_center) {
        noti_center->bg_colour = color;
        if (color.a < 255) {
            render_complex_transparent_rect (renderer, &noti_center->bg_texture,
                &noti_center->ui_element->transform->rect, color);
            noti_center->bg_texture_rect.w = noti_center->ui_element->transform->rect.w;
            noti_center->bg_texture_rect.h = noti_center->ui_element->transform->rect.h;
        }

        noti_center->colour = true;
    } 

}

// removes the background from the noticenter
void ui_noti_center_remove_background (NotiCenter *noti_center) {

    if (noti_center) {
        if (noti_center->bg_texture) {
            SDL_DestroyTexture (noti_center->bg_texture);
            noti_center->bg_texture = NULL;
        }

        memset (&noti_center->bg_colour, 0, sizeof (RGBA_Color));
        noti_center->colour = false;
    }

}

// draws the notification center
void ui_noti_center_draw (NotiCenter *noti_center, Renderer *renderer) {

    if (noti_center && renderer) {
        // render the background
        if (noti_center->bg_texture) {
            SDL_RenderCopyEx (renderer->renderer, noti_center->bg_texture, 
                &noti_center->bg_texture_rect, &noti_center->ui_element->transform->rect, 
                0, 0, SDL_FLIP_NONE);
        }

        else if (noti_center->colour) 
            render_basic_filled_rect (renderer, &noti_center->ui_element->transform->rect, noti_center->bg_colour);

        // render the outline border
        if (noti_center->outline) 
            render_basic_outline_rect (renderer, &noti_center->ui_element->transform->rect, noti_center->outline_colour, 
                noti_center->outline_scale_x, noti_center->outline_scale_y);

        if (noti_center->active_notifications->size < noti_center->max_display) {
            if (noti_center->notifications->size > 0) {
                Notification *noti = NULL;
                for (ListElement *le = dlist_start (noti_center->notifications); le; le = le->next) {
                    noti = (Notification *) le->data;
                    
                    // check for available space in the notification center UI
                    u32 new_height = noti_center->ui_element->transform->rect.y + noti_center->offset;
                    bool ok = false;
                    if (noti_center->bottom) {
                        if ((new_height - noti->transform->rect.h) > (noti_center->ui_element->transform->rect.y))
                            ok = true;
                    }

                    else if ((new_height + noti->transform->rect.h) < (noti_center->ui_element->transform->rect.y + noti_center->ui_element->transform->rect.h))
                        ok = true;

                    if (ok) {
                        // adjust notification position values
                        // noti->transform->rect.y = noti_center->bottom ? (new_height - noti->transform->rect.h) : new_height;
                        // ui_notification_update_pos (noti);
                        if (noti_center->bottom) noti_center->offset -= noti->transform->rect.h;
                        else noti_center->offset += noti->transform->rect.h;

                        // push the notification to the active ones
                        void *noti_data = dlist_remove_element (noti_center->notifications, le);
                        dlist_insert_after (noti_center->active_notifications, 
                            dlist_end (noti_center->active_notifications), noti_data);
                        timer_start (((Notification *) noti_data)->life);
                        
                        if (noti_center->active_notifications->size >= noti_center->max_display) break;
                    }
                }
            }
        }

        // render & update the active notifications
        if (noti_center->active_notifications->size > 0) {
            u32 offset = noti_center->bottom ? noti_center->ui_element->transform->rect.h : 0;
            Notification *noti = NULL;
            for (ListElement *le = dlist_start (noti_center->active_notifications); le; le = le->next) {
                noti = (Notification *) le->data;

                u32 new_height = noti_center->ui_element->transform->rect.y + offset;
                noti->transform->rect.y = noti_center->bottom ? (new_height - noti->transform->rect.h) : new_height;
                ui_notification_update_pos (noti);
                if (noti_center->bottom) offset -= noti->transform->rect.h;
                else offset += noti->transform->rect.h;

                ui_notification_draw (noti, renderer);

                // check for lifetime
                if ((timer_get_ticks (noti->life) / 1000) >= noti->lifetime) {
                    // remove older notifications
                    Notification *old_noti = (Notification *) dlist_remove_element (noti_center->active_notifications, le);
                    if (noti_center->bottom) noti_center->offset += old_noti->transform->rect.h;
                    else noti_center->offset -= old_noti->transform->rect.h;
                    ui_notification_delete (old_noti);
                    if (noti_center->active_notifications->size <= 0) break;
                }   
            }       
        }
    }

}

#pragma endregion