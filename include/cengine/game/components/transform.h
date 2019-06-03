#ifndef _CENGINE_COMPONENT_TRANSFORM_H_
#define _CENGINE_COMPONENT_TRANSFORM_H_

#include "cengine/types/types.h"
#include "cengine/types/vector2d.h"

typedef struct Transform {

    u32 goID;
    Vector2D position;
    // Layer layer;

} Transform;

extern Transform *transform_new (u32 objectID);
extern void transform_destroy (Transform *transform);

#endif