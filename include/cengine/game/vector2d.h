#ifndef _CENGINE_VECTOR2D_H_
#define _CENGINE_VECTOR2D_H_

typedef struct Point {

    int x, y;

} Point;

typedef struct Vector2D { 

    float x, y;

} Vector2D;

extern float vector_magnitude (Vector2D v);
extern Vector2D vector_normalize (Vector2D vector);

extern Vector2D vector_negate (Vector2D v);

extern float vector_dot (Vector2D a, Vector2D b);

extern Vector2D vector_add (Vector2D a, Vector2D b);
extern void vector_add_equal (Vector2D *a, Vector2D b);

extern Vector2D vector_substract (Vector2D a, Vector2D b);
extern void vector_substract_equal (Vector2D *a, Vector2D b);

extern Vector2D vector_multiply (Vector2D v, float scalar);
extern void vector_multiply_equal (Vector2D *v, float scalar);

extern Vector2D vector_divide (Vector2D v, float divisor);
extern void vector_divide_equal (Vector2D *v, float divisor);

extern Vector2D vector_project (Vector2D project, Vector2D onto);
extern Vector2D unit_vector (Vector2D v);
extern Vector2D vector_rotate (Vector2D v, float radian);
// extern Vector2D vector_rotate_90 (Vector2D v);

/*** UNITY LIKE FUNCS ***/

// returns a copy of (vector) with its magnitude clamped to (maxLength)
extern Vector2D vector_clamp_magnitude (Vector2D vector, float maxLength);

// moves a point (current) in a straight line towards a (target) point
extern Vector2D vector_move_towards (Vector2D current, Vector2D target, float maxDistanceDelta);

// gradually changes a vector towards a desired goal over time
extern Vector2D vector_smooth_damp (Vector2D current, Vector2D target, Vector2D *currentVelocity,
    float smoothTime, float maxSpeed, float deltaTime);

#endif