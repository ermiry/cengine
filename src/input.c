// #include <stdbool.h>

// #include "blackrock.h"

// #include "game/game.h"
// #include "game/player.h"
// #include "game/item.h"

// #include "collections/dlist.h"

// #include "ui/ui.h"
// #include "ui/gameUI.h"

// /*** MISC ***/

// bool typing = false;

// void input_start_typing (void) {

//     SDL_StartTextInput ();
//     typing = true;

// }

// void input_stop_typing (void) {

//     SDL_StopTextInput ();
//     typing = false;

// }

// /*** MAIN MENU ***/

// extern bool running;

// #pragma region MAIN MENU 

// #include "ui/menu.h"

// extern TextBox **selected_textBox;

// extern TextBox **loginTextBoxes;
// extern u8 login_textboxes_idx;

// extern Button *submitButton;

// void hanldeMenuEvent (UIScreen *activeScreen, SDL_Event event) {

//     if (event.type == SDL_KEYDOWN) {
//         SDL_Keycode key = event.key.keysym.sym;

//         switch (key) {
//             case SDLK_DOWN:
//                 if (activeMenuView == LOGIN_VIEW) {
//                     if (login_textboxes_idx < 3) {
//                         loginTextBoxes[login_textboxes_idx]->bgcolor = WHITE;
//                         login_textboxes_idx++;
//                         loginTextBoxes[login_textboxes_idx]->bgcolor = SILVER;
//                         selected_textBox = &loginTextBoxes[login_textboxes_idx];
//                     }

//                     else if (login_textboxes_idx == 3) {
//                         submitButton->bgcolor = SILVER;
//                         loginTextBoxes[login_textboxes_idx]->bgcolor = WHITE;
//                         login_textboxes_idx++;
//                     }
//                 }
//             break;
//             case SDLK_UP:
//                 if (activeMenuView == LOGIN_VIEW) {
//                     if (login_textboxes_idx > 0) {
//                         if (login_textboxes_idx == 4) {
//                             login_textboxes_idx--;
//                             loginTextBoxes[login_textboxes_idx]->bgcolor = SILVER;
//                             submitButton->bgcolor = WHITE;
//                         }

//                         else {
//                             loginTextBoxes[login_textboxes_idx]->bgcolor = WHITE;
//                             login_textboxes_idx--;
//                             loginTextBoxes[login_textboxes_idx]->bgcolor = SILVER;
//                             selected_textBox = &loginTextBoxes[login_textboxes_idx];
//                         }
//                     }
//                 }
//             break;

//             case SDLK_BACKSPACE: 
//                 if (typing) ui_textbox_delete_text (*selected_textBox);
//             break;

//             case SDLK_RETURN: 
//             case SDLK_RETURN2:
//                 if (activeMenuView == LOGIN_VIEW) 
//                     if (login_textboxes_idx == 4) 
//                         submitButton->event (getBlackCredentials ());
            
//             break;

//             case SDLK_c: 
//                 if (activeMenuView == MULTI_MENU_VIEW)
//                     pthread_create_detachable ((void *) multiplayer_createLobby, NULL); 
//             break;

//             case SDLK_p: if (activeMenuView == LAUNCH_VIEW) createMainMenu (); break;
//             case SDLK_m: if (activeMenuView == MAIN_MENU_VIEW) toggleMultiplayerMenu (); break;
//             case SDLK_b: if (activeMenuView == MULTI_MENU_VIEW) toggleMultiplayerMenu (); break;

//             case SDLK_j: 
//                 if (activeMenuView == MULTI_MENU_VIEW)
//                     pthread_create_detachable ((void *) multiplayer_joinLobby, NULL); 
//             break;
            
//             case SDLK_s: if (activeMenuView == MAIN_MENU_VIEW) startGame (); break;
//             // case SDLK_c: break;     // TODO: toggle credits window
//             case SDLK_e: if (activeMenuView == LAUNCH_VIEW) running = false; break;

//             #ifdef CLIENT_DEBUG 
//                 case SDLK_t: client_makeTestRequest (player_client, main_connection); break;
//             #endif

//             default: break;
//         }

//     }

// }

// #pragma endregion

// /*** GAME ***/

// #pragma region GAME

// void resolveCombat (Position newPos) {

//     // check what is blocking the movement
//     DoubleList *blockers = getObjectsAtPos (newPos.x, newPos.y);

//     if (blockers == NULL || dlist_size (blockers) <= 0) {
//         free (blockers);
//         return;
//     }

//     for (ListElement *e = dlist_start (blockers); e != NULL; e = e->next) {
//         Combat *c = (Combat *) getComponent ((GameObject *) e->data, COMBAT);
//         if (c != NULL) {
//             fight (main_player->combat, c, true);
//             break;
//         }
//     }

//     dlist_clean (blockers);

// }

// Position *playerPos = NULL;

// void move (u8 newX, u8 newY) {

