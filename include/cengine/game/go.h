#ifndef _CENGINE_GAME_OBJECT_H_
#define _CENGINE_GAME_OBJECT_H_

#define DEFAULT_MAX_GOS     200

#define COMP_COUNT      4

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/renderer.h"

#include "cengine/game/components/graphics.h"
#include "cengine/game/components/transform.h"

#include "cengine/collections/dlist.h"

typedef struct GameObject {
    
    i32 id;

    char *name;
    char *tag;      // FIXME: change this when adding to tags!!

    Layer *layer;

    void *components[COMP_COUNT];
    DoubleList *user_components;

    DoubleList *children;

    void (*update)(void *data);

} GameObject;

extern GameObject **gameObjects;
extern u32 max_gos;
extern u32 curr_max_objs;
extern u32 new_go_id;

// init our game objects array
extern u8 game_objects_init_all (void);

extern GameObject *game_object_new (const char *name, const char *tag);
extern void game_object_destroy (GameObject *go);
extern void game_object_destroy_ref (void *data);

// this is used to avoid go destruction when destroying go's children
extern void game_object_destroy_dummy (void *ptr);

extern int game_object_comparator (void *one, void *two);

extern void game_object_add_child (GameObject *parent, GameObject *child);
extern GameObject *game_object_remove_child (GameObject *parent, GameObject *child);

extern GameObject *game_object_get_by_id (u32 id);

// clean up game objects
extern void game_object_destroy_all (void);

// update every game object
extern void game_object_update_all (void);

// add a game object into a layer
extern int game_object_set_layer (GameObject *go, const char *layer);

/*** Tags ***/

typedef struct GameObjectTag {

    String *name;
    DoubleList *gos;

} GameObjectTag;

// create a new go tag with that name
extern void game_object_tag_create (const char *name);

extern GameObjectTag *game_object_tag_get_by_name (const char *tag_name);

// adds a game object to a tag, returns 0 on success, 1 on error
extern int game_object_add_to_tag (GameObject *go, const char *tag_name);
// removes a game object from a tag, returns 0 on success, 1 on error
extern int game_object_remove_from_tag (GameObject *go, const char *tag_name);

/*** Components ***/

typedef enum GameComponent {

    TRANSFORM_COMP = 0,
    GRAPHICS_COMP,
    ANIMATOR_COMP,
    BOX_COLLIDER_COMP

} GameComponent;

extern void *game_object_add_component (GameObject *go, GameComponent component);
extern void *game_object_get_component (GameObject *go, GameComponent component);
extern void game_object_remove_component (GameObject *go, GameComponent component);

/*** User defined componets ***/

typedef struct UserComponent {

    String *name;
    void *component;
    void *(*add)(u32);
    void (*remove)(void *);
    void (*update)(void *);

} UserComponent;

// creates a new user component
extern UserComponent *user_component_new (const char *name, 
    void *(*add)(u32), void (*remove)(void *), void (*update)(void *));
extern void user_component_delete (void *ptr);

// registers a new user component, returns 0 on success, 1 on error
extern int user_component_register (UserComponent *user_comp);

// returns the component inside the user component for quick access
extern void *game_object_add_user_component (GameObject *go, const char *component_name);

extern void *game_object_get_user_component (GameObject *go, const char *name);

extern void game_object_user_component_remove (GameObject *go, const char *name);

#endif