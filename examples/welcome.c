#include "cengine/cengine.h"
#include "cengine/assets.h"
#include "cengine/manager/manager.h"

#include "cengine/utils/log.h"

static SDL_Surface *icon_surface = NULL;

// this method will be called when the SDL_QUIT event is detected 
// (when you close the window for example)
static void app_quit (void) { running = false; }

static int app_init () {

	unsigned int errors = 0;
	unsigned int retval = 0;

	// sets the function to be executed on SDL_QUIT event
	cengine_set_quit (app_quit);

	// set the directory to look for assets
	cengine_assets_set_path ("./assets");

	// init cengine capabilities
	retval = cengine_init ();
	if (retval) cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to init cengine!");
	errors |= retval;

	// create our main renderer and window
	WindowSize window_size = { 1920, 1080 };
	// we give our renderer the name "main" and that is how we will be able to get it at any place in the application
	Renderer *main_renderer = renderer_create_with_window ("main", 0, SDL_RENDERER_SOFTWARE | SDL_RENDERER_ACCELERATED,
		"Cengine", window_size, 0);

	// FIXME: load cengine logo
	// icon_surface = surface_load_image ("./assets/cengine-128.png");
	// if (icon_surface) window_set_icon (main_renderer->window, icon_surface);
	// else cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to load icon!");

	// we load our main font that will be used for the ui elements
	Font *main_font = ui_font_create ("roboto", "./assets/fonts/Roboto-Regular.ttf");
	if (main_font) {
		ui_font_set_sizes (main_font, 2, 24, 32);
		retval = ui_font_load (main_font, main_renderer, TTF_STYLE_NORMAL);
		errors |= retval;
	}

	else {
		cengine_log_msg (stderr, LOG_ERROR, LOG_NO_TYPE, "Failed to create main font!");
		errors |= 1;
	}

	return errors;

}

// this method will be called at the end of the application
static int app_end (void) {

	(void) cengine_end ();

	cengine_log_msg (stdout, LOG_SUCCESS, LOG_NO_TYPE, "Done!");

	return 0;

}

#pragma region app state

#include "cengine/ui/ui.h"
#include "cengine/ui/panel.h"
#include "cengine/ui/textbox.h"

static State *app_state = NULL;

static Panel *background_panel = NULL;
static TextBox *title_text = NULL;

static void app_update (void) {

	// game_object_update_all ();

}

// this method is called when we enter the "app" state
static void app_on_enter (void) { 

	app_state->update = app_update;

	// get our renderer
	// this only works because we set the renderer to be named like that
	Renderer *main_renderer = renderer_get_by_name ("main");

	// get our default font (the first one we added)
	Font *font = ui_font_get_default ();

	u32 screen_width = main_renderer->window->window_size.width;
	u32 screen_height = main_renderer->window->window_size.height;

	// set a panel as the background element
	RGBA_Color electromagnetic = { 47, 54, 64, 255 };
	background_panel = ui_panel_create (0, 0, screen_width, screen_height, UI_POS_MIDDLE_CENTER, main_renderer);
	ui_panel_set_bg_colour (background_panel, main_renderer, electromagnetic);
	ui_element_set_layer (main_renderer->ui, background_panel->ui_element, "back");

	// add a text title at the middle of the screen
	title_text = ui_textbox_create (0, 0, 200, 50, UI_POS_MIDDLE_CENTER, main_renderer);
	ui_textbox_set_text (title_text, main_renderer, "Welcome to Cengine!", font, 32, RGBA_WHITE, false);
	ui_textbox_set_text_pos (title_text, UI_POS_MIDDLE_CENTER);
	ui_element_set_layer (main_renderer->ui, title_text->ui_element, "top");

}

// this method will be called when we exit the "app" state
// in this particular example this will not happen until we 
// exit the application
static void app_on_exit (void) { 

	if (background_panel) ui_element_destroy (background_panel->ui_element);
	if (title_text) ui_element_destroy (title_text->ui_element);

}

// we create our "app" state (our only one for this application)
static State *app_state_new (void) {

	State *new_app_state = (State *) malloc (sizeof (State));
	if (new_app_state) {
		// new_game_state->state = IN_GAME;

		new_app_state->update = NULL;

		new_app_state->on_enter = app_on_enter;
		new_app_state->on_exit = app_on_exit;
	}

}

#pragma endregion

int main (void) {

	// first we init everything related with our own application
	if (!app_init ()) {
		// if that succeded, we can now create a base app state and state manager
		app_state = app_state_new ();
		manager = manager_new (app_state);

		// we are ready to start cengine
		// cengine will run in two threads,   
		// the main one is used for rendering and input
		// and the second one, is used for updates (like GameObjects)
		cengine_start (30);
	}

	return app_end ();

}