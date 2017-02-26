#ifndef CONSOLE_HEADER
#define CONSOLE_HEADER

#include "Computer.h"

struct Console : public ScreenElement
{
	Computer* parent;
	Console(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent) : ScreenElement(_x1, _y1, _x2, _y2, _name, CONSOLE), parent(_parent)
	{
		parent->char_grid.clear();
		print_to_grid("Type 'help' for more information.");
		print_to_grid("/>_");
	}

	void draw(TextureManager* texture_manager);
	void print_to_grid(string text);
	void give_focus();
	void take_focus();
	void press_key(unsigned char key);
	int grid_width = 48;
	int grid_height = 10;
	int input_row = 0;
};

#endif