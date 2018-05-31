#ifndef ANIMATION
#define ANIMATION

#include "Computer.h"

struct Animation : public ScreenElement
{
	bool up, jump;
	int frame, total;
	string base_name;
	Animation(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, int _total, bool _jump = false) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), up(true), frame(1), total(_total), base_name(_name), jump(_jump)
	{
		name = base_name + to_string(frame) + ".png";
	}

	void animate()
	{
		if (up)
		{
			frame++;
			if (frame >= total)
			{
				if (jump)
					frame = 1;
				
				else up = false;
			}
		}

		else
		{
			frame--;
			if (frame <= 1)
				up = true;
		}

		name = base_name + to_string(frame) + ".png";
	}
};

#endif