#ifndef CONSOLE_HEADER
#define CONSOLE_HEADER

#include "Computer.h"

struct Console : public ScreenElement
{
	Computer* parent;
	string current_directory;
	Console(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent) : ScreenElement(_x1, _y1, _x2, _y2, _name, CONSOLE), parent(_parent)
	{
		draw_cursor = true;
		frames = 1;
		parent->char_grid.clear();
		print_to_grid("Type 'help' for more information.");
		current_directory = "/";
		print_to_grid(current_directory + ">");
		//grid_width = (x2 - x1) / ((float)DENIAL_CHAR_WIDTH * 0.39);
		//grid_height = (y2 - y1) / ((float)DENIAL_CHAR_HEIGHT * 0.39);
		grid_width = (x2 - x1) / 9.0;
		grid_height = (y2 - y1) / ((404.0 - 390.0) + 4.0);
	}

	void draw(TextureManager* texture_manager);
	void print_to_grid(string text);
	void give_focus();
	void take_focus();
	void press_key(unsigned char key);
	void animate();
	int grid_width = 48;
	int grid_height = 10;
	int input_row = 0;
	bool draw_cursor;
	int frames;
};

#endif