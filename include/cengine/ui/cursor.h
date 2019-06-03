#ifndef _CENGINE_UI_CURSOR_H_
#define _CENGINE_UI_CURSOR_H_

#define CURSOR_DEFAULT_WIDTH            8
#define CURSOR_DEFAULT_HEIGHT           8
#define CURSOR_DEFAULT_HOT_X            4
#define CURSOR_DEFAULT_HOT_Y            4

typedef struct Cursor {

    SDL_Cursor *cursor; 
    int w, h;
    int hot_x, hot_y;
    
    Sprite *sprite;
    SpriteSheet *sprite_sheet;
    bool refSprite;

    u32 x_sprite_offset, y_sprite_offset;
    bool multipleSprites;
    Flip flip;

} Cursor;

extern Cursor *main_cursor;

extern Cursor *ui_cursor_new (void);
extern void ui_cursor_delete (Cursor *cursor);

extern void ui_cursor_draw (Cursor *cursor);

// update cursor with new values
extern int ui_cursor_update (Cursor *c, int w, int h, int hot_x, int hot_y);

/*** Getters & Setters ***/

extern int ui_cursor_get_width (Cursor *c);

extern int ui_cursor_set_width (Cursor *c, int new_width);

extern int ui_cursor_get_height (Cursor *c);

extern int ui_cursor_set_height (Cursor *c, int new_height);

extern int ui_cursor_get_hot_x (Cursor *c);

extern int ui_cursor_get_hot_y (Cursor *c);

#endif