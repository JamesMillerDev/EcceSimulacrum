#ifndef EXPANDABLE_HEADER
#define EXPANDABLE_HEADER

#include "Computer.h"
#include "ExpandableImage.h"

struct ExpandableHeader : public ScreenElement
{
	int offset;
	bool going_right;
	bool have_mouse_over;
	ExpandableImage* image;
	ExpandableHeader(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, ExpandableImage* _image) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), image(_image)
	{
		offset = 0;
		going_right = false;
		have_mouse_over = false;
	}

	void mouse_over(int x, int y)
	{
		if (have_mouse_over)
			return;

		have_mouse_over = true;
		image->toggle();
		going_right = true;
	}

	void mouse_off()
	{
		if (!have_mouse_over)
			return;

		have_mouse_over = false;
		image->toggle();
		going_right = false;
	}

	void animate()
	{
		if (going_right)
		{
			x2 += 10;
			if (x2 >= 580)
				x2 = 580;
		}

		else
		{
			x2 -= 10;
			if (x2 <= 251)
				x2 = 251;
		}
	}

	void draw(TextureManager* texture_manager)
	{
		if (x2 > 251)
		{
			int offset = x2 - 251;
			ScreenElement body(offset, y1, offset + 251, y2, name);
			body.draw(texture_manager);
			ScreenElement material(0, y1, offset, y2, "headermaterial.png");
			material.draw(texture_manager);
		}

		else ScreenElement::draw(texture_manager);
	}
};

#endif