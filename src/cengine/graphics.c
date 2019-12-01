#include <stdlib.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/graphics.h"

/*** Common RGBA Colors ***/

RGBA_Color RGBA_NO_COLOR = { 0, 0, 0, 0 };
RGBA_Color RGBA_WHITE = { 255, 255, 255, 255 };
RGBA_Color RGBA_BLACK = { 0, 0, 0, 255 };
RGBA_Color RGBA_RED = { 255, 0, 0, 255 };
RGBA_Color RGBA_GREEN = { 0, 255, 0, 255 };
RGBA_Color RGBA_BLUE = { 0, 0, 255, 255 };

RGBA_Color rgba_color_create (u8 r, u8 g, u8 b, u8 a) { 

    RGBA_Color retval = { r, g, b, a };
    return retval;

}

ImageData *image_data_new (u32 w, u32 h, String *filename) {

    ImageData *img_data = (ImageData *) malloc (sizeof (ImageData));
    if (img_data) {
        img_data->w = w;
        img_data->h = h;

        img_data->filename = filename ? filename : NULL;
    }

    return img_data;

}

void image_data_delete (void *img_data_ptr) { 
    
    if (img_data_ptr) {
        ImageData *img_data = (ImageData *) img_data_ptr;
        str_delete (img_data->filename);
        free (img_data_ptr);
    }  
    
}