#include "BrokenThing.h"

void BrokenThing::mouse_clicked(int button, int state, int x, int y)
{
	if (state != GLUT_DOWN)
		return;
	
	if (!clicked)
	{
		clicked = true;
		scroll_panel->break_points.push_back(pair<int, int>(x, y));
		auto pieces = scroll_panel->get_pieces(x2 - x1, y2 - y1, 0, y1, "littleglassmap.png", 200, 139);
		scroll_panel->create_glass_textures(pieces, x2 - x1, y2 - y1, 0, y1, name, "littleglass.png", 200, 139, "p");
		glass_textures = scroll_panel->glass_textures;
		scroll_panel->break_points.clear();
		scroll_panel->glass_textures.clear();
		for (int i = 0; i < glass_textures.size(); ++i)
			glass_textures[i].translate(x1, y1);
	}

	else
	{
		for (int i = 0; i < glass_textures.size(); ++i)
		{
			glass_textures[i].fall();
			glass_textures[i].velocity_y = 5.0;
			glass_textures[i].velocity_x = rand() % 2 == 0 ? -2.0 : 2.0;
		}

		if (!reported)
		{
			reported = true;
			scroll_panel->parent->increment_breaking_stage();
		}
	}
}

void BrokenThing::animate()
{
	for (int i = 0; i < glass_textures.size(); ++i)
		glass_textures[i].animate();
}

void BrokenThing::draw(TextureManager* texture_manager)
{
	if (!clicked)
		ScreenElement::draw(texture_manager);

	else
	{
		for (int i = 0; i < glass_textures.size(); ++i)
			glass_textures[i].draw(texture_manager);
	}
}