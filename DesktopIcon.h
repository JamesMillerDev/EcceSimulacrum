#ifndef DESKTOP_ICON
#define DESKTOP_ICON

#include "Computer.h"

struct DesktopIcon : public ScreenElement
{
	Computer* parent;
	int time_last_clicked;
	Application target_app;
	DesktopIcon(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _target_app): ScreenElement(_x1, _y1, _x2, _y2, _name), parent(_parent), time_last_clicked(-1), target_app(_target_app){}
	void mouse_clicked(int button, int state, int x, int y);
	void draw(TextureManager* texture_manager);
};

#endif