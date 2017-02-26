#include "InfoText.h"
#include "fonts.h"

void InfoText::draw(TextureManager* texture_manager)
{
	ScreenElement::draw(texture_manager);
	draw_string(texture_manager, 32, text, x1 + 10.0, (y1 + y2) / 2.0, true, false, 0, no_scale);
}