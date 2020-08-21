#ifndef _CENGINE_GAME_OBJECT_H_
#define _CENGINE_GAME_OBJECT_H_

#include "cengine/types/types.h"
#include "cengine/types/string.h"

#include "cengine/collections/dlist.h"

#include "cengine/config.h"
#include "cengine/renderer.h"

#include "cengine/game/components/graphics.h"
#include "cengine/game/components/transform.h"

#define DEFAULT_MAX_GOS     200

#define COMP_COUNT          4

typedef struct GameObject {
    
    i32 id;

    String *name;
    String *tag;

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
CENGINE_PRIVATE u8 game_objects_init_all (void);

// creates a new GameObject, and you can give it a friendly name and add it directly to a tag
CENGINE_PUBLIC GameObject *game_object_new (const char *name, const char *tag);

// destroys a GameObject when you not longer need it
CENGINE_PUBLIC void game_object_destroy (GameObject *go);

// destroys a reference to a GameObject
CENGINE_PUBLIC void game_object_destroy_ref (void *data);

// this is used to avoid go destruction when destroying go's children
CENGINE_PUBLIC void game_object_destroy_dummy (void *ptr);

// compares two GameObjects by their ids
CENGINE_PUBLIC int game_object_comparator (const void *one, const void *two);

// adds a new child to the GameObject
CENGINE_PUBLIC void game_object_add_child (GameObject *parent, GameObject *child);

// removes a child from the GameObject
CENGINE_PUBLIC GameObject *game_object_remove_child (GameObject *parent, GameObject *child);

// gets a GameObject by its id
CENGINE_PUBLIC GameObject *game_object_get_by_id (u32 id);

// clean up game objects
CENGINE_PRIVATE void game_object_destroy_all (void);

// update every game object
CENGINE_PUBLIC void game_object_update_all (void);

// sets a new name for the GameObject
CENGINE_PUBLIC void game_object_set_name (GameObject *go, const char *name);

// sets the tag for the gameobject
// removes it from the one it is now and adds it to the new one
// returns 0 on success, 1 on error
CENGINE_PUBLIC int game_object_set_tag (GameObject *go, const char *tag_name);

// sets the render layer of the game object
// removes it from the one it is now and adds it to the new one
// returns 0 on success, 1 on error
CENGINE_PUBLIC int game_object_set_layer (GameObject *go, const char *layer_name);

/*** Tags ***/

typedef struct GameObjectTag {

    String *name;
    DoubleList *gos;

} GameObjectTag;

// create a new go tag with that name
CENGINE_EXPORT void game_object_tag_create (const char *name);

// gets the matching tag by its name
CENGINE_EXPORT GameObjectTag *game_object_tag_get_by_name (const char *tag_name);

// adds a game object to a tag, returns 0 on success, 1 on error
CENGINE_EXPORT int game_object_add_to_tag (GameObject *go, const char *tag_name);

// removes a game object from a tag, returns 0 on success, 1 on error
CENGINE_EXPORT int game_object_remove_from_tag (GameObject *go, const char *tag_name);

/*** Components ***/

typedef enum GameComponent {

    TRANSFORM_COMP = 0,
    GRAPHICS_COMP,
    ANIMATOR_COMP,
    BOX_COLLIDER_COMP

} GameComponent;

// adds a new component to the GameObject
CENGINE_EXPORT void *game_object_add_component (GameObject *go, GameComponent component);

// gets the matching component from the GameObject, returns NULL if no match
CENGINE_EXPORT void *game_object_get_component (GameObject *go, GameComponent component);

// removes the component from the GameObject
CENGINE_EXPORT void game_object_remove_component (GameObject *go, GameComponent component);

/*** User defined componets ***/

typedef struct UserComponent {

    String *name;
    void *component;
    void *(*add)(u32);
    void (*remove)(void *);
    void (*update)(void *);

} UserComponent;

// creates a new user component
CENGINE_EXPORT UserComponent *user_component_new (const char *name, 
    void *(*add)(u32), void (*remove)(void *), void (*update)(void *));

// deletes the user component when you are doing using it
CENGINE_EXPORT void user_component_delete (void *ptr);

// registers a new user component, returns 0 on success, 1 on error
CENGINE_EXPORT int user_component_register (UserComponent *user_comp);

// returns the component inside the user component for quick access
CENGINE_EXPORT void *game_object_add_user_component (GameObject *go, const char *component_name);

// gets the matching user component from the GameObject
CENGINE_EXPORT void *game_object_get_user_component (GameObject *go, const char *name);

// removes the matching user component from the GameObject
CENGINE_EXPORT void game_object_user_component_remove (GameObject *go, const char *name);

#endif