#ifndef STATIC_CANVAS
#define STATIC_CANVAS

#include "Computer.h"

struct StaticCanvas : public ScreenElement
{
	GLubyte* image;
	StaticCanvas(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, GLubyte* _image) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), image(_image) {}
	void draw(TextureManager* texture_manager)
	{
		glDisable(GL_TEXTURE_2D);
		glRasterPos2f(x1, y1);
		glDrawPixels(x2 - x1, y2 - y1, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glEnable(GL_TEXTURE_2D);
	}
};

#endif