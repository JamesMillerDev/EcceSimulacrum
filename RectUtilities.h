#ifndef RECT_UTILITIES
#define RECT_UTILITIES

#include "Computer.h"

int rand_in_int(int start, int end)
{
	return (rand() % (end + 1 - start)) + start;
}

vector<Rect> divide_rect(Rect rect, int degree)
{
	vector<Rect> pieces;
	for (int i = 0; i < degree; ++i)
	{
		int index = -1;
		Rect cur_rect;
		vector<Rect> new_pieces;
		if (i == 0)
			cur_rect = rect;

		else
		{
			int area = 0;
			for (int j = 0; j < pieces.size(); ++j)
			{
				int new_area = (pieces[j].x2 - pieces[j].x1) * (pieces[j].y2 - pieces[j].y1);
				if (new_area > area)
				{
					index = j;
					cur_rect = pieces[j];
					area = new_area;
				}
			}
		}

		int mode = rand() % 3;
		if (mode == 0)
		{
			int x1 = rand_in_int(cur_rect.x1, cur_rect.x2);
			int x2 = rand_in_int(x1, cur_rect.x2);
			new_pieces.push_back(Rect(x1, cur_rect.y1, x2, cur_rect.y2));
			new_pieces.push_back(Rect(cur_rect.x1, cur_rect.y1, x1, cur_rect.y2));
			new_pieces.push_back(Rect(x2, cur_rect.y1, cur_rect.x2, cur_rect.y2));
		}

		else if (mode == 1)
		{
			int y1 = rand_in_int(cur_rect.y1, cur_rect.y2);
			int y2 = rand_in_int(y1, cur_rect.y2);
			new_pieces.push_back(Rect(cur_rect.x1, y1, cur_rect.x2, y2));
			new_pieces.push_back(Rect(cur_rect.x1, cur_rect.y1, cur_rect.x2, y1));
			new_pieces.push_back(Rect(cur_rect.x1, y2, cur_rect.x2, cur_rect.y2));
		}

		else if (mode == 2)
		{
			int x1 = rand_in_int(cur_rect.x1, cur_rect.x2);
			int x2 = rand_in_int(x1, cur_rect.x2);
			int y1 = rand_in_int(cur_rect.y1, cur_rect.y2);
			int y2 = rand_in_int(y1, cur_rect.y2);
			new_pieces.push_back(Rect(x1, y1, x2, y2));
			new_pieces.push_back(Rect(cur_rect.x1, cur_rect.y1, x1, y2));
			new_pieces.push_back(Rect(cur_rect.x1, y2, x2, cur_rect.y2));
			new_pieces.push_back(Rect(x2, y1, cur_rect.x2, cur_rect.y2));
			new_pieces.push_back(Rect(x1, cur_rect.y1, cur_rect.x2, y1));
		}

		vector<Rect> final_pieces;
		if (index == -1)
			pieces = new_pieces;

		else
		{
			for (int j = 0; j < pieces.size(); ++j)
			{
				if (j == index)
					final_pieces.insert(final_pieces.end(), new_pieces.begin(), new_pieces.end());

				else final_pieces.push_back(pieces[j]);
			}

			pieces = final_pieces;
		}
	}

	return pieces;
}

vector<Rect> heavy_scatter(vector<Rect> rects, int index, bool yaxis, bool up)
{
	Rect rect = rects[index];
	vector<Rect> new_pieces;
	if (yaxis)
	{
		for (int i = rect.y1; i < rect.y2; ++i)
		{
			Rect new_rect = Rect(rect.x1, i, rect.x2, i + 1);
			new_rect.xtrans = (i - rect.y1) * (up ? 1 : -1);
			new_rect.xtrans += rand() % 9 - 4;
			new_pieces.push_back(new_rect);
		}
	}

	else
	{
		for (int i = rect.x1; i < rect.x2; ++i)
		{
			Rect new_rect = Rect(i, rect.y1, i + 1, rect.y2);
			new_rect.ytrans = (i - rect.x1) * (up ? 1 : -1);
			new_rect.ytrans += rand() % 9 - 4;
			new_pieces.push_back(new_rect);
		}
	}

	vector<Rect> final_rects;
	for (int i = 0; i < rects.size(); ++i)
	{
		if (i == index)
			final_rects.insert(final_rects.end(), new_pieces.begin(), new_pieces.end());

		else final_rects.push_back(rects[i]);
	}

	return final_rects;
}

vector<Rect> translate_skew(vector<Rect> rects, int degree)
{
	for (int i = 0; i < rects.size(); ++i)
	{
		rects[i].xtrans += rand() % (degree * 4 * 2 + 1) - degree * 4;
		rects[i].ytrans += rand() % (degree * 4 * 2 + 1) - degree * 4;
	}

	return rects;
}

#endif