#ifndef DRAG_ELEMENT
#define DRAG_ELEMENT

#include "Computer.h"

//TODO this and scroll bar, what if you "click" with the mouse wheel...
struct DragElement : public ScreenElement
{
	int prev_mouse_x, prev_mouse_y;
	int offset = 0;
	bool being_dragged = false;
	Computer* parent;
	DragElement(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application = NONE, bool _cache_texture = false, bool _border_element = false) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application, _cache_texture, _border_element), parent(_parent) {}
	void mouse_clicked(int button, int state, int x, int y)
	{
		if (state == GLUT_UP)
		{
			being_dragged = false;
			return;
		}

		being_dragged = true;
		prev_mouse_x = x;
		prev_mouse_y = y;
		offset = y - y1;
	}

	void mouse_moved(int x, int y)
	{
		if (!being_dragged)
			return;

		//float delta_y = y <= 56 ? (y1 + offset - 56) : y - prev_mouse_y;
		float delta_y;
		if (y <= 56)
		{
			delta_y = -1 * (y1 + offset - 56);
		}

		else
		{
			delta_y = y - y1 - offset;
		}

		parent->translate_application(parent->screen_elements, x - prev_mouse_x, delta_y, application);
		prev_mouse_x = x;
		prev_mouse_y = y;
	}
};

#endif