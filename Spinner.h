#ifndef SPINNER
#define SPINNER

#include "Computer.h"

struct Spinner : public ScreenElement
{
	int count;
	float start_angle, inc_angle;
	float center_x, center_y;
	int frames = 0;
	int counter = 1;
	Spinner(int _count, float _start_angle, float _center_x, float _center_y, Application _application): ScreenElement(0, 0, 0, 0, "", _application), count(_count), start_angle(_start_angle), center_x(_center_x), center_y(_center_y)
	{
		inc_angle = 360.0 / count;
	}

	void animate()
	{
		frames++;
		if (frames % 5 == 0)
			counter++;
	}

	void draw(TextureManager* texture_manager)
	{
		glPushMatrix();
			glTranslatef(center_x, center_y, 0.0);
			glRotatef(start_angle - counter * inc_angle, 0.0, 0.0, 1.0);
			for (int i = 0; i < count; ++i)
			{
				glRotatef(inc_angle, 0.0, 0.0, 1.0);
				float alpha = ((float)i / (float)count);
				ScreenElement spin(-45 - 22, -5, -20, 6, "spinnerdark.png");
				spin.draw(texture_manager);
				glColor4f(1.0, 1.0, 1.0, alpha);
				glDisable(GL_TEXTURE_2D);
				glRectf(scalex(-45 - 22), scaley(-5), scalex(-20), scaley(6));
				glEnable(GL_TEXTURE_2D);
				glColor4f(1.0, 1.0, 1.0, 1.0);
			}
		glPopMatrix();
	}
};

#endif