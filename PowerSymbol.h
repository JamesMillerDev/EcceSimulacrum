#ifndef POWER_SYMBOL
#define POWER_SYMBOL

#include "Computer.h"

struct PowerSymbol : public ScreenElement
{
	Computer* parent;
	PowerSymbol(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent): ScreenElement(_x1, _y1, _x2, _y2, _name), parent(_parent), mouse_on(false), timebase(0), current_frame(1) {}
	void mouse_over(int x, int y);
	void mouse_off();
	void mouse_clicked(int button, int state, int x, int y);
	void animate();
	bool mouse_on;
	int timebase;
	int current_frame;
};

#endif