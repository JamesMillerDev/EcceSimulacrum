#include <sstream>
#include "PowerSymbol.h"
#include "GameParams.h"

void PowerSymbol::mouse_over(int x, int y)
{
	if (!mouse_on)
		mouse_on = true;
}

void PowerSymbol::mouse_off()
{
	if (mouse_on)
		mouse_on = false;
}

void PowerSymbol::mouse_clicked(int button, int state, int x, int y)
{
	if (parent->the_end)
		return;

	if (parent->everything_stuck)
		parent->power_bucket++;
	
	if (state != GLUT_DOWN)
		return;

	parent->go_to_sleep();
}

void PowerSymbol::animate()
{
	/*int current_time = glutGet(GLUT_ELAPSED_TIME);
	if (current_time - timebase >= POWER_BUTTON_MS_TO_FRAME_CHANGE)
	{
		if (mouse_on && current_frame < POWER_BUTTON_MAX_FRAMES)
			current_frame++;

		else if (!mouse_on && current_frame > 1)
			current_frame--;

		std::stringstream ss;
		ss << current_frame;
		name = "powersymbol" + ss.str() + ".png";
		timebase = current_time;
	}*/

	if (parent->the_end)
		name = "blackwhitebutton.png";

	if (mouse_on)
		name = "powersymbol2.png";

	else name = "powersymbol1.png";
}