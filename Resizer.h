#ifndef RESIZER
#define RESIZER

#include "Computer.h"
#include "Canvas.h"

struct Resizer : public ScreenElement
{
	Computer* parent;
	Canvas* canvas;
	int mode;
	bool mouse_held = false;
	bool i_changed_cursor = false;
	int dx = 0, dy = 0;
	Resizer(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Canvas* _canvas, int _mode, Computer* _parent) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), 
		canvas(_canvas), mode(_mode), parent(_parent) 
	{
		no_scale = true;
	}

	void mouse_clicked(int button, int state, int x, int y)
	{
		if (state == GLUT_DOWN)
		{
			mouse_held = true;
			canvas->being_resized = true;
			return;
		}

		//TODO states besides up and down?
		if (state == GLUT_UP)
		{
			mouse_held = false;
			canvas->being_resized = false;
			return;
		}
	}

	void mouse_moved(int x, int y)
	{
		if (!mouse_held)
			return;

		dx = x - x1; //TODO not quite accurate
		dy = y - y1;
		if (dx + canvas->x2 > 1900)
			dx = 1900 - canvas->x2;

		if (dx + canvas->x2 < 250)
			dx = 250 - canvas->x2;

		if (dy + canvas->y1 < 70)
			dy = 70 - canvas->y1;

		if (dy + canvas->y1 > 750)
			dy = 750 - canvas->y1;
	}

	void mouse_over(int x, int y)
	{
		parent->set_cursor("cursorhand.png");
		parent->maintain_cursor = true;
		i_changed_cursor = true;
	}

	void mouse_off()
	{
		if (!mouse_held && i_changed_cursor)
		{
			parent->set_cursor("cursor.png");
			parent->maintain_cursor = false;
			i_changed_cursor = false;
		}
	}

	void take_focus()
	{
		has_focus = false;
		if (i_changed_cursor)
		{
			parent->set_cursor("cursor.png");
			parent->maintain_cursor = false;
			i_changed_cursor = false;
		}

		if (!mouse_held)
			return;

		mouse_held = false;
		if (mode == 0)
			canvas->resize(canvas->x1, canvas->y1, canvas->x2 + dx, canvas->y2);

		else if (mode == 1)
			canvas->resize(canvas->x1, canvas->y1 + dy, canvas->x2, canvas->y2);

		else if (mode == 2)
			canvas->resize(canvas->x1, canvas->y1 + dy, canvas->x2 + dx, canvas->y2);

		dx = dy = 0;
		canvas->being_resized = false;
	}

	void animate()
	{
		if (mode == 0)
		{
			x1 = canvas->x2 + 2;
			y1 = (canvas->y2 - canvas->y1) / 2 + canvas->y1;
			x2 = x1 + 5;
			y2 = y1 + 5;
		}

		else if (mode == 1)
		{
			x1 = (canvas->x2 - canvas->x1) / 2 + canvas->x1;
			y1 = canvas->y1 - 5;
			x2 = x1 + 5;
			y2 = y1 + 5;
		}

		else if (mode == 2)
		{
			x1 = canvas->x2 + 2;
			y1 = canvas->y1 - 5;
			x2 = x1 + 5;
			y2 = y1 + 5;
		}
	}

	void draw(TextureManager* texture_manager)
	{
		ScreenElement::draw(texture_manager);
		if (mouse_held)
		{
			glDisable(GL_MULTISAMPLE);
			int temp_dx = dx;
			int temp_dy = dy;
			if (mode == 0)
				dy = 0;

			if (mode == 1)
				dx = 1;

			glColor4f(0.0, 0.0, 0.0, 1.0);
			glBegin(GL_LINE_STRIP);
				glVertex2f(canvas->x1, canvas->y2);
				glVertex2f(canvas->x2 + dx, canvas->y2);
				glVertex2f(canvas->x2 + dx, canvas->y1 + dy);
				glVertex2f(canvas->x1, canvas->y1 + dy);
				glVertex2f(canvas->x1, canvas->y2);
			glEnd();
			glColor4f(1.0, 1.0, 1.0, 1.0);
			dx = temp_dx;
			dy = temp_dy;
			glEnable(GL_MULTISAMPLE);
		}
	}
};

#endif