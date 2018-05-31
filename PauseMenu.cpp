#include "PauseMenu.h"
#include "Button.h"

PauseMenu::PauseMenu(TextureManager* _texture_manager)
{
	texture_manager = _texture_manager;
	prev_mouse_x = prev_mouse_y = 100;
	pause_done = false;
	auto black = make_unique<ScreenElement>(0, 0, 1920, 1080, "black.png");
	black->alpha = 0.7;
	screen_elements.push_back(std::move(black));
	screen_elements.push_back(make_unique<ScreenElement>(880, 1080 - 402, -1, -1, "doyouwanttoquit.png"));
	screen_elements.push_back(make_unique<Button>(880, 1080 - 447, -1, -1, "yes1.png", NONE, []() { exit(0); }, []() {}, "", "yes2.png", "yes2.png", "", false, true));
	screen_elements.push_back(make_unique<Button>(999, 1080 - 447, -1, -1, "no1.png", NONE, [this]() { pause_done = true; }, []() {}, "", "no2.png", "no2.png", "", false, true));
}

void PauseMenu::window_resized(int new_width, int new_height)
{
	width = new_width;
	height = new_height;
}

void PauseMenu::setup_view()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, width, 0.0, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void PauseMenu::draw()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < screen_elements.size(); ++i)
		screen_elements[i]->draw(texture_manager);

	ScreenElement cursorelem = ScreenElement(prev_mouse_x, prev_mouse_y - texture_manager->get_height("cursor.png"), prev_mouse_x + texture_manager->get_width("cursor.png"), prev_mouse_y, "cursor.png");
	cursorelem.translate(-1, 1);
	cursorelem.draw(texture_manager);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void PauseMenu::mouse_moved(int x, int y)
{
	y = glutGet(GLUT_WINDOW_HEIGHT) - y; //-1?
	x = (int)((float)x * (1920.0 / (float)glutGet(GLUT_WINDOW_WIDTH)));
	y = (int)((float)y * (1080.0 / (float)glutGet(GLUT_WINDOW_HEIGHT)));
	bool already_moused_over = false;
	for (int i = screen_elements.size() - 1; i >= 0; --i)
	{
		ScreenElement* se = screen_elements[i].get();
		if (se->x1 <= x && x <= se->x2 && se->y1 <= y && y <= se->y2 && !already_moused_over && !se->never_focus)
		{
			se->mouse_over(x, y);
			already_moused_over = true;
		}

		else se->mouse_off();
	}

	prev_mouse_x = x;
	prev_mouse_y = y;
}

void PauseMenu::mouse_clicked(int button, int state, int x, int y)
{
	y = glutGet(GLUT_WINDOW_HEIGHT) - y; //-1?
	x = (int)((float)x * (1920.0 / (float)glutGet(GLUT_WINDOW_WIDTH)));
	y = (int)((float)y * (1080.0 / (float)glutGet(GLUT_WINDOW_HEIGHT)));
	bool already_clicked = false;
	for (int i = screen_elements.size() - 1; i >= 0; --i)
	{
		ScreenElement* se = screen_elements[i].get();
		if (state == GLUT_DOWN)
		{
			if ((se->x1 <= x && x <= se->x2 && se->y1 <= y && y <= se->y2) && !already_clicked)
			{
				if (se->responds_to(button))
					se->mouse_clicked(button, state, x, y);

				already_clicked = true;
			}

			else
				se->take_focus();
		}

		else if (state == GLUT_UP)
		{
			if (true)
			{
				if (se->x1 <= x && x <= se->x2 && se->y1 <= y && y <= se->y2 && !already_clicked)
				{
					se->mouse_clicked(button, state, x, y);
					already_clicked = true;
				}

				else
				{
					if (se->receive_out_of_bounds_clicks)
						se->mouse_clicked(button, state, x, y);
				}
			}
		}
	}
}