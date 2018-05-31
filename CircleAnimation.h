#ifndef CIRCLE_ANIMATION
#define CIRCLE_ANIMATION

#include "Computer.h"

struct CircleAnimation : public ScreenElement
{
	Computer* parent;
	vector<ScreenElement> circles;
	CircleAnimation(float _x1, float _y1, float _x2, float _y2, Computer* _parent) : ScreenElement(_x1, _y1, _x2, _y2, ""), parent(_parent)
	{
		circles.push_back(ScreenElement(x1 - 10, 1080, x1 + 10, 1100, "lightcircle.png"));
		circles.push_back(ScreenElement(x1 - 7, 1090, x1 + 7, 1104, "lightcircle.png"));
		circles.push_back(ScreenElement(x1 - 4, 1102, x1 + 4, 1110, "lightcircle.png"));
	}

	int center_y(ScreenElement s)
	{
		return s.y1 + (s.y2 - s.y1) / 2;
	}

	void animate()
	{
		bool done = true;
		for (int i = 0; i < circles.size(); ++i)
		{
			if (center_y(circles[i]) > y1)
				circles[i].translate(0, -5);

			else
			{
				circles[i].x1 -= 5;
				circles[i].y1 -= 5;
				circles[i].x2 += 5;
				circles[i].y2 += 5;
				if (circles[i].alpha >= 0.03)
					circles[i].alpha -= 0.03;

				else circles[i].alpha = 0;
			}

			if (circles[i].alpha > 0.0)
				done = false;
		}

		if (done)
		{
			marked_for_deletion = true;
			parent->process_control_changes();
		}
	}

	void draw(TextureManager* texture_manager)
	{
		for (int i = 0; i < circles.size(); ++i)
			circles[i].draw(texture_manager);
	}
};

#endif