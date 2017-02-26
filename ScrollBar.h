#ifndef SCROLL_BAR
#define SCROLL_BAR

#include "Computer.h"
#include "ScrollPanel.h"

struct ScrollBar : public ScreenElement
{
    int prev_mouse_x;
    int prev_mouse_y;
    bool being_dragged;
	bool glowing = false;
    ScrollPanel* scroll_panel;
    ScrollBar(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, ScrollPanel* _scroll_panel);
    void move(float delta);
    void mouse_clicked(int button, int state, int x, int y);
    void mouse_moved(int x, int y);
	void mouse_over(int x, int y);
	void mouse_off();
	void take_focus();
	void draw(TextureManager* texture_manager);
};

#endif