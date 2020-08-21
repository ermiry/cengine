#ifndef _CENGINE_CAMERA_H_
#define _CENGINE_CAMERA_H_

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/types/types.h"
#include "cengine/types/vector2d.h"

#include "cengine/config.h"
#include "cengine/renderer.h"

#include "cengine/game/go.h"
#include "cengine/game/components/transform.h"

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

CENGINE_PUBLIC Camera *main_camera;

CENGINE_PUBLIC Camera *camera_new (Renderer *renderer);

CENGINE_PUBLIC void camera_destroy (Camera *cam);

CENGINE_PUBLIC void camera_update (Camera *cam);

CENGINE_PUBLIC void camera_set_center (Camera *cam, u32 x, u32 y);

CENGINE_PUBLIC void camera_set_size (Camera *cam, u32 width, u32 height);

CENGINE_PUBLIC void camera_set_target (Camera *cam, Transform *target);

CENGINE_PUBLIC CamRect camera_world_to_screen (Camera *cam, const CamRect destRect);

#endif