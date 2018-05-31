#ifndef BACKGROUND_ANIMATION
#define BACKGROUND_ANIMATION

#include "Computer.h"

struct BackgroundAnimation : public ScreenElement
{
	int frame;
	string current_name;
	BackgroundAnimation(float _x1, float _y1, float _x2, float _y2, string _name) : ScreenElement(_x1, _y1, _x2, _y2, _name), frame(1) {}
	void animate()
	{
		frame++;
		if (frame > 120)
			frame = 1;

		string num = to_string(frame);
		string zeros = string(4 - num.size(), '0');
		current_name = zeros + num + ".png";
	}

	void draw(TextureManager* texture_manager)
	{
		string old_name = name;
		name = current_name;
		ScreenElement::draw(texture_manager);
		name = old_name;
	}
};

#endif