#ifndef SIZE_SLIDER
#define SIZE_SLIDER

#include "Computer.h"
#include "Canvas.h"

struct SizeSlider : public ScreenElement
{
	int prev_mouse_x;
	bool size;
	bool mouse_held = false;
	bool i_set_cursor = false;
	Canvas* canvas;
	SizeSlider(int _x1, int _y1, int _x2, int _y2, string _name, Application _application, bool _size, Canvas* _canvas) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), size(_size), prev_mouse_x(0), canvas(_canvas) {}
	void mouse_clicked(int button, int state, int x, int y)
	{
		if (state == GLUT_DOWN)
		{
			mouse_held = true;
			prev_mouse_x = x;
		}

		else if (state == GLUT_UP)
			mouse_held = false;
	}

	void mouse_moved(int x, int y)
	{
		if (!mouse_held)
			return;
		
		if (size)
			canvas->change_size(x - prev_mouse_x);

		else canvas->change_opacity(x - prev_mouse_x);
		prev_mouse_x = x;
	}

	void mouse_over(int x, int y)
	{
		if (!canvas->parent->cursor_lock)
		{
			canvas->parent->set_cursor("cursorleftright.png");
			canvas->parent->maintain_cursor = true;
			canvas->parent->cursor_lock = true;
			i_set_cursor = true;
		}
	}

	void mouse_off()
	{
		if (!mouse_held && i_set_cursor)
		{
			canvas->parent->maintain_cursor = false;
			i_set_cursor = false;
			canvas->parent->cursor_lock = false;
		}
	}

	void take_focus()
	{
		ScreenElement::take_focus();
		if (i_set_cursor)
		{
			canvas->parent->cursor_lock = false;
			canvas->parent->set_cursor("cursor.png");
			mouse_held = false;
		}
	}
};

#endif