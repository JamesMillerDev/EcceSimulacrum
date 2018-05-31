#ifndef NUMBER_FIELD
#define NUMBER_FIELD

#include "TextField.h"
#include "Button.h"

struct NumberField : public TextField
{
	int number = 0;
	NumberField(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application) : TextField(_x1, _y1, _x2, _y2, _name, _parent, _application)
	{
		parent->to_be_added.push_back(make_unique<Button>(x2 - 50.0, y2 - 50.0, x2, y2, "arrow.png", application,
			[this]()
		{
			this->number++;
		}, [](){}));

		parent->to_be_added.push_back(make_unique<Button>(x2 - 50.0, y1, x2, y1 + 50.0, "arrow2.png", application,
			[this]()
		{
			this->number--;
		}, [](){}));
	}

	void draw(TextureManager* texture_manager)
	{
		text = std::to_string(number);
		TextField::draw(texture_manager);
	}
};

#endif NUMBER_FIELD