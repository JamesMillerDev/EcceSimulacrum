#ifndef FALLING_LETTER
#define FALLING_LETTER

#include "Computer.h"
#include "fonts.h"

struct FallingLetter : public ScreenElement
{
	int sign;
	float rotation_angle;
	FallingLetter(float _x1, float _y1, float _x2, float _y2, string _name) : ScreenElement(_x1, _y1, _x2, _y2, _name)
	{
		sign = 2;
	}

	void draw(TextureManager* texture_manager)
	{
		draw_string(texture_manager, 32, name, x1, y1);
	}
};

#endif