#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "cengine/types/types.h"
#include "cengine/types/string.h"
#include "cengine/game/go.h"
#include "cengine/animation.h"

#include "cengine/game/components/graphics.h"
#include "cengine/game/components/transform.h"
#include "cengine/game/components/collider.h"

GameObject **gameObjects = NULL;
u32 max_gos = 0;
u32 curr_max_objs = 0;
u32 new_go_id = 0;

static DoubleList *user_components;        // user defined components

static bool game_objects_realloc (void) {

    u32 new_max_gos = curr_max_objs * 2;

    gameObjects = realloc (gameObjects, new_max_gos * sizeof (GameObject *));

    if (gameObjects) {
        max_gos = new_max_gos;
        return true;
    }

    return false;

}

/*** Tags ***/

static DoubleList *tags = NULL;

void game_object_destroy_dummy (void *ptr);
int game_object_comparator (void *one, void *two);

static GameObjectTag *game_object_tag_new (const char *name) {

    GameObjectTag *tag = (GameObjectTag *) malloc (sizeof (GameObjectTag));
    if (tag) {
        tag->name = str_new (name);
        tag->gos = dlist_init (game_object_destroy_dummy, game_object_comparator);
    }

    return tag;

}

static void game_object_tag_delete (void *ptr) {

    if (ptr) {
        GameObjectTag *tag = (GameObjectTag *) ptr;
        str_delete (tag->name);
        dlist_destroy (tag->gos);

        free (tag);
    }

}

// create a new go tag with that name
void game_object_tag_create (const char *name) {

    if (name) {
        GameObjectTag *tag = game_object_tag_new (name);
        dlist_insert_after (tags, dlist_end (tags), tag);
    }

}

GameObjectTag *game_object_tag_get_by_name (const char *tag_name) {

    GameObjectTag *retval = NULL;

    if (tag_name) {
        GameObjectTag *tag = NULL;
        for (ListElement *le = dlist_start (tags); le; le = le->next) {
            tag = (GameObjectTag *) le->data;
            if (!strcmp (tag_name, tag->name->str)) {
                retval = tag;
                break;
            }
        }
    }

    return retval;    

}

// adds a game object to a tag, returns 0 on success, 1 on error
int game_object_add_to_tag (GameObject *go, const char *tag_name) {

    int retval = 1;

    if (go && tag_name) {
        GameObjectTag *tag = game_object_tag_get_by_name (tag_name);
        if (tag) {
            dlist_insert_after (tag->gos, dlist_end (tag->gos), go);
            retval = 0;
        }
    }

    return retval;

}

// removes a game object from a tag, returns 0 on success, 1 on error
int game_object_remove_from_tag (GameObject *go, const char *tag_name) {

    int retval = 1;

    if (go && tag_name) {
        GameObjectTag *tag = game_object_tag_get_by_name (tag_name);
        if (tag) {
            // search the game object inside the tag
            ListElement *le = dlist_get_element (tag->gos, go);
            if (le) {
                void *data = dlist_remove_element (tag->gos, le);
                if (data) retval = 0;
                
            } 
        }
    }

    return retval;

}

void user_component_delete (void *ptr);

