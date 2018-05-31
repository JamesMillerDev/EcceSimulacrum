#ifndef FADING_OBJECT
#define FADING_OBJECT

#include "Computer.h"

struct FadingObject : public ScreenElement
{
	Computer* parent;
	FadingObject(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent) : ScreenElement(_x1, _y1, _x2, _y2, _name), parent(_parent) {}
	void animate()
	{
		alpha -= 0.02;
		if (alpha <= 0)
		{
			marked_for_deletion = true;
			parent->process_control_changes();
		}
	}
};

#endif