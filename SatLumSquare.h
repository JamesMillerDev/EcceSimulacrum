#ifndef SATLUMSQUARE
#define SATLUMSQUARE

#include "Computer.h"
#include "Canvas.h"

struct SatLumSquare : public ScreenElement
{
	Canvas* canvas;
	GLubyte* image;
	int pos_x, pos_y;
	bool mouse_held = false;
	SatLumSquare(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Canvas* _canvas) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), canvas(_canvas)
	{
		/*x1 = floor(scalex(x1));
		x2 = floor(scalex(x2));
		y1 = floor(scaley(y1));
		y2 = floor(scaley(y2));
		no_scale = true;*/
		no_scale = true;
		image = new GLubyte[(int)(x2 - x1) * (int)(y2 - y1) * 4];
	}

	void draw(TextureManager* texture_manager)
	{
		for (int i = 0; i < x2 - x1; ++i)
		{
			for (int j = 0; j < y2 - y1; ++j)
			{
				float sat = (float)i / (float)(x2 - x1);
				float lum = (float)j / (float)(y2 - y1);
				Vector3int rgb = canvas->hsl_to_rgb(atof(canvas->h_box->text.c_str()), sat * 100, lum * 100);
				int start = j * (y2 - y1) * 4 + i * 4;
				image[start] = (int)rgb.x;
				image[start + 1] = (int)rgb.y;
				image[start + 2] = (int)rgb.z;
				image[start + 3] = 255;
			}
		}

		glDisable(GL_TEXTURE_2D);
		glRasterPos2f(x1, y1);
		glDrawPixels(x2 - x1, y2 - y1, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glEnable(GL_TEXTURE_2D);
		ScreenElement circle(x1 + pos_x - 5, y1 + pos_y - 5, x1 + pos_x + 5, y1 + pos_y + 5, "circle.png");
		circle.no_scale = true;
		circle.draw(texture_manager);
	}

	void mouse_clicked(int button, int state, int x, int y)
	{
		if (state == GLUT_UP)
		{
			mouse_held = false;
			return;
		}

		pos_x = x - x1;
		pos_y = y - y1;
		canvas->s_box->type_into(to_string((int)(100.0 * (float)pos_x / (float)(x2 - x1))));
		canvas->l_box->type_into(to_string((int)(100.0 * (float)pos_y / (float)(y2 - y1))));
		mouse_held = true;
	}

	void mouse_moved(int x, int y)
	{
		if (!mouse_held)
			return;

		pos_x += x - x1 - pos_x;
		pos_y += y - y1 - pos_y;
		if (pos_x >= x2 - x1)
			pos_x = x2 - x1 /*- 1*/;

		if (pos_x < 0)
			pos_x = 0;

		if (pos_y >= y2 - y1)
			pos_y = y2 - y1 /*- 1*/;

		if (pos_y < 0)
			pos_y = 0;

		canvas->s_box->type_into(to_string((int)(100.0 * (float)pos_x / (float)(x2 - x1))));
		canvas->l_box->type_into(to_string((int)(100.0 * (float)pos_y / (float)(y2 - y1))));
	}

	void animate()
	{
		pos_x = (int)(atof(canvas->s_box->text.c_str()) / 100.0 * (x2 - x1));
		pos_y = (int)(atof(canvas->l_box->text.c_str()) / 100.0 * (y2 - y1));
	}

	~SatLumSquare()
	{
		delete[] image;
	}
};

#endif