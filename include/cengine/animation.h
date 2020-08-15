#ifndef _CENGINE_ANIMATION_H_
#define _CENGINE_ANIMATION_H_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/collections/dlist.h"

#include "cengine/config.h"
#include "cengine/timer.h"
#include "cengine/sprites.h"

#define DEFAULT_ANIM_SPEED      100

typedef struct AnimData {

    unsigned int w, h;
    int scale;
    DoubleList *animations;

} AnimData;

CENGINE_PUBLIC void anim_data_delete (AnimData *data);

// parses an animation json file into a list of animations
CENGINE_EXPORT AnimData *animation_file_parse (const char *filename);

typedef struct Animation {

    String *name;
    IndividualSprite **frames;
    u8 n_frames;
    u32 speed;

} Animation;

CENGINE_EXPORT Animation *animation_new (u8 n_frames, ...);

// create an animation with the requested values
CENGINE_EXPORT Animation *animation_create (const char *name, u8 n_frames, DoubleList *anim_points, unsigned int speed);

CENGINE_EXPORT void animation_delete (void *ptr);

CENGINE_EXPORT void animation_set_name (Animation *animation, const char *name);

CENGINE_EXPORT void animation_set_speed (Animation *animation, u32 speed);

CENGINE_EXPORT Animation *animation_get_by_name (DoubleList *animations, const char *name);

typedef struct Animator {

    // 03/02/2020 -- 9:57 -- unique animator id
    u32 id;

    u32 go_id;
    bool start;
    bool playing;
    u8 currFrame;
    u8 n_animations;
    Animation **animations;
    Animation *currAnimation;
    Animation *defaultAnimation;
    Timer *timer;

} Animator;

CENGINE_PUBLIC Animator *animator_new (u32 objectID);

CENGINE_PUBLIC void animator_destroy (Animator *animator);

CENGINE_EXPORT void animator_set_default_animation (Animator *animator, Animation *animation);

CENGINE_EXPORT void animator_set_current_animation (Animator *animator, Animation *animation);

CENGINE_EXPORT void animator_play_animation (Animator *animator, Animation *animation);

/*** ANIM THREAD ***/

CENGINE_PRIVATE int animations_init (void);

CENGINE_PRIVATE u8 animations_end (void);

#endif