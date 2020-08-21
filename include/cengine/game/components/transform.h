#ifndef _CENGINE_COMPONENT_TRANSFORM_H_
#define _CENGINE_COMPONENT_TRANSFORM_H_

#include "cengine/types/types.h"
#include "cengine/types/vector2d.h"

#include "cengine/config.h"

typedef struct Transform {

    u32 go_id;
    Vector2D position;
    // Layer layer;

} Transform;

CENGINE_PUBLIC Transform *transform_new (u32 objectID);

CENGINE_PUBLIC void transform_destroy (Transform *transform);

#endif