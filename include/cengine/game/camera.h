#ifndef _CENGINE_CAMERA_H_
#define _CENGINE_CAMERA_H_

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/game/go.h"
#include "cengine/types/vector2d.h"

#define DEFAULT_CAM_ACCEL           1
#define DEFAULT_CAM_MAX_VEL         2
#define DEFAULT_CAM_MIN_VEL         1
#define DEFAULT_CAM_DRAG            0.9

#define DEFAULT_CAM_ZOOM_SPEED      0.01
#define DEFAULT_CAM_ZOOM_TO_SPEED   1
#define DEFAULT_CAM_MIN_ZOOM        1.5
#define DEFAULT_CAM_MAX_ZOOM        0.5

#define DEFAULT_CAM_ORTHO_SIZE      1

typedef struct SDL_Rect CamRect;

struct _Camera {

    u32 windowWidth, windowHeight;

    // position
    Transform transform;
    CamRect bounds;

    // motion
    float accelerationRate;
    float maxVel;
    float drag;
    Vector2D velocity;
    Vector2D direction;
    Vector2D center;
    Vector2D acceleration;

    bool isFollwing;
    Transform *target;

};

typedef struct _Camera Camera;

extern Camera *main_camera;

extern Camera *camera_new (u32 windowWidth, u32 windowHeight);
extern void camera_destroy (Camera *cam);

extern void camera_update (Camera *cam);

extern void camera_set_center (Camera *cam, u32 x, u32 y);
extern void camera_set_size (Camera *cam, u32 width, u32 height);

extern void camera_set_target (Camera *cam, Transform *target);

extern CamRect camera_world_to_screen (Camera *cam, const CamRect destRect);

#endif