//     Position newPos = { .x = newX, .y = newY };
//     if (canMove (newPos, true)) {
//         recalculateFov = true;
//         playerPos->x = newX;
//         playerPos->y = newY;
//     } 
//     else resolveCombat (newPos);
//     playerTookTurn = true; 

// }

// /*** GAME UI ***/

// void moveInInventory (u8 newX, u8 newY) {

//     if (newX >= 0 && newX <= 6) inventoryXIdx = newX;
//     if (newY >= 0 && newY < 3) inventoryYIdx = newY;

// }

// void moveInLoot (u8 newY) { if (newY >= 0 && (newY < dlist_size (activeLootRects))) lootYIdx = newY; }

// void moveInCharacter (u8 newX, u8 newY, bool moveRight) {

//     if (newX >= 0 && newX < 2) characterXIdx = newX;
//     if (newY >= 0 && newY < 6) characterYIdx = newY;

// }

// bool isInUI (void) {

//     if (lootView != NULL || inventoryView != NULL || characterView != NULL) return true;
//     else return false;

// }

// // FIXME: tooltip logic
// void closeUIMenu (void) {

//     if (tooltipView != NULL) {
//         if (lootView != NULL) toggleTooltip (0);
//         else toggleTooltip (1);
//     } 

//     if (lootView != NULL) toggleLootWindow ();

//     if (inventoryView != NULL) toggleInventory ();

//     if (characterView != NULL) toggleCharacter ();

// }

// /*** NAVIGATION BETWEEN MENUS ***/

// // when the game inits, the active view is the map
// UIView *activeView = NULL;

// // As of 02/09/2018 -- 01:25 -- we can only switch views between the inventory and character
// void swicthView (void) {

//     // switch the views in the list
//     if ((characterView != NULL) && (inventoryView != NULL) || lootView != NULL) {
//         void *prev = dlist_remove_element (activeScene->views, (dlist_end (activeScene->views))->prev);
//         void *last = dlist_remove_element (activeScene->views, (dlist_end (activeScene->views)));

//         dlist_insert_after (activeScene->views, dlist_end (activeScene->views), last);
//         dlist_insert_after (activeScene->views, dlist_end (activeScene->views), prev);

//         activeView = (UIView *) (dlist_end (activeScene->views))->data;
//     }

// }

// /*** GAME EVENTS ***/

// extern void gameOver (void);

// // FIXME:
// void triggerEvent (void) {

//     if (activeView == inventoryView) {
//         Item *item = getInvSelectedItem ();
//         if (item) {
//             if (item->callback != NULL) item->callback (item);

//         } 
//     } 

//     else if (activeView == characterView) {
//         Item *item = getCharSelectedItem ();
//         if (item) {
//             if (item->callback != NULL) item->callback (item);
//         }
//     }

//     // loop through all of our surrounding items in search for 
//     // an event listener to trigger
//     else if (activeView == mapView) {
//         DoubleList *gos = getObjectsAtPos (playerPos->x, playerPos->y);
//         if (gos != NULL) {
//             Event *ev = NULL;
//             for (ListElement *e = dlist_start (gos); e != NULL; e = e ->next) {
//                 ev = (Event *) getComponent ((GameObject *) e->data, EVENT);
//                 // trigger just the first event we find
//                 if (ev != NULL) {
//                     ev->callback (e->data);
//                     break;
//                 }
//             }

//             if (dlist_size (gos) > 0) dlist_clean (gos);
//             else free (gos);
//         }
//     }
            
// }

// void hanldeGameEvent (UIScreen *activeScreen, SDL_Event event) {

//     playerPos = main_player->pos;

//     if (event.type == SDL_KEYDOWN) {
//         SDL_Keycode key = event.key.keysym.sym;

//         switch (key) {
//             case SDLK_w:
//                 if (activeView == mapView) move (playerPos->x, playerPos->y - 1);
//                 else if (activeView == lootView) moveInLoot (lootYIdx - 1);
//                 else if (activeView == inventoryView) moveInInventory (inventoryXIdx, inventoryYIdx - 1);
//                 else if (activeView == characterView) moveInCharacter (characterXIdx, characterYIdx - 1, false);
//                 break;
//             case SDLK_s: 
//                 if (activeView == mapView) move (playerPos->x, playerPos->y + 1);
//                 else if (activeView == lootView) moveInLoot (lootYIdx + 1);
//                 else if (activeView == inventoryView) moveInInventory (inventoryXIdx, inventoryYIdx + 1);
//                 else if (activeView == characterView) moveInCharacter (characterXIdx, characterYIdx + 1, false);
//                 break;
//             case SDLK_a: 
//                 if (activeView == mapView) move (playerPos->x - 1, playerPos->y);
//                 else if (activeView == inventoryView) moveInInventory (inventoryXIdx - 1, inventoryYIdx);
//                 else if (activeView == characterView) moveInCharacter (characterXIdx - 1, characterYIdx, false);
//                 break;
//             case SDLK_d:
//                 if (activeView == mapView) move (playerPos->x + 1, playerPos->y);
//                 else if (activeView == inventoryView) moveInInventory (inventoryXIdx + 1, inventoryYIdx);
//                 else if (activeView == characterView) moveInCharacter (characterXIdx + 1, characterYIdx, true);
//                 break;

