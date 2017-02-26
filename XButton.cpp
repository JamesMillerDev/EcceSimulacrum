#include "XButton.h"

void XButton::mouse_clicked(int button, int state, int x, int y)
{
	if (state != GLUT_DOWN)
		return;

	parent->close_application(application);
}