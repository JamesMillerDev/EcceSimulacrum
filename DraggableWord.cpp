#include "DraggableWord.h"
#include "BlankEssay.h"

DraggableWord::DraggableWord(float _x1, float _y1, float _x2, float _y2, string _name, BlankEssay* _owner) : ScreenElement(_x1, _y1, _x2, _y2, _name), movable(true), being_dragged(false), owner(_owner)
{
}

void DraggableWord::mouse_clicked(int button, int state, int x, int y)
{
	if (button != GLUT_LEFT)
		return;

	if (!movable)
		return;

	if (state == GLUT_UP)
	{
		being_dragged = false;
		owner->word_in_blank(this);
	}

	else if (state == GLUT_DOWN)
	{
		being_dragged = true;
		prev_mouse_x = x;
		prev_mouse_y = y;
	}
}

void DraggableWord::mouse_moved(int x, int y)
{
	if (!being_dragged)
		return;

	int xtrans = x - prev_mouse_x;
	int ytrans = y - prev_mouse_y;
	if (x1 + xtrans >= owner->x1 && x2 + xtrans <= owner->x2)
		translate(xtrans, 0);

	else
		(x1 + xtrans < owner->x1) ? translate(owner->x1 - x1, 0) : translate(owner->x2 - x2, 0);

	if (y1 + ytrans >= owner->y1 && y2 + ytrans <= owner->y2)
		translate(0, ytrans);

	else
		(y1 + ytrans < owner->y1) ? translate(0, owner->y1 - y1) : translate(0, owner->y2 - y2);

	prev_mouse_x = x;
	prev_mouse_y = y;
	if (prev_mouse_x < owner->x1)
		prev_mouse_x = owner->x1;

	if (prev_mouse_x > owner->x2)
		prev_mouse_x = owner->x2;

	if (prev_mouse_y < owner->y1)
		prev_mouse_y = owner->y1;

	if (prev_mouse_y > owner->y2)
		prev_mouse_y = owner->y2;
}