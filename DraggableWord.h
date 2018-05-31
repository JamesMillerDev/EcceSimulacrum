#ifndef DRAGGABLE_WORD
#define DRAGGABLE_WORD

#include "Computer.h"

struct BlankEssay;

struct DraggableWord : public ScreenElement
{
	DraggableWord(float _x1, float _y1, float _x2, float _y2, string _name, BlankEssay* _owner);
	BlankEssay* owner;
	void mouse_clicked(int button, int state, int x, int y);
	void mouse_moved(int x, int y);
	bool movable, being_dragged;
	int prev_mouse_x, prev_mouse_y;
};

#endif