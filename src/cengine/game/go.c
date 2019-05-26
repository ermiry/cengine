#include <stdlib.h>
#include <stdbool.h>

#include "cengine/renderer.h"
#include "cengine/textures.h"
#include "cengine/animation.h"
#include "cengine/game/go.h"

#pragma region Components

static Transform *transform_new (u32 objectID) {

    Transform *new_transform = (Transform *) malloc (sizeof (Transform));
    if (new_transform) {
        new_transform->goID = objectID;
        new_transform->position.x = 0;
        new_transform->position.y = 0;
    }

    return new_transform;

}

static void transform_destroy (Transform *transform) { if (transform) free (transform); }

static Graphics *graphics_new (u32 objectID) {

    Graphics *new_graphics = (Graphics *) malloc (sizeof (Graphics));
    if (new_graphics) {
        new_graphics->goID = objectID;
        new_graphics->sprite = NULL;
        new_graphics->spriteSheet = NULL;
        new_graphics->refSprite = false;
        new_graphics->multipleSprites = false;
        new_graphics->x_sprite_offset = 0;
        new_graphics->y_sprite_offset = 0;
        new_graphics->layer = UNSET_LAYER;
        new_graphics->flip = NO_FLIP;
    }

    return new_graphics;

}

static void graphics_destroy (Graphics *graphics) {

    if (graphics) {
        if (graphics->refSprite) {
            graphics->sprite = NULL;
            graphics->spriteSheet = NULL;
        }

        else {
            if (graphics->sprite) sprite_destroy (graphics->sprite);
            if (graphics->spriteSheet) sprite_sheet_destroy (graphics->spriteSheet);
        }

        free (graphics);
    }

}

void graphics_set_sprite (Graphics *graphics, const char *filename) {

    if (graphics && filename) {
        graphics->sprite = sprite_load (filename, main_renderer);
        graphics->spriteSheet = NULL;
        graphics->multipleSprites = false;
    }

}

void graphics_set_sprite_sheet (Graphics *graphics, const char *filename) {

    if (graphics && filename) {
        graphics->sprite = NULL;
        graphics->spriteSheet = sprite_sheet_load (filename, main_renderer);
        graphics->multipleSprites = true;
    }

}

void graphics_ref_sprite (Graphics *graphics, Sprite *sprite) {

    if (graphics && sprite) {
        graphics->sprite = sprite;
        graphics->spriteSheet = NULL;
        graphics->multipleSprites = false;
        graphics->refSprite = true;
    }

}

void graphics_ref_sprite_sheet (Graphics *graphics, SpriteSheet *spriteSheet) {

    if (graphics && spriteSheet) {
        graphics->sprite = NULL;
        graphics->spriteSheet = spriteSheet;
        graphics->multipleSprites = true;
        graphics->refSprite = true;
    }

}

// FIXME: Add colliders logic!

/* static BoxCollider *collider_box_new (u32 objectID) {

    BoxCollider *new_collider = (BoxCollider *) malloc (sizeof (BoxCollider));
    if (new_collider) {
        new_collider->x = new_collider->y = 0;
        new_collider->w = new_collider->h = 0;
    }

    return new_collider;

}

void collider_box_init (u32 x, u32 y, u32 w, u32 h) {}

bool collider_box_collision (const BoxCollider *a, const BoxCollider *b) {

    if (a && b) 
        if (a->x + a->w >= b->x &&
            b->x + b->w >= a->x &&
            a->y + a->h >= b->y &&
            b->y + b->h >= a->y)
                return true;

    return false;

}

static void collider_box_destroy (BoxCollider *box) { if (box) free (box); } */

#pragma endregion

#pragma region Game Objects

static GameObject **gameObjects;
static u32 max_gos;
static u32 curr_max_objs;
static u32 new_go_id;

static bool game_objects_realloc (void) {

    u32 new_max_gos = curr_max_objs * 2;

    gameObjects = realloc (gameObjects, new_max_gos * sizeof (GameObject *));

    if (gameObjects) {
        max_gos = new_max_gos;
        return true;
    }

    return false;

}

// init our game objects array
static u8 game_objects_init_all (void) {

    gameObjects = (GameObject **) calloc (DEFAULT_MAX_GOS, sizeof (GameObject *));
    if (gameObjects) {
        for (u32 i = 0; i < DEFAULT_MAX_GOS; i++) gameObjects[i] = NULL;

        max_gos = DEFAULT_MAX_GOS;
        curr_max_objs = 0;
        new_go_id = 0;

        return 0;
    }

    return 1;

}

static i32 game_object_get_free_spot (void) {

    for (u32 i = 0; i < curr_max_objs; i++)
        if (gameObjects[i]->id == -1)
            return i;

    return -1;

}

static void game_object_init (GameObject *go, u32 id, const char *name, const char *tag) {

    if (go) {
        go->id = id;

        if (name) {
            go->name = (char *) calloc (strlen (name) + 1, sizeof (char));
            strcpy (go->name, name);
        }

        else go->name = NULL;

        if (tag) {
            go->tag = (char *) calloc (strlen (tag) + 1, sizeof (char));
            strcpy (go->tag, tag);
        }

        else go->tag = NULL;

        for (u8 i = 0; i < COMP_COUNT; i++) go->components[i] = NULL;

        go->children = NULL;

        go->update = NULL;
    }

}

