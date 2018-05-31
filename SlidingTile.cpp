#include "SlidingTile.h"

vector<vector<int>> SlidingTile::board;
ScreenElement* SlidingTile::win_label;
bool SlidingTile::win_locked;

int SlidingTile::get_number(int x, int y)
{
	if (x < 0 || x > 2 || y < 0 || y > 2)
		return -1;

	return board[y][x];
}

void SlidingTile::mouse_clicked(int button, int state, int x, int y)
{
	if (get_number(pos_x, pos_y - 1) == 0)
	{
		board[pos_y - 1][pos_x] = number;
		board[pos_y][pos_x] = 0;
		pos_y -= 1;
		translate(0, 80);
	}

	else if (get_number(pos_x, pos_y + 1) == 0)
	{
		board[pos_y + 1][pos_x] = number;
		board[pos_y][pos_x] = 0;
		pos_y += 1;
		translate(0, -80);
	}

	else if (get_number(pos_x - 1, pos_y) == 0)
	{
		board[pos_y][pos_x - 1] = number;
		board[pos_y][pos_x] = 0;
		pos_x -= 1;
		translate(-80, 0);
	}

	else if (get_number(pos_x + 1, pos_y) == 0)
	{
		board[pos_y][pos_x + 1] = number;
		board[pos_y][pos_x] = 0;
		pos_x += 1;
		translate(80, 0);
	}

	if (get_number(0, 0) == 1 && get_number(1, 0) == 2 && get_number(2, 0) == 3 && get_number(0, 1) == 4 && get_number(1, 1) == 5 && get_number(2, 1) == 6 &&
		get_number(0, 2) == 7 && get_number(1, 2) == 8 && get_number(2, 2) == 0 && !win_locked)
		win_label->is_visible = true;
}