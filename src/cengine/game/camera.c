#include <stdlib.h>

#include "cengine/renderer.h"
#include "cengine/input.h"
#include "cengine/game/camera.h"
#include "cengine/types/vector2d.h"

Camera *main_camera = NULL;

static void camera_init (Camera *cam, u32 windowWidth, u32 windowHeight);

// camera constructor
Camera *camera_new (u32 windowWidth, u32 windowHeight) {

    Camera *cam = (Camera *) malloc (sizeof (Camera));
    if (cam) camera_init (cam, windowWidth, windowHeight);

    return cam;

}

void camera_destroy (Camera *cam) {

    if (cam) {
        cam->target = NULL;

        free (cam);
    }

}

void camera_set_center (Camera *cam, u32 x, u32 y) {

    if (cam) {
        cam->center.x = x;
        cam->center.y = y;
        cam->bounds.x = x - (cam->bounds.w * 0.5);
        cam->bounds.y = y - (cam->bounds.h * 0.5);
    }

}

// TODO:
void camera_set_size (Camera *cam, u32 width, u32 height) {

    if (cam) {

    }

}

void camera_set_vel (Camera *cam, Vector2D vel) { if (cam) cam->velocity = vel; }

void camera_set_acceleration (Camera *cam, Vector2D accel) { if (cam) cam->acceleration = accel; }

void camera_set_direction (Camera *cam, Vector2D dir) { if (cam) cam->direction = dir; }

void camera_set_target (Camera *cam, Transform *target) {

    if (cam && target) {
        cam->target = target;
        cam->isFollwing = true;
    }

}

// set camera parameters to default
static void camera_init (Camera *cam, u32 windowWidth, u32 windowHeight) {

    // position
    cam->transform.position.x = main_renderer->window_size.width / 2;
    cam->transform.position.y = main_renderer->window_size.height / 2;

    cam->windowWidth = windowWidth;
    cam->windowHeight = windowHeight;

    cam->bounds.x = cam->bounds.y = 0;
    cam->bounds.w = windowWidth;
    cam->bounds.h = windowHeight;

    // motion
    cam->accelerationRate = DEFAULT_CAM_ACCEL;
    cam->maxVel = DEFAULT_CAM_MAX_VEL;
    cam->drag = DEFAULT_CAM_DRAG;

    camera_set_center (cam, 0, 0);

    Vector2D nullVector = { 0, 0 };
    camera_set_vel (cam, nullVector);
    camera_set_acceleration (cam, nullVector);
    camera_set_direction (cam, nullVector);

    cam->isFollwing = false;
    cam->target = NULL;

}

/*** SCREEN ***/

#pragma region SCREEN

static Point point_world_to_screen (Camera *cam, const Point p, 
    float xScale, float yScale) {

    Point retPoint = p;

    retPoint.x -= cam->bounds.x;
    retPoint.y -= cam->bounds.y;

    retPoint.x *= xScale;
    retPoint.y *= yScale;

    return retPoint;

}

CamRect camera_world_to_screen (Camera *cam, const CamRect destRect) {

    if (cam) {
        CamRect screenRect = destRect;

        float xScale = (float) cam->windowWidth / cam->bounds.w;
        float yScale = (float) cam->windowHeight / cam->bounds.h;

        Point screenPoint = { screenRect.x, screenRect.y };
        screenPoint = point_world_to_screen (cam, screenPoint, xScale, yScale);

        screenRect.x = screenPoint.x;
        screenRect.y = screenPoint.y;
        screenRect.w = (int) (screenRect.w * xScale);
        screenRect.h = (int) (screenRect.h * yScale);

        return screenRect;
    }

}

#pragma endregion

/*** MOVEMENT ***/

#pragma region MOVEMENT

void camera_set_max_vel (Camera *cam, float maxVel) { 
    
    if (cam) cam->maxVel = maxVel > 0 ? maxVel : 0; 
    
}

// TODO: do we need to normalize the movement when in diagonal?
// TODO: check again with draft
void camera_pan (Camera *cam, float xdir, float ydir) {

    if (cam) {
        Vector2D move = { xdir, ydir };
        // vector_multiply_equal (&move, cam->accelerationRate);
        // vector_add_equal (&cam->acceleration, move);

        vector_add_equal (&cam->center, move);
    }

}

#pragma endregion

void camera_update (Camera *cam) {

    // camera input
    #ifdef DEV
        if (input_is_key_down (SDL_SCANCODE_LEFT)) camera_pan (cam, -15, 0);
        if (input_is_key_down (SDL_SCANCODE_RIGHT)) camera_pan (cam, 15, 0);
        if (input_is_key_down (SDL_SCANCODE_UP)) camera_pan (cam, 0, -15);
        if (input_is_key_down (SDL_SCANCODE_DOWN)) camera_pan (cam, 0, 15);
    #endif

    // camera movement
    u32 x = cam->transform.position.x;
    u32 y = cam->transform.position.y;

    // FIXME:
    // if (cam->isFollwing) {
    //     if (abs (x - cam->target->position.x) > cam->margin.x)
    //         x = lerp (x, cam->target->position.x, cam->smoothing.x * deltaTime);

    //     if (abs (y - cam->target->position.y) > cam->margin.y)
    //         y = lerp (y, cam->target->position.y, cam->smoothing.y * deltaTime);
    // }

    // bounds - used to calculate what gets rendered to the screen
    cam->bounds.x = (cam->center.x - cam->bounds.w * 0.5);
    cam->bounds.y = (cam->center.y - cam->bounds.h * 0.5);

}