#ifndef INFO_TEXT
#define INFO_TEXT

#include <string>
#include "Computer.h"

using std::string;

struct InfoText : public ScreenElement
{
	string text;
	bool highlight;
	bool center_horiz;
	InfoText(float _x1, float _y1, float _x2, float _y2, string name, string _text, Application application = INFO_BOX, bool _highlight = false) : ScreenElement(_x1, _y1, _x2, _y2, name, application), text(_text), highlight(_highlight), center_horiz(false) {}
	void draw(TextureManager* texture_manager);
};

#endif