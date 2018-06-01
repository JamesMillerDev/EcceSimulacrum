#include "XButton.h"

void XButton::mouse_clicked(int button, int state, int x, int y)
{
	if (state != GLUT_DOWN)
		return;

	parent->close_application(application);
	if (parent->everything_stuck &&
		parent->minimize_bucket >= 3 &&
		parent->close_bucket >= 3 &&
		parent->url_bucket >= 3 &&
		parent->scroll_bucket >= 3 &&
		parent->power_bucket >= 3 &&
		parent->tray_bucket >= 3 &&
		parent->breaking_stage == 0)
		parent->increment_breaking_stage();

}