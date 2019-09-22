#include <stdlib.h>

#include "cengine/types/types.h"
#include "cengine/types/vector2d.h"
#include "cengine/game/components/transform.h"

Transform *transform_new (u32 objectID) {

    Transform *new_transform = (Transform *) malloc (sizeof (Transform));
    if (new_transform) {
        new_transform->go_id = objectID;
        new_transform->position.x = 0;
        new_transform->position.y = 0;
        // new_transform->layer = UNSET_LAYER;
    }

    return new_transform;

}

void transform_destroy (Transform *transform) { if (transform) free (transform); }