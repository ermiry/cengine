#ifndef _CENGINE_ANIMATION_H_
#define _CENGINE_ANIMATION_H_

#include <stdbool.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/timer.h"
#include "cengine/sprites.h"
#include "cengine/collections/dlist.h"

#define DEFAULT_ANIM_SPEED      100

typedef struct AnimData {

    unsigned int w, h;
    int scale;
    DoubleList *animations;

} AnimData;

extern void anim_data_delete (AnimData *data);

// parses an animation json file into a list of animations
extern AnimData *animation_file_parse (const char *filename);

typedef struct Animation {

    String *name;
    IndividualSprite **frames;
    u8 n_frames;
    u32 speed;

} Animation;

extern Animation *animation_new (u8 n_frames, ...);
// create an animation with the requested values
extern Animation *animation_create (const char *name, u8 n_frames, DoubleList *anim_points, unsigned int speed);
extern void animation_delete (void *ptr);

extern void animation_set_name (Animation *animation, const char *name);
extern void animation_set_speed (Animation *animation, u32 speed);

extern Animation *animation_get_by_name (DoubleList *animations, const char *name);

typedef struct Animator {

    u32 goID;
    bool start;
    bool playing;
    u8 currFrame;
    u8 n_animations;
    Animation **animations;
    Animation *currAnimation;
    Animation *defaultAnimation;
    Timer *timer;

} Animator;

extern Animator *animator_new (u32 objectID);
extern void animator_destroy (Animator *animator);

void animator_set_default_animation (Animator *animator, Animation *animation);
extern void animator_set_current_animation (Animator *animator, Animation *animation);
extern void animator_play_animation (Animator *animator, Animation *animation);

/*** ANIM THREAD ***/

extern int animations_init (void);
extern void animations_end (void);

#endif