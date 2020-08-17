#ifndef _CENGINE_UI_NOTIFICATION_H_
#define _CENGINE_UI_NOTIFICATION_H_

#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include "cengine/types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/config.h"
#include "cengine/graphics.h"
#include "cengine/timer.h"

#include "cengine/ui/ui.h"
#include "cengine/ui/font.h"
#include "cengine/ui/components/text.h"
#include "cengine/ui/components/transform.h"

struct _NotiCenter;

/*** Notification ***/

typedef enum NotificationType {

    NOTI_TYPE_UNDEFINED = 0,

    NOTI_TYPE_INFO,
    NOTI_TYPE_SUCCESS,
    NOTI_TYPE_WARNING,
    NOTI_TYPE_ERROR,

    NOTI_TYPE_CUSTOM,

} NotificationType;

#define NOTI_TIMESTAMP_DEFAULT_SIZE         20
#define NOTI_TITLE_DEFAULT_SIZE             32
#define NOTI_MSG_DEFAULT_SIZE               20

typedef struct Notification {

    NotificationType type;
    float lifetime;

    struct tm *timestamp;
    Text *timestamp_text;
    
    Text *title;
    Text *msg;

    // background
    UITransform *transform;
    RGBA_Color bgcolor;

    Timer *life;

} Notification;

CENGINE_PUBLIC void ui_notification_delete (void *noti_ptr);

// sets the timestamp to be displayed in the notification
CENGINE_EXPORT void ui_notification_set_timestmap (Notification *noti, struct tm *timestamp);

// sets the notification's timestamp display options
CENGINE_EXPORT void ui_notification_set_timestamp_options (Notification *noti, 
    Font *font, u32 size, RGBA_Color color);

// sets the color to be used in the notification's timestamp
CENGINE_EXPORT void ui_notification_set_timestamp_color (Notification *noti, RGBA_Color color);

// sets the font to be used in the notification's timestamp
CENGINE_EXPORT void ui_notification_set_timestamp_font (Notification *noti, Font *font);

// sets the title of the notification with some options
CENGINE_EXPORT void ui_notification_set_title (Notification *noti, const char *text, 
    Font *font, u32 size, RGBA_Color color);

// sets some options for the notification's title
CENGINE_EXPORT void ui_notification_set_title_options (Notification *noti, 
    Font *font, u32 size, RGBA_Color color);

// sets the color of the notification's title
CENGINE_EXPORT void ui_notification_set_title_color (Notification *noti, RGBA_Color color);

// sets the font to be used in the notification's title
CENGINE_EXPORT void ui_notification_set_title_font (Notification *noti, Font *font);

// sets the msg of the notification with some options
CENGINE_EXPORT void ui_notification_set_msg (Notification *noti, const char *text, 
    Font *font, u32 size, RGBA_Color color);

// sets some options for the notification's msg
CENGINE_EXPORT void ui_notification_set_msg_options (Notification *noti, 
    Font *font, u32 size, RGBA_Color color);

// sets the color of the notification's message
CENGINE_EXPORT void ui_notification_set_msg_color (Notification *noti, RGBA_Color color);

// sets the font to be used in the notification's msg
CENGINE_EXPORT void ui_notification_set_msg_font (Notification *noti, Font *font);

// sets a custom background color for the notification
CENGINE_EXPORT void ui_notification_set_bg_color (Notification *noti, RGBA_Color color);

// creates and display a notification
// type: default types have already created designs, select custom to pass your own
// lifetime: secs to display
// display_timestamp: option to display the timestamp of the notification
// title: the title of the notification (NULL for empty)
// msg: the message to be displayed (NULL for empty)
// uses the first set font for the texts with default values for sizes
CENGINE_EXPORT Notification *ui_notification_create (NotificationType type, float lifetime, bool display_timestamp,
    const char *title, const char *msg);

// adds the notification to the notification center where you want to display it
// and displays the notification for x seconds (lifetime)
CENGINE_EXPORT void ui_notification_display (struct _NotiCenter *noti_center, Notification *notification, Renderer *renderer);

// creates the notification with the passed values and default options by notification type
// and then displays it in the notification center
CENGINE_EXPORT void ui_notification_create_and_display (struct _NotiCenter *noti_center, Renderer *renderer, NotificationType type, 
    float lifetime, bool display_timestamp,
    const char *title, const char *msg);

/*** Notification Center ***/

// FIXME: change to work with percentages!!
#define NOTI_CENTER_DEFAULT_WIDTH           400
#define NOTI_CENTER_DEFAULT_HEIGHT          1000

struct _NotiCenter {

    UIElement *ui_element;

    bool outline;
    RGBA_Color outline_colour;
    float outline_scale_x;
    float outline_scale_y;

    bool colour;
    RGBA_Color bg_colour;
    SDL_Texture *bg_texture;
    SDL_Rect bg_texture_rect;

    DoubleList *notifications;          // notifications queue
    u8 max_display;                     // max notis to be displayed at once
    
    bool bottom;
    DoubleList *active_notifications;   // notifications being displayed
    u32 offset;

};

typedef struct _NotiCenter NotiCenter;

CENGINE_PUBLIC void ui_noti_center_delete (void *noti_center_ptr);

// creates a new notification center
// max_display: max number of notifications to display at once
// position: where do you want the notification center to be
CENGINE_EXPORT NotiCenter *ui_noti_center_create (UI *ui, u8 max_display, UIPosition pos, Renderer *renderer);

// sets the notification center position in the screen
CENGINE_EXPORT void ui_noti_center_set_position (NotiCenter *noti_center, UIPosition pos);

// sets the notification center dimensions
CENGINE_EXPORT void ui_noti_center_set_dimensions (NotiCenter *noti_center, u32 width, u32 height);

// sets the noti center's outline colour
CENGINE_EXPORT void ui_noti_center_set_ouline_colour (NotiCenter *noti_center, RGBA_Color colour);

// sets the noti center's outline scale
CENGINE_EXPORT void ui_noti_center_set_ouline_scale (NotiCenter *noti_center, float x_scale, float y_scale);

// removes the ouline form the noti center
CENGINE_EXPORT void ui_noti_center_remove_outline (NotiCenter *noti_center);

// sets the notification center background color
CENGINE_EXPORT void ui_noti_center_set_bg_color (NotiCenter *noti_center, Renderer *renderer, RGBA_Color color);

// removes the background from the noticenter
CENGINE_EXPORT void ui_noti_center_remove_background (NotiCenter *noti_center);

 // draws the notification center
CENGINE_PRIVATE void ui_noti_center_draw (NotiCenter *noti_center, Renderer *renderer);

#endif