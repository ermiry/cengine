#include <stdlib.h>
#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/game/components/collider.h"

BoxCollider *collider_box_new (u32 objectID) {

    BoxCollider *new_collider = (BoxCollider *) malloc (sizeof (BoxCollider));
    if (new_collider) {
        new_collider->x = new_collider->y = 0;
        new_collider->w = new_collider->h = 0;
    }

    return new_collider;

}

void collider_box_init (u32 x, u32 y, u32 w, u32 h) {}

void collider_box_delete (BoxCollider *box) { if (box) free (box); }

bool collider_box_collision (const BoxCollider *a, const BoxCollider *b) {

    if (a && b) 
        if (a->x + a->w >= b->x &&
            b->x + b->w >= a->x &&
            a->y + a->h >= b->y &&
            b->y + b->h >= a->y)
                return true;

    return false;

}