// init our game objects array
u8 game_objects_init_all (void) {

    u8 retval = 1;

    gameObjects = (GameObject **) calloc (DEFAULT_MAX_GOS, sizeof (GameObject *));
    if (gameObjects) {
        for (u32 i = 0; i < DEFAULT_MAX_GOS; i++) gameObjects[i] = NULL;

        max_gos = DEFAULT_MAX_GOS;
        curr_max_objs = 0;
        new_go_id = 0;

        tags = dlist_init (game_object_tag_delete, NULL);   // init gos tags

        // init user defined components list
        user_components = dlist_init (user_component_delete, NULL);

        retval = 0;
    }

    return retval;

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

        go->user_components = dlist_init (user_component_delete, NULL);

        // all game objects are added to the dafult layer when they are initialized
        layer_add_object ("default", go);
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
void game_object_destroy_dummy (void *ptr) {}

int game_object_comparator (void *one, void *two) {

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

        layer_remove_object (go->layer->name->str, go);

        // individually destroy each component
        transform_destroy ((Transform *) go->components[TRANSFORM_COMP]);
        graphics_destroy ((Graphics *) go->components[GRAPHICS_COMP]);
        animator_destroy ((Animator *) go->components[ANIMATOR_COMP]);

        // destroy user defined components
        dlist_destroy (go->user_components);
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

        // destroy user defined components
        dlist_destroy (go->user_components);

        // layer_remove_object (go->layer->name->str, go);

        if (go->name) free (go->name);
        if (go->tag) free (go->tag);

        free (go);
    }

}

// clean up game objects
void game_object_destroy_all (void) {

    // destroy gos tags
    dlist_destroy (tags);

    // destroy user defined components list
    dlist_destroy (user_components);

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

/*** Components ***/

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

            default: break;
        }
    }

}

/*** User defined componets ***/

// creates a new user component
UserComponent *user_component_new (const char *name, 
    void *(*add)(u32), void (*remove)(void *), void (*update)(void *)) {

    UserComponent *user_comp = (UserComponent *) malloc (sizeof (UserComponent));
    if (user_comp) {
        user_comp->name = str_new (name);
        user_comp->component = NULL;
        user_comp->add = add;
        user_comp->remove = remove;
        user_comp->update = update;
    }

    return user_comp;

}

void user_component_delete (void *ptr) {

    if (ptr) {
        UserComponent *user_comp = (UserComponent *) ptr;
        str_delete (user_comp->name);

        if (user_comp->component && user_comp->remove) 
            user_comp->remove (user_comp->component) ;

        free (user_comp);
    }

}

// registers a new user component, returns 0 on success, 1 on error
int user_component_register (UserComponent *user_comp) {

    int retval = 1;

    if (user_comp) {
        retval = dlist_insert_after (user_components, 
            dlist_end (user_components), user_comp) == true ? 0 : 1;
    }

    return retval;

}

// gets a user defined component by name
static UserComponent *user_component_get (DoubleList *components, const char *name) {

    UserComponent *user_comp = NULL;

    if (name) {
        for (ListElement *le = dlist_start (components); le; le = le->next) {
            user_comp = (UserComponent *) le->data;
            if (!strcmp (name, user_comp->name->str)) break;
        }
    }

    return user_comp;

}

// returns the component inside the user component for quick access
void *game_object_add_user_component (GameObject *go, const char *component_name) {

    void *retval = NULL;

    if (go && component_name) {
        // get the component by name
        UserComponent *user_comp = user_component_get (user_components, component_name);
        if (user_comp) {
            // add the component to the game object
            UserComponent *new_comp = user_component_new (user_comp->name->str, 
                user_comp->add, user_comp->remove, user_comp->update);
            retval = new_comp->component = new_comp->add (go->id);

            go->update = new_comp->update;

            // add the new user component to the game object
            dlist_insert_after (go->user_components, dlist_end (go->user_components), new_comp);
        }
    }

    return retval;

}

void *game_object_get_user_component (GameObject *go, const char *name) {

    if (go && name) return (user_component_get (go->user_components, name))->component;
        
}

void game_object_user_component_remove (GameObject *go, const char *name) {

    if (go && name) {
        ListElement *le = NULL;
        UserComponent *user_comp = NULL;
        for (le = dlist_start (user_components); le; le = le->next) {
            user_comp = (UserComponent *) le->data;
            if (!strcmp (name, user_comp->name->str)) {
                dlist_remove_element (go->user_components, le);
                user_component_delete (user_comp);
                break;
            }
        }
    }

}

/*** Layers ***/

int game_object_set_layer (GameObject *go, const char *layer) {

    return layer_add_object (layer, go);

}