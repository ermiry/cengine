#include <math.h>

#include "cengine/types/vector2d.h"

const Vector2D zeroVector = { 0, 0 };

float vector_magnitude (Vector2D v) { return sqrtf (v.x * v.x + v.y * v.y); }

static const float Epsilon = 0.00001F;

Vector2D vector_divide (Vector2D v, float divisor);

Vector2D vector_normalize (Vector2D vector) {

    float magnitude = vector_magnitude (vector);
    if (magnitude > Epsilon) return vector_divide (vector, magnitude);
    else return zeroVector;

}

Vector2D vector_negate (Vector2D v) {

    Vector2D n = { -v.x, -v.y };
    return n;

}

float vector_dot (Vector2D a, Vector2D b) { return (a.x * b.x) + (a.y * b.y); }

Vector2D vector_add (Vector2D a, Vector2D b) {

    Vector2D c = { a.x + b.x, a.y + b.y };
    return c;

}

void vector_add_equal (Vector2D *a, Vector2D b) { a->x += b.x; a->y += b.y; }

Vector2D vector_substract (Vector2D a, Vector2D b) {

    Vector2D c = { a.x - b.x, a.y - b.y };
    return c;

}

void vector_substract_equal (Vector2D *a, Vector2D b) { a->x -= b.x; a->y -= b.y; }

Vector2D vector_multiply (Vector2D v, float scalar) {

    Vector2D r = { v.x * scalar, v.y * scalar };
    return r;
}

void vector_multiply_equal (Vector2D *v, float scalar) { v->x *= scalar; v->y *= scalar; }

Vector2D vector_divide (Vector2D v, float divisor) {

    Vector2D r = { 0, 0 };
    if (divisor != 0) 
    {
        r.x = v.x / divisor;
        r.y = v.y / divisor;
    }
    return r;
}

void vector_divide_equal (Vector2D *v, float divisor) { 
    
    if (divisor) {
        v->x /= divisor; 
        v->y /= divisor;
    }  
    
}

Vector2D unit_vector (Vector2D v) {

    float length = vector_magnitude (v);
    if (length > 0) return vector_divide (v, length);
    return v;

}

Vector2D rotate_vector (Vector2D v, float radian) {

    float sine = sinf (radian);
    float cosine = cosf (radian);
    Vector2D r = { v.x * cosine + v.y * sine, v.x * sine + v.y * cosine };
    return r;

}

/*** UNITY LIKE FUNCS ***/

// returns a copy of (vector) with its magnitude clamped to (maxLength)
Vector2D vector_clamp_magnitude (Vector2D vector, float maxLength) {

    float magnitude = vector_magnitude (vector);
    if ((magnitude * magnitude) > (maxLength * maxLength)) 
        return vector_multiply (vector_normalize (vector), maxLength);
    
    return vector;    

}

// moves a point (current) in a straight line towards a (target) point
Vector2D vector_move_towards (Vector2D current, Vector2D target, float maxDistanceDelta) {

    Vector2D toVector = vector_substract (target, current);
    float dist = vector_magnitude (toVector);

    // NOTE: original code uses float.epsilon (roughly 1.401298E-45)
    if (dist <= maxDistanceDelta || dist < 0)
        return target;
    
    // original -> current + toVector / dist * maxDistanceDelta;
    return vector_add (current, vector_divide (toVector, (dist * maxDistanceDelta)));

}

// current velocity is passed as a ref in original code
// gradually changes a vector towards a desired goal over time
Vector2D vector_smooth_damp (Vector2D current, Vector2D target, Vector2D *currentVelocity,
    float smoothTime, float maxSpeed, float deltaTime) {

    // smoothTime = Mathf.Max (0.0001F, smoothTime);
    if (smoothTime < 0.0001) smoothTime = 0.0001;
    float omega = 2 / smoothTime;

    float x = omega * deltaTime;
    float expo = 1 / (1 + x + 0.48 * x * x + 0.235 * x * x * x);
    Vector2D change = vector_substract (current, target);
    Vector2D originalTo = target;

    float maxChange = maxSpeed * smoothTime;
    change = vector_clamp_magnitude (change, maxChange);
    target = vector_substract (current, change);

    // Vector3 temp = (currentVelocity + omega * change) * deltaTime;
    Vector2D temp = vector_multiply (vector_add (*currentVelocity, vector_multiply (change, omega)), deltaTime);
    // currentVelocity = (currentVelocity - omega * temp) * exp;
    *currentVelocity = vector_multiply (vector_substract (*currentVelocity, vector_multiply (temp, omega)), expo);
    // Vector3 output = target + (change + temp) * exp;
    Vector2D output = vector_add (target, vector_multiply (vector_add (change, temp), expo))  ;

    if (vector_dot (vector_substract (originalTo, current), vector_substract (output, originalTo)) > 0) {
        output = originalTo;
        *currentVelocity = vector_divide (vector_substract (output, originalTo), deltaTime);
    }

    return output;

}