#include "ScrollBar.h"

ScrollBar::ScrollBar(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, ScrollPanel* _scroll_panel): ScreenElement(_x1, _y1, _x2, _y2, _name, _application)
{
    prev_mouse_x = 0;
    prev_mouse_y = 0;
    being_dragged = false;
    scroll_panel = _scroll_panel;
}

void ScrollBar::mouse_clicked(int button, int state, int x, int y)
{
    if (state == GLUT_UP)
    {
        being_dragged = false;
        return;
    }
    
    prev_mouse_x = x;
    prev_mouse_y = y;
    being_dragged = true;
}

void ScrollBar::mouse_moved(int x, int y)
{
    if (!being_dragged)
        return;
    
    int delta = y - prev_mouse_y;
    scroll_panel->tick_scrollbar(delta);
    prev_mouse_x = x;
    prev_mouse_y = y;
    move(delta);
}

void ScrollBar::move(float delta)
{
    y1 += delta;
    y2 += delta;
    if (delta < 0)
    {
        if (y1 < 76.0)
        {
            float correction = 76.0 - y1;
            y1 = 76.0;
            y2 += correction;
        }
    }
    
    if (delta > 0)
    {
        if (y2 > 1024.0)
        {
            float correction = 1024.0 - y2;
            y1 += correction;
            y2 = 1024.0;
        }
    }
}

void ScrollBar::mouse_over(int x, int y)
{
	glowing = true;
}

void ScrollBar::mouse_off()
{
	glowing = false;
}

void ScrollBar::take_focus()
{
	being_dragged = false;
}

void ScrollBar::draw(TextureManager* texture_manager)
{
	ScreenElement::draw(texture_manager);
	ScreenElement cap1(x1, y1, x2, y1 + 50, "cap1.png");
	ScreenElement cap2(x1, y2 - 50, x2, y2, "cap2.png");
	cap1.draw(texture_manager);
	cap2.draw(texture_manager);
	if (being_dragged)
	{
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glColor4f(0.0, 0.0, 0.0, 0.5);
			glVertex2f(x1, y1);
			glVertex2f(x1, y2);
			glVertex2f(x2, y2);
			glVertex2f(x2, y1);
		glEnd();
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_TEXTURE_2D);
	}

	else if (glowing)
	{
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
		glColor4f(1.0, 1.0, 1.0, 0.5);
			glVertex2f(x1, y1);
			glVertex2f(x1, y2);
			glVertex2f(x2, y2);
			glVertex2f(x2, y1);
		glEnd();
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_TEXTURE_2D);
	}
}