#ifndef PAUSE_MENU
#define PAUSE_MENU

#include "Computer.h"

struct PauseMenu
{
	vector<unique_ptr<ScreenElement>> screen_elements;
	int prev_mouse_x, prev_mouse_y, width, height;
	TextureManager* texture_manager;
	bool pause_done;
	void window_resized(int new_width, int new_height);
	void setup_view();
	void draw();
	void mouse_moved(int x, int y);
	void mouse_clicked(int button, int state, int x, int y);
	PauseMenu(TextureManager* _texture_manager);
};

#endif