// game object constructor
GameObject *game_object_new (const char *name, const char *tag) {

    GameObject *new_go = NULL;

    // first check if we have a reusable go in the array
    i32 spot = game_object_get_free_spot ();

    if (spot >= 0) {
        new_go = gameObjects[spot];
        game_object_init (new_go, spot, name, tag);
    } 

    else {
        if (new_go_id >= max_gos) game_objects_realloc ();

        new_go = (GameObject *) malloc (sizeof (GameObject));
        if (new_go) {
            game_object_init (new_go, new_go_id, name, tag);
            gameObjects[new_go->id] = new_go;
            new_go_id++;
            curr_max_objs++;
        }
    } 

    return new_go;

}

// this is used to avoid go destruction when destroying go's children
static void game_object_destroy_dummy (void *ptr) {}

static int game_object_comparator (void *one, void *two) {

    if (one && two) {
        GameObject *go_one = (GameObject *) one;
        GameObject *go_two = (GameObject *) two;

        if (go_one->id < go_two->id) return -1;
        else if (go_one->id == go_two->id) return 0;
        else return 1;
    }

}

void game_object_add_child (GameObject *parent, GameObject *child) {

    if (parent && child) {
        if (!parent->children) parent->children = dlist_init (game_object_destroy_dummy, game_object_comparator);
        dlist_insert_after (parent->children, dlist_end (parent->children), child);
    }

}

GameObject *game_object_remove_child (GameObject *parent, GameObject *child) {

    if (parent && child) {
        if (parent->children) { 
            GameObject *go = NULL;
            for (ListElement *le = dlist_start (parent->children); le; le = le->next) {
                go = (GameObject *) le->data;
                if (go->id == child->id) 
                    return (GameObject *) dlist_remove_element (parent->children, le);
            }
        }
    }

    return NULL;

}

GameObject *game_object_get_by_id (u32 id) { if (id <= curr_max_objs) return gameObjects[id]; }

// mark as inactive or reusable the game object
void game_object_destroy (GameObject *go) {

    if (go) {
        go->id = -1;
        go->update = NULL;

        if (go->name) free (go->name);
        if (go->tag) free (go->tag);

        if (go->children) free (go->children);

        // individually destroy each component
        transform_destroy ((Transform *) go->components[TRANSFORM_COMP]);
        graphics_destroy ((Graphics *) go->components[GRAPHICS_COMP]);
        animator_destroy ((Animator *) go->components[ANIMATOR_COMP]);

        // player_comp_delete ((Player *) go->components[PLAYER_COMP]);
    }

}

void game_object_destroy_ref (void *data) { game_object_destroy ((GameObject *) data); } 

static void game_object_delete (GameObject *go) {

    if (go) {
        go->update = NULL;

        // individually destroy each component
        transform_destroy ((Transform *) go->components[TRANSFORM_COMP]);
        graphics_destroy ((Graphics *) go->components[GRAPHICS_COMP]);
        animator_destroy ((Animator *) go->components[ANIMATOR_COMP]);

        // player_comp_delete ((Player *) go->components[PLAYER_COMP]);

        if (go->name) free (go->name);
        if (go->tag) free (go->tag);

        free (go);
    }

}

// clean up game objects
void game_object_destroy_all (void) {

    for (u32 i = 0; i < curr_max_objs; i++) 
        if (gameObjects[i])
            game_object_delete (gameObjects[i]);

    free (gameObjects);

}

// update every game object
void game_object_update_all (void) {
     
    for (u32 i = 0; i < curr_max_objs; i++) {
        if (gameObjects[i]->id != -1) {
            if (gameObjects[i]->update)
                gameObjects[i]->update (gameObjects[i]);
        }
    }

}

void *game_object_add_component (GameObject *go, GameComponent component) {

    void *retval = NULL;

    if (go) {
        switch (component) {
            case TRANSFORM_COMP: 
                retval = go->components[component] = transform_new (go->id); 
                break;
            case GRAPHICS_COMP: 
                retval = go->components[component] = graphics_new (go->id); 
                break;
            case ANIMATOR_COMP: 
                retval = go->components[component] = animator_new (go->id); 
                break;

            // case PLAYER_COMP: 
            //     retval = go->components[component] = player_comp_new (go->id); 
            //     go->update = player_update;
            //     break;
            // case ENEMY_COMP:
            //     retval = go->components[component] = enemy_create_comp (go->id);
            //     go->update = enemy_update;
            //     break;

            default: break;
        }
    }

    return retval;

}

void *game_object_get_component (GameObject *go, GameComponent component) {

    if (go) return go->components[component];

}

void game_object_remove_component (GameObject *go, GameComponent component) {

    if (go) {
        switch (component) {
            case TRANSFORM_COMP: 
                transform_destroy (go->components[component]); 
                break;
            case GRAPHICS_COMP: 
                graphics_destroy (go->components[component]);
                break;
            case ANIMATOR_COMP: 
                animator_destroy (go->components[component]);
                break;

            // case PLAYER_COMP: 
            //     player_comp_delete (go->components[component]);
            //     go->update = NULL;
            //     break;
            // case ENEMY_COMP:
            //     enemy_destroy_comp (go->components[component]);
            //     go->update = NULL;
            //     break;

            default: break;
        }
    }

}

#pragma endregion