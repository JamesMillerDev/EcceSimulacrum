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
	float prev_hue;
	SatLumSquare(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Canvas* _canvas) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), canvas(_canvas)
	{
		/*x1 = floor(scalex(x1));
		x2 = floor(scalex(x2));
		y1 = floor(scaley(y1));
		y2 = floor(scaley(y2));
		no_scale = true;*/
		no_scale = true;
		image = new GLubyte[(int)(x2 - x1) * (int)(y2 - y1) * 4];
		prev_hue = -1;
	}

	void init_image()
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
	}

	void draw(TextureManager* texture_manager)
	{
		if (prev_hue == -1)
		{
			prev_hue = atof(canvas->h_box->text.c_str());
			init_image();
		}

		glDisable(GL_TEXTURE_2D);
		//glRasterPos2f(x1, y1);
		//glDrawPixels(x2 - x1, y2 - y1, GL_RGBA, GL_UNSIGNED_BYTE, image);
		if (x1 < 0 || y1 < 0)
		{
			int x_loss = x1 < 0 ? x1 : 0;
			int y_loss = y1 < 0 ? y1 : 0;
			int new_temp_width = (int)(x2 - x1) + x_loss;
			int new_temp_height = (int)(y2 - y1) + y_loss;
			glPixelStorei(GL_UNPACK_ROW_LENGTH, x2 - x1);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, -x_loss);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, -y_loss);
			glRasterPos2f(x_loss != 0 ? 0.0 : x1, y_loss != 0 ? 0.0 : y1);
			glDrawPixels(new_temp_width, new_temp_height, GL_RGBA, GL_UNSIGNED_BYTE, image);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		}

		else
		{
			glRasterPos2f(x1, y1);
			glDrawPixels(x2 - x1, y2 - y1, GL_RGBA, GL_UNSIGNED_BYTE, image);
		}

		glEnable(GL_TEXTURE_2D);
		ScreenElement circle(x1 + pos_x - 5, y1 + pos_y - 5, x1 + pos_x + 5, y1 + pos_y + 5, atof(canvas->l_box->text.c_str()) > 50 ? "circle.png" : "circle2.png");
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
		float cur_hue = atof(canvas->h_box->text.c_str());
		if (cur_hue != prev_hue)
		{
			init_image();
			prev_hue = cur_hue;
		}
	}

	~SatLumSquare()
	{
		delete[] image;
	}
};

#endif