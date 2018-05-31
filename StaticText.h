#ifndef STATIC_TEXT
#define STATIC_TEXT

#include "Computer.h"
#include "fonts.h"

struct StaticText : public ScreenElement
{
	string text;
	float ledger;
	int points;
	Computer* parent;
	bool multiline;
	StaticText(float _x1, float _y1, string _text, Application application, int _points, Computer* _parent, bool _multiline = false) : ScreenElement(_x1, _y1, _x1, _y1, "", application), text(_text), ledger(_y1), points(_points), parent(_parent), multiline(_multiline)
	{
		auto top_and_bottom = get_top_and_bottom(text, points);
		y1 = ledger + top_and_bottom.second;
		y2 = ledger + top_and_bottom.first;
		auto widths = draw_string(parent->texture_manager, points, text, 0, 0, false);
		x2 = x1 + widths.back();
	}

	void draw(TextureManager* texture_manager)
	{
		if (!multiline)
			draw_string(texture_manager, points, text, x1, ledger, true, true, 0);

		else draw_string_bounded(texture_manager, points, text, x1, ledger, 5000, 20);
	}

	void top_justify(int top)
	{
		this->translate(0, top - y2);
	}

	void translate(float delta_x, float delta_y)
	{
		ScreenElement::translate(delta_x, delta_y);
		ledger += delta_y;
	}
};

#endif