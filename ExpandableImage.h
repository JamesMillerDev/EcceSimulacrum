#ifndef EXPANDABLE_IMAGE
#define EXPANDABLE_IMAGE

#include "Computer.h"
#include "ScrollPanel.h"

struct ExpandableImage : public ScreenElement
{
	double accstep;
	double speed;
	double height;
	double old_height;
	double duration;
	double old_duration;
	int speed_sign;
	double distance_traversed;
	bool animating;
	bool camera;
	int accframes, sumframes;
	ScrollPanel* scroll_panel;
	ExpandableImage() : ScreenElement(0, 0, 0, 0, ""), animating(false) {}
	ExpandableImage(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, float _height, float _duration = 0.5, bool _camera = false) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), height(_height), duration(_duration), camera(_camera), scroll_panel(0)
	{
		partial = true;
		animating = false;
		accframes = 10;
		sumframes = 55;
		speed_sign = 1;
		old_height = height;
		old_duration = duration;
	}

	void draw(TextureManager* texture_manager)
	{
		ScreenElement::draw(texture_manager);
	}

	void reset()
	{
		y1 = y2;
		speed_sign = 1;
	}

	void toggle()
	{
		speed = 0;
		animating = true;
		speed_sign *= -1;
		if (speed_sign == -1)
			height = old_height - (y2 - y1);

		else height = y2 - y1;
		if (camera)
			height = old_height;

		accstep = height / (2 * sumframes + (60.0*duration - 2 * accframes)*accframes);
		distance_traversed = 0.0;
	}

	void animate()
	{
		if (!animating)
			return;

		if (scroll_panel != 0)
		{
			for (int i = 0; i < scroll_panel->children.size(); ++i)
			{
				if (scroll_panel->children[i]->y2 < y1)
					scroll_panel->children[i]->translate(0, speed_sign * speed);
			}

			scroll_panel->resize_page(-speed_sign * speed);
		}

		y1 += speed_sign * speed;
		if (!camera)
		{
			if (y1 >= y2 && speed != 0)
			{
				y1 = y2;
				animating = false;
				return;
			}

			if (y1 <= y2 - old_height && speed != 0)
			{
				y1 = y2 - old_height;
				animating = false;
				return;
			}
		}

		distance_traversed += abs(speed);
		if (height - distance_traversed < sumframes * accstep)
			speed -= accstep;

		else if (speed < accframes * accstep)
			speed += accstep;

		if (speed <= 0)
		{
			if (!camera)
				(speed_sign == -1 && !camera) ? y1 = y2 - old_height : y1 = y2;

			animating = false;
			return;
		}
	}
};

#endif