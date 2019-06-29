#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "cengine/os.h"

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/cengine.h"
#include "cengine/timer.h"
#include "cengine/sprites.h"
#include "cengine/threads/thread.h"
#include "cengine/animation.h"
#include "cengine/game/go.h"

#include "cengine/collections/dlist.h"

#include "cengine/utils/file.h"
#include "cengine/utils/json.h"
#include "cengine/utils/log.h"

static bool anim_init = false;

static AnimData *anim_data_new (void) {

    AnimData *anim_data = (AnimData *) malloc (sizeof (AnimData));
    if (anim_data) {
        memset (anim_data, 0, sizeof (AnimData));
        anim_data->animations = dlist_init (animation_delete, NULL);
    }

    return anim_data;

}

// the animations list is used in the entity's component
void anim_data_delete (AnimData *data) { 
    
    if (data) {
        dlist_destroy (data->animations);
        free (data); 
    } 
    
}

/*** Animation Files ***/

// parse the array of anim points for a given animation and return them in a list
static DoubleList *animation_file_parse_anim_points (unsigned int n_points, json_value *points_array) {

    DoubleList *points = NULL;

    if (points_array) {
        points = dlist_init (free, NULL);

        IndividualSprite *p = NULL;
        json_value *point_object = NULL;
        for (unsigned int i = 0; i < n_points; i++) {
            point_object = points_array->u.array.values[i];
            if (point_object) {
                p = (IndividualSprite *) malloc (sizeof (IndividualSprite));
                memset (p, 0, sizeof (IndividualSprite));

                p->col = point_object->u.object.values[0].value->u.integer;
                p->row = point_object->u.object.values[1].value->u.integer;

                dlist_insert_after (points, dlist_end (points), p);
            }
        }
    }

    return points;

}

// parses an animation json file into a list of animations
AnimData *animation_file_parse (const char *filename) {

    AnimData *anim_data = NULL;

    if (filename) {
        json_value *value = file_json_parse (filename);

        if (value) {
            anim_data = anim_data_new ();

            // process json values into individual animations
            json_value *size_object = value->u.object.values[0].value;
            anim_data->w = size_object->u.object.values[0].value->u.integer;
            anim_data->h = size_object->u.object.values[1].value->u.integer;

            anim_data->scale = value->u.object.values[1].value->u.integer;

            json_value *animations_array = value->u.object.values[2].value;
            json_value *anim_object = NULL;
            Animation *anim = NULL;
            for (unsigned int i = 0; i < animations_array->u.array.length; i++) {
                anim_object = animations_array->u.array.values[i];

                int n_frames = anim_object->u.object.values[1].value->u.integer;
                DoubleList *anim_points = animation_file_parse_anim_points (n_frames, anim_object->u.object.values[2].value);

                const char *name = anim_object->u.object.values[0].value->u.string.ptr;
                int speed = anim_object->u.object.values[3].value->u.integer;

                anim = animation_create (name, n_frames, anim_points, speed);
                dlist_insert_after (anim_data->animations, dlist_end (anim_data->animations), anim);
            }

            json_value_free (value);
        }
    }

    return anim_data;

}

/*** Animation ***/

Animation *animation_new (u8 n_frames, ...) {

    va_list valist;
    va_start (valist, n_frames);

    Animation *animation = (Animation *) malloc (sizeof (Animation));
    if (animation) {
        animation->name = NULL;
        animation->speed = DEFAULT_ANIM_SPEED;
        animation->n_frames = n_frames;
        animation->frames = (IndividualSprite **) calloc (n_frames, sizeof (IndividualSprite *));

        for (u8 i = 0; i < n_frames; i++)
            animation->frames[i] = va_arg (valist, IndividualSprite *);

        va_end (valist);
    }

    return animation;

}

// create an animation with the requested values
Animation *animation_create (const char *name, u8 n_frames, DoubleList *anim_points, unsigned int speed) {

    Animation *anim = (Animation *) malloc (sizeof (Animation));
    if (anim) {
        anim->name = str_create (name);
        anim->speed = speed;

        anim->n_frames = n_frames;
        anim->frames = (IndividualSprite **) calloc (n_frames, sizeof (IndividualSprite *));
        unsigned int i = 0;
        for (ListElement *le = dlist_start (anim_points); le; le = le->next) {
            anim->frames[i] = (IndividualSprite *) le->data;
            i++;
        }

        dlist_clean (anim_points);
    }

    return anim;

}

void animation_delete (void *ptr) {

    if (ptr) {
        Animation *animation = (Animation *) ptr;
        str_delete (animation->name);
        if (animation->frames) free (animation->frames);

        free (animation);
    }

}

void animation_set_name (Animation *animation, const char *name) {

    if (animation && name) animation->name = str_new (name);

}

void animation_set_speed (Animation *animation, u32 speed) {

    if (animation) animation->speed = speed;

}

