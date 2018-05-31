#ifndef SLIDING_TILE
#define SLIDING_TILE

#include "Computer.h"

struct SlidingTile : public ScreenElement
{
	static vector<vector<int>> board;
	static ScreenElement* win_label;
	static bool win_locked;
	int number, pos_x, pos_y;
	SlidingTile(float _x1, float _y1, float _x2, float _y2, string _name, int _number, int _pos_x, int _pos_y, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), 
		number(_number), pos_x(_pos_x), pos_y(_pos_y) {}
	int get_number(int x, int y);
	void mouse_clicked(int button, int state, int x, int y);
};

#endif