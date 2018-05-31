#include "ScrollBar.h"

ScrollBar::ScrollBar(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, ScrollPanel* _scroll_panel): ScreenElement(_x1, _y1, _x2, _y2, _name, _application)
{
    prev_mouse_x = 0;
    prev_mouse_y = 0;
    being_dragged = false;
    scroll_panel = _scroll_panel;
	min_y = 76.0;
	max_y = 1024.0;
}

float ScrollBar::range()
{
	return max_y - min_y;
}

void ScrollBar::mouse_clicked(int button, int state, int x, int y)
{
	if (animating)
		return;
	
	if (state == GLUT_UP)
    {
        being_dragged = false;
        return;
    }
    
	if (scroll_panel->parent->everything_stuck)
		scroll_panel->parent->scroll_bucket++;

	prev_mouse_x = x;
    prev_mouse_y = y;
    being_dragged = true;
	dragging_height = y - y1;
	if (scroll_panel->parent->everything_stuck && scroll_panel->parent->breaking_stage == 2)
	{
		animating = true;
		being_dragged = false;
		glowing = false;
		fall_speed = 20;
		speed_sign = -1;
		to_traverse = y1 - min_y;
		traversed = 0;
	}
}

void ScrollBar::animate()
{
	if (animating)
	{
		frames++;
		if (frames < 120 && frames % 4 == 0)
			being_dragged = !being_dragged;

		translate(0, speed_sign * fall_speed);
		traversed += fall_speed;
		if (traversed >= to_traverse)
		{
			if (y1 <= min_y)
			{
				translate(0, min_y - y1);
				to_traverse /= 2;
			}

			traversed = 0;
			speed_sign *= -1;
			if (to_traverse <= 10 && y1 == min_y)
			{
				fall_speed = 0;
				scroll_panel->parent->increment_breaking_stage();
			}
		}
	}
}

void ScrollBar::mouse_moved(int x, int y)
{
	if (animating)
		return;
	
	if (!being_dragged)
        return;
    
    int delta = y - prev_mouse_y;
    prev_mouse_x = x;
    prev_mouse_y = y;
	if (delta < 0 && y - y1 < dragging_height || delta > 0 && y - y1 > dragging_height)
	{
		scroll_panel->tick_scrollbar(delta);
		move(delta);
	}
}

void ScrollBar::move(float delta)
{
    y1 += delta;
    y2 += delta;
    if (delta < 0)
    {
        if (y1 < min_y)
        {
            float correction = min_y - y1;
            y1 = min_y;
            y2 += correction;
        }
    }
    
    if (delta > 0)
    {
        if (y2 > max_y)
        {
            float correction = max_y - y2;
            y1 += correction;
            y2 = max_y;
        }
    }
}

void ScrollBar::mouse_over(int x, int y)
{
	if (animating)
		return;

	glowing = true;
}

void ScrollBar::mouse_off()
{
	if (animating)
		return;
	
	glowing = false;
}

void ScrollBar::take_focus()
{
	if (animating)
		return;

	ScreenElement::take_focus();
	being_dragged = false;
}

void ScrollBar::draw(TextureManager* texture_manager)
{
	if (name == "scrollbar.png")
	{
		ScreenElement::draw(texture_manager);
		return;
	}
	
	float oldx1 = x1;
	float oldx2 = x2;
	float oldy1 = y1;
	float oldy2 = y2;
	x1 = round(x1);
	x2 = round(x2);
	y1 = round(y1);
	y2 = round(y2);
	if (being_dragged && name != "bluescrollbar.png")
		glColor4f(0.7, 0.7, 0.7, 1.0);

	if (name != "bluescrollbar.png")
	{
		y1 += 4;
		y2 -= 4;
	}

	ScreenElement::draw(texture_manager);
	if (name != "bluescrollbar.png")
	{
		y1 -= 4;
		y2 += 4;
	}

	ScreenElement cap1(x1, y1, x2, y1 + 4, name == "bluescrollbar.png" ? "bluecap1.png" : "cap1.png");
	ScreenElement cap2(x1, y2 - 4, x2, y2, name == "bluescrollbar.png" ? "bluecap2.png" : "cap2.png");
	if (name == "bluescrollbar.png")
	{
		cap1.y1 = y1 - 16;
		cap1.y2 = y1;
		cap2.y1 = y2;
		cap2.y2 = y2 + 16;
	}

	cap1.draw(texture_manager);
	cap2.draw(texture_manager);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	/*if (being_dragged)
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
	}*/

	/*else*/ if (glowing && !being_dragged && name != "bluescrollbar.png")
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