//             case SDLK_e: triggerEvent (); break;
//             case SDLK_g:
//                 if (activeView == lootView) getLootItem (lootYIdx);
//                 else if (activeView == mapView) getItem (); 
//                 break;
//             case SDLK_c: 
//                 if (activeView == lootView) collectGold (); 
//                 else toggleCharacter ();
//                 break;

//             // drop item
//             case SDLK_SPACE:
//                 if (activeView == inventoryView) {
//                     Item *item = getInvSelectedItem ();
//                     if (item != NULL) dropItem (item);
//                 } 
//                 break;

//             case SDLK_i: toggleInventory (); break;

//             case SDLK_p: togglePauseMenu (); break;

//             // switch between the open windows
//             case SDLK_TAB: swicthView (); break;

//             // toggle tooltip
//             case SDLK_LSHIFT: 
//                 if (activeView == lootView) toggleTooltip (0);
//                 else if (activeView == inventoryView) toggleTooltip (1);
//                 else if (activeView == characterView) toggleTooltip (2);
//                 break;

//             case SDLK_ESCAPE: closeUIMenu (); break;

//             // FIXME:
//             // quit to main menu
//             // case SDLK_q: if (activeView == scoreScreen) returnToMainMenu (); break;

//             // FIXME: this is only for testing!!
//             case SDLK_k: gameOver (); break;

//             default: break;
//         }
//     }

// }

// #pragma endregion

// /*** POST GAME ***/

// #pragma region POST GAME

// void handlePostGameEvent (UIScreen *activeScreen, SDL_Event event) {

//     if (event.type == SDL_KEYDOWN) {
//         SDL_Keycode key = event.key.keysym.sym;

//         switch (key) {
//             case SDLK_c: if (postGameScene->activeView == deathScreen) showScore (); break;
//             case SDLK_r: if (postGameScene->activeView == scoreScreen) retry (); break;
//             case SDLK_l: 
//                 if (postGameScene->activeView == scoreScreen) toggleLeaderBoards (); 
//                 break;

//             // FIXME:
//             // case SDLK_q: returnToMainMenu (); break;
//         }
//     }

// }

// #pragma endregion

/*** NEW INPUT ***/

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "blackrock.h"

#include "game/vector2d.h" 

#include "cengine/input.h"

extern void quit (void);

/*** Mouse ***/

Vector2D mousePos = { 0, 0 };

bool mouseButtonStates[N_MOUSE_BUTTONS];

bool input_get_mouse_button_state (MouseButton button) { return mouseButtonStates[button]; }

void input_on_mouse_button_down (SDL_Event event) {

    switch (event.button.button) {
        case SDL_BUTTON_LEFT: mouseButtonStates[MOUSE_LEFT] = true; break;
        case SDL_BUTTON_MIDDLE: mouseButtonStates[MOUSE_MIDDLE] = true; break;
        case SDL_BUTTON_RIGHT: mouseButtonStates[MOUSE_RIGHT] = true; break;

        default: break;
    }

}

void input_on_mouse_button_up (SDL_Event event) {

    switch (event.button.button) {
        case SDL_BUTTON_LEFT: mouseButtonStates[MOUSE_LEFT] = false; break;
        case SDL_BUTTON_MIDDLE: mouseButtonStates[MOUSE_MIDDLE] = false; break;
        case SDL_BUTTON_RIGHT: mouseButtonStates[MOUSE_RIGHT] = false; break;

        default: break;
    }

}

/*** KEYBOARD ***/

const u8 *keys_states = NULL;

void input_key_down (void) { keys_states = SDL_GetKeyboardState (NULL); }

void input_key_up (void) { keys_states = SDL_GetKeyboardState (NULL); }

bool input_is_key_down (const SDL_Scancode key) { 
    
    if (keys_states) if (keys_states[key]) return true;
    return false;
    
}

/*** INPUT ***/

void input_init (void) {

    for (u8 i = 0; i < N_MOUSE_BUTTONS; i++) mouseButtonStates[i] = false;

}

void input_handle (SDL_Event event) {

    while (SDL_PollEvent (&event)) {
        switch (event.type) {
            case SDL_QUIT: quit (); break;

            case SDL_MOUSEMOTION: 
                mousePos.x = event.motion.x;
                mousePos.y = event.motion.y;
                break;

            case SDL_MOUSEBUTTONDOWN: input_on_mouse_button_down (event); break;
            case SDL_MOUSEBUTTONUP: input_on_mouse_button_up (event); break;

            case SDL_KEYDOWN: input_key_down (); break;
            case SDL_KEYUP: input_key_up (); break;

            default: break;
        }
    }

}