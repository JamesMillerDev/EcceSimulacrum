#ifndef BROKEN_THING
#define BROKEN_THING

#include "Computer.h"
#include "ScrollPanel.h"

struct BrokenThing : public ScreenElement
{
	ScrollPanel* scroll_panel;
	BrokenThing(int _x1, int _y1, int _x2, int _y2, string _name, ScrollPanel* _scroll_panel) : ScreenElement(_x1, _y1, _x2, _y2, _name), scroll_panel(_scroll_panel) {}
	void draw(TextureManager* texture_manager);
	void mouse_clicked(int button, int state, int x, int y);
	void animate();
	vector<PhysicsPiece> glass_textures;
	bool clicked = false;
	bool reported = false;
};

#endif