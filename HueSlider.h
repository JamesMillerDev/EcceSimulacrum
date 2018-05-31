#ifndef HUE_SLIDER
#define HUE_SLIDER

#include "Computer.h"
#include "Canvas.h"

struct HueSlider : public ScreenElement
{
	Canvas* canvas;
	GLubyte* image;
	bool mouse_held = false;
	int pos_y = 0;
	HueSlider(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Canvas* _canvas) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), canvas(_canvas)
	{
		/*x1 = floor(scalex(x1));
		x2 = floor(scalex(x2));
		y1 = floor(scaley(y1));
		y2 = floor(scaley(y2));
		no_scale = true;*/
		no_scale = true;
		image = new GLubyte[(int)(x2 - x1) * (int)(y2 - y1) * 4];
		for (int j = 0; j < y2 - y1; ++j)
		{
			float hue = ((float)j / (y2 - y1)) * 360.0;
			Vector3int rgb = canvas->hsl_to_rgb(hue, 100.0, 50.0);
			for (int i = 0; i < x2 - x1; ++i)
			{
				int start = j * (x2 - x1) * 4 + i * 4;
				image[start] = rgb.x;
				image[start + 1] = rgb.y;
				image[start + 2] = rgb.z;
				image[start + 3] = 255;
			}
		}
	}

	void draw(TextureManager* texture_manager)
	{
		glPushMatrix();
		glTranslatef(x2, y1 + pos_y, 0.0);
		glRotatef(-30, 0.0, 0.0, 1.0);
		ScreenElement arrow(0, 0, 10, 10, "smallarrow.png");
		arrow.draw(texture_manager);
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
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
	}

	void mouse_clicked(int button, int state, int x, int y)
	{
		if (state == GLUT_UP)
		{
			mouse_held = false;
			return;
		}

		pos_y = y - y1;
		canvas->h_box->type_into(to_string((int)((float)pos_y / (y2 - y1) * 360.0)));
		mouse_held = true;
	}

	void mouse_moved(int x, int y)
	{
		if (!mouse_held)
			return;
		
		pos_y += y - y1 - pos_y;
		if (pos_y >= (y2 - y1))
			pos_y = y2 - y1 - 1;

		if (pos_y < 0)
			pos_y = 0;

		canvas->h_box->type_into(to_string((int)((float)pos_y / (y2 - y1) * 360.0)));
	}

	void animate()
	{
		pos_y = atof(canvas->h_box->text.c_str()) / 360.0 * (y2 - y1);
	}

	~HueSlider()
	{
		delete[] image;
	}
};

#endif