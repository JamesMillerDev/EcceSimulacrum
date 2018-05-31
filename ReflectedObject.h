#ifndef REFLECTED_OBJECT
#define REFLECTED_OBJECT

#include "Computer.h"
#include "RectUtilities.h"

struct ReflectedObject : public ScreenElement
{
	bool reflecting = false;
	float y_velocity, x_velocity;
	bool velocity_increasing = true;
	int velocity_counter;
	float local_alpha = 0.0;
	vector<Rect> pieces;
	int mode, degree, frames;
	ScrollPanel* scroll_panel;
	ReflectedObject(int _x1, int _y1, int _x2, int _y2, string _name, Application _application, int _mode, ScrollPanel* _scroll_panel) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), mode(_mode), scroll_panel(_scroll_panel)
	{
		frames = 0;
		y_velocity = -9.0;
		velocity_counter = 8;
		if (mode == 1)
		{
			x_velocity = 1;//rand() % 9 - 4;
			y_velocity = 0;//rand() % 9 - 4;
			degree = 20;
		}
	}

	void draw_pieces(TextureManager* texture_manager)
	{
		if (need_resize)
		{
			x2 = x1 + texture_manager->get_width(name);
			y2 = y1 + texture_manager->get_height(name);
			need_resize = false;
		}

		texture_manager->change_texture(name);
		for (int i = 0; i < pieces.size(); ++i)
		{
			float ex1 = (pieces[i].x1 - x1) / (x2 - x1);
			float ex2 = (pieces[i].x2 - x1) / (x2 - x1);
			float ey1 = 1.0 - (pieces[i].y1 - y1) / (y2 - y1);
			float ey2 = 1.0 - (pieces[i].y2 - y1) / (y2 - y1);
			glBegin(GL_QUADS);
			glTexCoord2f(ex1, ey1); glVertex2f(scalex(pieces[i].x1 + pieces[i].xtrans), scaley(pieces[i].y1 + pieces[i].ytrans));
			glTexCoord2f(ex1, ey2); glVertex2f(scalex(pieces[i].x1 + pieces[i].xtrans), scaley(pieces[i].y2 + pieces[i].ytrans));
			glTexCoord2f(ex2, ey2); glVertex2f(scalex(pieces[i].x2 + pieces[i].xtrans), scaley(pieces[i].y2 + pieces[i].ytrans));
			glTexCoord2f(ex2, ey1); glVertex2f(scalex(pieces[i].x2 + pieces[i].xtrans), scaley(pieces[i].y1 + pieces[i].ytrans));
			glEnd();
		}
	}

	void draw(TextureManager* texture_manager)
	{
		if (!reflecting)
			glColor4f(1.0, 1.0, 1.0, local_alpha);

		if (!pieces.empty())
			draw_pieces(texture_manager);

		else ScreenElement::draw(texture_manager);
		if (!reflecting)
		{
			reflecting = true;
			glPushMatrix();
			glScalef(1.0, -1.0, 1.0);
			glTranslatef(0.0, -1.0 * glutGet(GLUT_WINDOW_HEIGHT), 0.0);
			glColor4f(1.0, 1.0, 1.0, 0.17 * local_alpha);
			draw(texture_manager);
			glPopMatrix();
			glColor4f(1.0, 1.0, 1.0, 1.0);
			reflecting = false;
		}
	}

	void mode1_animate()
	{
		frames++;
		if (frames > 40)
		{
			if (local_alpha > 0)
				local_alpha -= 0.05;

			else
			{
				scroll_panel->ask_for_delete(this);
				return;
			}

			if (degree < 20)
				degree++;
		}

		else
		{
			if (local_alpha < 1.0)
				local_alpha += 0.05;

			if (degree > 0)
				degree--;
		}

		translate(x_velocity, y_velocity);
		if (degree > 0)
		{
			pieces = divide_rect(Rect(x1, y1, x2, y2), degree);
			pieces = translate_skew(pieces, degree);
		}

		else pieces.clear();
	}

	void animate()
	{
		if (mode == 1)
		{
			mode1_animate();
			return;
		}

		if (local_alpha < 1.0)
			local_alpha += 0.02;

		if (y_velocity == 0.0 && velocity_counter == 0)
		{
			if (pieces.empty())
			{
				//pieces = divide_rect(Rect(x1, y1, x2, y2), 3);
				//pieces = translate_skew(pieces, 3);
			}
		}

		translate(0, y_velocity);
		if (velocity_increasing)
		{
			y_velocity += 1.0;
			if (y_velocity >= velocity_counter)
			{
				if (velocity_counter == 0)
					y_velocity = 0.0;

				else
				{
					velocity_counter--;
					velocity_increasing = false;
				}
			}
		}

		else
		{
			y_velocity -= 1.0;
			if (y_velocity <= -1 * velocity_counter)
			{
				if (velocity_counter == 0)
					y_velocity = 0.0;

				else
				{
					velocity_counter--;
					velocity_increasing = true;
				}
			}
		}
	}
};

#endif