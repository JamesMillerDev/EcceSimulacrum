#ifndef INFO_TEXT
#define INFO_TEXT

#include <string>
#include "Computer.h"

using std::string;

struct InfoText : public ScreenElement
{
	string text;
	InfoText(float _x1, float _y1, float _x2, float _y2, string name, string _text, Application application = INFO_BOX) : ScreenElement(_x1, _y1, _x2, _y2, name, application), text(_text) {}
	void draw(TextureManager* texture_manager);
};

#endif