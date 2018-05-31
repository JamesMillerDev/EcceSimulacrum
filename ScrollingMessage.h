#ifndef SCROLLING_MESSAGE
#define SCROLLING_MESSAGE

#include "Computer.h"
#include "fonts.h"

struct ScrollingMessage : public ScreenElement
{
	int index;
	int end;
	int frames;
	ScrollingMessage(float _x1, float _y1, float _x2, float _y2, int _index) : ScreenElement(_x1, _y1, _x2, _y2, ""), index(_index), end(_index), frames(0) {}
	static string master_string;
	void animate()
	{
		frames++;
		if (frames % 30 == 0)
		{
			int spaces = 0;
			for (int i = index; i < end; ++i)
			{
				if (master_string[i] == ' ')
					spaces++;
			}

			if (spaces >= 25)
			{
				index++;
				end++;
			}

			else end++;
		}

		//TODO do this better
		if (end >= master_string.length())
		{
			int distance = end - index;
			index = 0;
			end = distance;
		}
	}

	void draw(TextureManager* texture_manager)
	{
		int cur_y = y2;
		int spaces = 0;
		int prev = index;
		for (int i = index; i < end; ++i)
		{
			if (master_string[i] == ' ')
				spaces++;

			if (spaces == 5)
			{
				draw_string(texture_manager, 32, master_string.substr(prev, i - prev), x1, cur_y);
				prev = i + 1;
				spaces = 0;
				cur_y -= 40;
			}
		}

		if (prev != end)
			draw_string(texture_manager, 32, master_string.substr(prev, end - prev), x1, cur_y);
	}
};

#endif