#ifndef _CENGINE_TYPES_VECTOR2D_H_
#define _CENGINE_TYPES_VECTOR2D_H_

#include "cengine/config.h"

typedef struct Vector2D { 

    float x, y;

} Vector2D;

CENGINE_PUBLIC const Vector2D zero_vector;

CENGINE_PUBLIC Vector2D *vector_new (void);

CENGINE_PUBLIC Vector2D *vector_create (float x, float y);

CENGINE_PUBLIC void vector_delete (void *vector_ptr);

CENGINE_PUBLIC float vector_magnitude (Vector2D v);

CENGINE_PUBLIC Vector2D vector_normalize (Vector2D vector);

CENGINE_PUBLIC Vector2D vector_negate (Vector2D v);

CENGINE_PUBLIC float vector_dot (Vector2D a, Vector2D b);

CENGINE_PUBLIC Vector2D vector_add (Vector2D a, Vector2D b);

CENGINE_PUBLIC void vector_add_equal (Vector2D *a, Vector2D b);

CENGINE_PUBLIC Vector2D vector_substract (Vector2D a, Vector2D b);

CENGINE_PUBLIC void vector_substract_equal (Vector2D *a, Vector2D b);

CENGINE_PUBLIC Vector2D vector_multiply (Vector2D v, float scalar);

CENGINE_PUBLIC void vector_multiply_equal (Vector2D *v, float scalar);

CENGINE_PUBLIC Vector2D vector_divide (Vector2D v, float divisor);

CENGINE_PUBLIC void vector_divide_equal (Vector2D *v, float divisor);

CENGINE_PUBLIC Vector2D vector_project (Vector2D project, Vector2D onto);

CENGINE_PUBLIC Vector2D unit_vector (Vector2D v);

CENGINE_PUBLIC Vector2D vector_rotate (Vector2D v, float radian);

// CENGINE_PUBLIC Vector2D vector_rotate_90 (Vector2D v);

/*** Unity like functions ***/

// returns a copy of (vector) with its magnitude clamped to (maxLength)
CENGINE_PUBLIC Vector2D vector_clamp_magnitude (Vector2D vector, float maxLength);

// moves a point (current) in a straight line towards a (target) point
CENGINE_PUBLIC Vector2D vector_move_towards (Vector2D current, Vector2D target, float maxDistanceDelta);

// gradually changes a vector towards a desired goal over time
CENGINE_PUBLIC Vector2D vector_smooth_damp (Vector2D current, Vector2D target, Vector2D *currentVelocity,
    float smoothTime, float maxSpeed, float deltaTime);

#endif