#include "InfoText.h"
#include "fonts.h"

void InfoText::draw(TextureManager* texture_manager)
{
	ScreenElement::draw(texture_manager);
	float xcoord = x1 + 10.0;
	if (center_horiz)
	{
		int width = draw_string(texture_manager, 32, text, 0, 0, false).back();
		int mid = x1 + (x2 - x1) / 2;
		xcoord = mid - (width / 2);
	}

	glColor4f(1.0, 1.0, 1.0, alpha);
	draw_string(texture_manager, 32, text, xcoord, (y1 + y2) / 2.0, true, highlight, 0, no_scale, false, false); //was 32
	glColor4f(1.0, 1.0, 1.0, 1.0);
}