Animation *animation_get_by_name (DoubleList *animations, const char *name) {

    Animation *retval = NULL;

    if (animations && name) {
        Animation *anim = NULL;
        for (ListElement *le = dlist_start (animations); le; le = le->next) {
            anim = (Animation *) le->data;
            if (!strcmp (anim->name->str, name)) {
                retval = anim;
                break;
            }
        }
    }

    return retval;

}

/*** Animator ***/

DoubleList *animators = NULL;

Animator *animator_new (u32 objectID) {

    Animator *new_animator = (Animator *) malloc (sizeof (Animator));
    if (new_animator) {
        new_animator->goID = objectID;
        new_animator->start = true;
        new_animator->playing = false;
        new_animator->defaultAnimation = NULL;
        new_animator->currAnimation = NULL;
        new_animator->currFrame = 0;
        new_animator->n_animations = 0;
        new_animator->animations = NULL;
        new_animator->timer = timer_new ();

        dlist_insert_after (animators, dlist_end (animators), new_animator);
    }
    
    return new_animator;

}

void animator_destroy (Animator *animator) {

    if (animator) {
        if (animator->animations) {
            for (u8 i = 0; i < animator->n_animations; i++)
                if (animator->animations[i])
                    animation_delete (animator->animations[i]);

            free (animator->animations);
        }

        timer_destroy (animator->timer);

        void *data = dlist_remove_element (animators, dlist_get_element (animators, animator));
        if (data) free (data);
        else free (animator);
    }

}

static void animator_destroy_ref (void *data) { 
    
    if (data) animator_destroy ((Animator *) data);
    
}

static int animator_comparator (void *one, void *two) {

    if (one && two) {
        Animator *anim_one = (Animator *) one;
        Animator *anim_two = (Animator *) two;

        if (anim_one->goID < anim_two->goID) return -1;
        else if (anim_one->goID == anim_two->goID) return 0;
        else return 1;
    }
}

void animator_set_default_animation (Animator *animator, Animation *animation) {

    if (animator && animation) 
        animator->defaultAnimation = animation;

}

void animator_set_current_animation (Animator *animator, Animation *animation) {

    if (animator && animation) 
        if (!animator->playing) animator->currAnimation = animation; 

}

void animator_play_animation (Animator *animator, Animation *animation) {

    if (animator && animation) {
        animator_set_current_animation (animator, animation);
        animator->start = true;
        animator->playing = true;
        animator->currFrame = 0;
        timer_start (animator->timer);
    } 

}

/*** Anim Thread ***/

void *animations_update (void *data) {

    thread_set_name ("animation");

    u32 timePerFrame = 1000 / fps_limit;
    u32 frameStart = 0;
    i32 sleepTime = 0;

    float deltaTime = 0;
    u32 deltaTicks = 0;
    u32 fps = 0;

    while (running) {
        frameStart = SDL_GetTicks ();

        if (dlist_size (animators) > 0) {
             // update all animations
            Animator *animator = NULL;
            Graphics *graphics = NULL;
            for (ListElement *le = dlist_start (animators); le != NULL; le = le->next) {
                animator = (Animator *) le->data;
                graphics = (Graphics *) game_object_get_component (game_object_get_by_id (animator->goID), GRAPHICS_COMP);
                animator->currFrame = (int) (((animator->timer->ticks / animator->currAnimation->speed) %
                        animator->currAnimation->n_frames));

                graphics->x_sprite_offset = animator->currAnimation->frames[animator->currFrame]->col;
                graphics->y_sprite_offset = animator->currAnimation->frames[animator->currFrame]->row;

                if (animator->playing) {
                    if (animator->currFrame >= (animator->currAnimation->n_frames - 1)) {
                        animator->playing = false;
                        animator->currAnimation = animator->defaultAnimation;
                        animator->currFrame = 0;
                        timer_start (animator->timer);
                    }
                }
            }
        }

        // limit the FPS
        sleepTime = timePerFrame - (SDL_GetTicks () - frameStart);
        if (sleepTime > 0) SDL_Delay (sleepTime);

        // update animators timers
        if (dlist_size (animators) > 0) {
            Animator *animator = NULL;
            for (ListElement *le = dlist_start (animators); le; le = le->next) {
                animator = (Animator *) le->data;
                animator->timer->ticks = SDL_GetTicks () - animator->timer->startTicks;
            }
        }

        // count fps
        deltaTime = SDL_GetTicks () - frameStart;
        deltaTicks += deltaTime;
        fps++;
        if (deltaTicks >= 1000) {
            // printf ("anim fps: %i\n", fps);
            deltaTicks = 0;
            fps = 0;
        }
    }

}

/*** Public ***/

int animations_init (void) {

    int errors = 0;

    animators = dlist_init (animator_destroy_ref, animator_comparator);
    if (animators) {
        if (thread_create_detachable (animations_update, NULL, "animations")) {
            cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to create animations thread!");
            errors = 1;
        }
    }

    else {
        #ifdef CENGINE_DEBUG
        cengine_log_msg (stderr, ERROR, NO_TYPE, "Failed to create animators list!");
        #endif
        errors = 1;
    }

    return errors;

}

void animations_end (void) {

    dlist_destroy (animators);

}