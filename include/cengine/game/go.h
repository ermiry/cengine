#ifndef _CENGINE_GAME_OBJECT_H_
#define _CENGINE_GAME_OBJECT_H_

#define DEFAULT_MAX_GOS     200

#define COMP_COUNT      7

#include "types/types.h"
#include "cengine/collections/dlist.h"

#include "cengine/sprites.h"
#include "cengine/types/vector2d.h"

// TODO: better tag management -> create a list for each tag?
typedef struct GameObject {
    
    i32 id;

    char *name;
    char *tag;
    void *components[COMP_COUNT];

    DoubleList *children;
    void (*update)(void *data);

} GameObject;

extern GameObject *game_object_new (const char *name, const char *tag);
extern void game_object_destroy (GameObject *go);
extern void game_object_destroy_ref (void *data);

extern void game_object_add_child (GameObject *parent, GameObject *child);
extern GameObject *game_object_remove_child (GameObject *parent, GameObject *child);

extern GameObject *game_object_get_by_id (u32 id);

// clean up game objects
extern void game_object_destroy_all (void);

// update every game object
extern void game_object_update_all (void);

/*** Components ***/

typedef enum GameComponent {

    TRANSFORM_COMP = 0,
    GRAPHICS_COMP,
    ANIMATOR_COMP,
    BOX_COLLIDER_COMP,

    PLAYER_COMP,
    ENEMY_COMP,
    ITEM_COMP,

} GameComponent;

extern void *game_object_add_component (GameObject *go, GameComponent component);
extern void *game_object_get_component (GameObject *go, GameComponent component);
extern void game_object_remove_component (GameObject *go, GameComponent component);

typedef struct Transform {

    u32 goID;
    Vector2D position;

} Transform;

typedef struct Position {

    u32 objectId;
    u8 x, y;
    u8 layer;   

} Position;

typedef enum Layer {

    UNSET_LAYER = 0,
    GROUND_LAYER = 1,
    LOWER_LAYER = 2,
    MID_LAYER = 3,
    TOP_LAYER = 4,

} Layer;

typedef enum Flip {

    NO_FLIP = 0x00000000,
    FLIP_HORIZONTAL = 0x00000001,
    FLIP_VERTICAL = 0x00000002

} Flip;

typedef struct Graphics {

    u32 goID;

    Sprite *sprite;
    SpriteSheet *spriteSheet;
    bool refSprite;

    u32 x_sprite_offset, y_sprite_offset;
    bool multipleSprites;
    Layer layer; 
    Flip flip;
    bool hasBeenSeen;
    bool visibleOutsideFov;

} Graphics;

extern void graphics_set_sprite (Graphics *graphics, const char *filename);
extern void graphics_set_sprite_sheet (Graphics *graphics, const char *filename);

extern void graphics_ref_sprite (Graphics *graphics, Sprite *sprite);
extern void graphics_ref_sprite_sheet (Graphics *graphics, SpriteSheet *spriteSheet);

#endif