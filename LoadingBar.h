#ifndef LOADING_BAR
#define LOADING_BAR

#include "Computer.h"

struct LoadingBar : public ScreenElement
{
	int frames, position, pos_inc;
	LoadingBar(float _x1, float _y1, float _x2, float _y2, string _name, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application)
	{
		frames = 0;
		position = 0;
		pos_inc = 1;
	}

	void animate()
	{
		frames++;
		if (frames % 2 == 0)
		{
			position += pos_inc;
			if (position >= 24)
				pos_inc = -1;

			else if (position <= 0)
				pos_inc = 1;
		}
	}

	void draw(TextureManager* texture_manager)
	{
		ScreenElement::draw(texture_manager);
		for (int i = position; i >= position - 5; --i)
		{
			int b1 = x1 + i * 20 + 3 + 2;
			int b2 = b1 + 12;
			if (b1 < x1 + 3 || b2 > x2 - 3)
				continue;

			ScreenElement bar(b1, y1 + 5, b2, y2 - 5, "loadingbar.png");
			bar.draw(texture_manager);
		}
	}
};

#endif