#ifndef _CENGINE_COMPONENTS_COLLIDER_H_
#define _CENGINE_COMPONENTS_COLLIDER_H_

#include <stdbool.h>

#include "cengine/types/types.h"

#include "cengine/config.h"

typedef struct BoxCollider {

    int x, y;
    int w, h;

} BoxCollider;

CENGINE_PUBLIC BoxCollider *collider_box_new (u32 objectID);

CENGINE_PUBLIC void collider_box_init (u32 x, u32 y, u32 w, u32 h);

CENGINE_PUBLIC void collider_box_delete (BoxCollider *box);

CENGINE_PUBLIC bool collider_box_collision (const BoxCollider *a, const BoxCollider *b);

#endif