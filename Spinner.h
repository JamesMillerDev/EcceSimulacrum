#ifndef SPINNER
#define SPINNER

#include "Computer.h"

struct Spinner : public ScreenElement
{
	int count;
	float start_angle, inc_angle;
	float center_x, center_y;
	int current_index = 0;
	int frames = 0;
	Spinner(int _count, float _start_angle, float _center_x, float _center_y, Application _application): ScreenElement(0, 0, 0, 0, "", _application), count(_count), start_angle(_start_angle), center_x(_center_x), center_y(_center_y)
	{
		inc_angle = 360.0 / count;
	}

	void animate()
	{
		frames++;
		if (frames % 5 == 0)
		{
			current_index++;
			if (current_index >= count)
				current_index = 0;
		}
	}

	void draw(TextureManager* texture_manager)
	{
		glPushMatrix();
			glTranslatef(center_x, center_y, 0.0);
			glRotatef(start_angle + inc_angle, 0.0, 0.0, 1.0);
			for (int i = 0; i < count; ++i)
			{
				glRotatef(-inc_angle, 0.0, 0.0, 1.0);
				string texture_name = (current_index == i) ? "spinnerdark.png" : "spinnerlight.png";
				ScreenElement spin(-93 - 20, -4, -20, 5, texture_name);
				spin.draw(texture_manager);
			}
		glPopMatrix();
	}
};

#endif