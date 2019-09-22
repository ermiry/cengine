#ifndef _CENGINE_UI_POSITION_H_
#define _CENGINE_UI_POSITION_H_

#include <stdbool.h>

#include <SDL2/SDL_rect.h>

typedef enum UIPosition {

    UI_POS_FREE = 0,

    UI_POS_MIDDLE_CENTER,

    UI_POS_UPPER_CENTER,
    UI_POS_RIGHT_UPPER_CORNER,
    UI_POS_RIGHT_CENTER,
    UI_POS_RIGHT_BOTTOM_CORNER,
    UI_POS_BOTTOM_CENTER,
    UI_POS_LEFT_BOTTOM_CORNER,
    UI_POS_LEFT_CENTER,
    UI_POS_LEFT_UPPER_CORNER

} UIPosition;

extern void ui_position_update (void *transform_ptr, SDL_Rect *ref_rect, bool offset);

#endif