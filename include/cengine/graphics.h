#ifndef _CENGINE_GRAPHICS_H_
#define _CENGINE_GRAPHICS_H_

#include <SDL2/SDL_pixels.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/config.h"

/*** RGBA Colors ***/

typedef SDL_Color RGBA_Color;

CENGINE_PUBLIC RGBA_Color RGBA_NO_COLOR;
CENGINE_PUBLIC RGBA_Color RGBA_WHITE;
CENGINE_PUBLIC RGBA_Color RGBA_BLACK;
CENGINE_PUBLIC RGBA_Color RGBA_RED;
CENGINE_PUBLIC RGBA_Color RGBA_GREEN;
CENGINE_PUBLIC RGBA_Color RGBA_BLUE;

CENGINE_PUBLIC RGBA_Color rgba_color_create (u8 r, u8 g, u8 b, u8 a);

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

typedef enum Flip {

    NO_FLIP             = 0x00000000,
    FLIP_HORIZONTAL     = 0x00000001,
    FLIP_VERTICAL       = 0x00000002

} Flip;

struct _ImageData {

    u32 w, h;
    String *filename;

};

typedef struct _ImageData ImageData;

CENGINE_PUBLIC ImageData *image_data_new (u32 w, u32 h, String *filename);

CENGINE_PUBLIC void image_data_delete (void *img_data_ptr);

#endif