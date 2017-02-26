#ifndef X_BUTTON
#define X_BUTTON

#include "Computer.h"

struct XButton : public ScreenElement
{
	Computer* parent;
	XButton(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application): ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent) {}
	void mouse_clicked(int button, int state, int x, int y);
};

#endif