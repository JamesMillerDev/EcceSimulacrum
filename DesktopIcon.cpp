#include "DesktopIcon.h"

void DesktopIcon::mouse_clicked(int button, int state, int x, int y)
{
	if (state != GLUT_DOWN)
		return;
	
	int time_clicked = glutGet(GLUT_ELAPSED_TIME);
	if (time_last_clicked < 0)
	{
		time_last_clicked = time_clicked;
		return;
	}

	if (time_clicked - time_last_clicked < 750)
	{
		parent->start_application(target_app);
		//if (!(parent->update_in_progress && (target_app == FILES || target_app == CONSOLE)))
		//	parent->focus_application = target_app;

		take_focus();
		time_last_clicked = 0;
		return;
	}

	time_last_clicked = time_clicked;
}

void DesktopIcon::draw(TextureManager* texture_manager)
{
	if (has_focus)
		glColor4f(0.0, 0.4, 1.0, 1.0);
		
	ScreenElement::draw(texture_manager);
	glColor4f(1.0, 1.0, 1.0, 1.0);
}