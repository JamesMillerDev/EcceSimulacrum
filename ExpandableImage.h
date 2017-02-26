#ifndef EXPANDABLE_IMAGE
#define EXPANDABLE_IMAGE

#include "Computer.h"

struct ExpandableImage : public ScreenElement
{
	float accstep;
	float speed;
	float height;
	float duration;
	int speed_sign;
	float distance_traversed;
	bool animating;
	bool camera;
	int accframes, sumframes;
	ExpandableImage() : ScreenElement(0, 0, 0, 0, ""), animating(false) {}
	ExpandableImage(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, float _height, float _duration = 0.5, bool _camera = false) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), height(_height), duration(_duration), camera(_camera)
	{
		partial = true;
		animating = false;
		accframes = 10;
		sumframes = 55;
		speed_sign = 1;
	}

	void draw(TextureManager* texture_manager)
	{
		ScreenElement::draw(texture_manager);
	}

	void toggle()
	{
		if (animating)
			return;

		speed = 0;
		animating = true;
		accstep = height / (2 * sumframes + (60.0*duration - 2 * accframes)*accframes);
		distance_traversed = 0.0;
		speed_sign *= -1;
	}

	void animate()
	{
		if (!animating)
			return;

		y1 += speed_sign * speed;
		distance_traversed += abs(speed);
		if (height - distance_traversed < sumframes * accstep)
			speed -= accstep;

		else if (speed < accframes * accstep)
			speed += accstep;

		if (speed <= 0)
		{
			(speed_sign == -1 && !camera) ? y1 = y2 - height : y1 = y2;
			animating = false;
			return;
		}
	}
};

#endif