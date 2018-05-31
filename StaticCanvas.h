#ifndef STATIC_CANVAS
#define STATIC_CANVAS

#include "Computer.h"

struct StaticCanvas : public ScreenElement
{
	Computer* parent;
	StaticCanvas(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Computer* _parent) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent)
	{
		parent->texture_manager->load_texture("contest", true, false, parent->uploaded_image.image, parent->uploaded_image.xsize, parent->uploaded_image.ysize);
		name = "contest";
	}

	void draw(TextureManager* texture_manager)
	{
		this->invert = true;
		ScreenElement::draw(texture_manager);
	}
};

#endif