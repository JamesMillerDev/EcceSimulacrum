#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <thread>
#include <chrono>
#include "Computer.h"
#include "PowerSymbol.h"
#include "TextField.h"
#include "DesktopIcon.h"
#include "XButton.h"
#include "GameParams.h"
#include "fonts.h"
#include "ScrollPanel.h"
#include "ScrollBar.h"
#include "Button.h"
#include "Files.h"
#include "Console.h"
#include "InfoText.h"
#include "BubbleGame.h"
#include "Canvas.h"
#include "Resizer.h"
#include "MMO.h"
#include "HelpCenter.h"
#include "NumberField.h"
#include "Video.h"
#include "ProgressBar.h"
#include "Decryption.h"
#include "keymappings.h"

using namespace std;

//Can't allow user to resize window, otherwise TERRIBLE THINGS WILL HAPPEN (please fix that)
//TODO if application adds new controls while minimized, they show up on the desktop
//TODO clicking on things while forcing_cursor?
//TODO up click on different element should not take focus?  Destroys highlighting in text box.

float scalex(float x)
{
	return x * ((float)glutGet(GLUT_WINDOW_WIDTH) / 1920.0);
}

float scaley(float y)
{
	return y * ((float)glutGet(GLUT_WINDOW_HEIGHT) / 1080.0);
}

float unscalex(float x)
{
	return x * (1920.0 / (float)glutGet(GLUT_WINDOW_WIDTH));
}

float unscaley(float y)
{
	return y * (1080.0 / (float)glutGet(GLUT_WINDOW_HEIGHT));
}

Computer::Computer(TextureManager* manager)
{
	whitelist = { "search", "reddit", "ain", "nenda", "useful" };
	mmo_password = "a";
	applications_open = 0;
	char buffer[100];
	focus_element = NULL;
	computer_state = STATE_DESKTOP;
	emails = { "deliverfailurebold.png", "forumemail2bold.png", "forumemail1bold.png", "rfh2bold.png", "rfh1bold.png" };
	for (int i = 0; i < emails.size(); ++i)
		read_email[emails[i]] = false;

	current_wait = 0;
	prev_time = 0;
	alpha = 1.0;
	alpha_increment = -ALPHA_INCREMENT;
	computer_done = false;
	going_to_sleep = false;
	texture_manager = manager;
	messages.push_back(Message("EXF Boot Agent v2.07.09", 300));
	messages.push_back(Message("Checking dev0... [OK]", 1));
	messages.push_back(Message("Checking DDRS... [OK]", 1));
	messages.push_back(Message("Calibrating eye tracking...", 3000));
	messages.push_back(Message("Loading kernel drivers", 200));
	messages.push_back(Message("ERROR:  Bad boot sector:  Recompiling module HBA0...", 1));
	for (int i = 1; i <= 21; ++i)
	{
		sprintf(buffer, "mixgc hba%d.c -o hba%d.o -l ldd -ICC:/dev/0/init -Floor -cxxnc -clo --sector-output 00A", i, i);
		messages.push_back(Message(string(buffer), 333));
	}

	messages.push_back(Message("mv hba0part0 /dev/0/init", 100));
	messages.push_back(Message("mv hba0part1 /dev/0/init", 100));
	messages.push_back(Message("Loading module HBA1... [OK]", 100));
	messages.push_back(Message("Display driver v7.01", 100));
	messages.push_back(Message("Initializing color matrix... [OK]", 100));
	messages.push_back(Message("Setting EE bump maps... [OK]", 100));
	messages.push_back(Message("Checking stereoscopic display... [FAILURE]", 100));
	messages.push_back(Message("Importing view scan 60Hz... [OK]", 100));
	messages.push_back(Message("Static Ethernet PXN compatible ee6.51.7", 100));
	messages.push_back(Message("Waiting for DHCP response...", 3000));
	messages.push_back(Message("DHCP OK 127.0.9.255", 100));
	messages.push_back(Message("Kernel modules initialized", 500));
	messages.push_back(Message("Loading desktop...", 2000));

	//PUT FLAG OVERRIDES HERE
	//read_email["rosecolored4bold.png"] = true;
	update_in_progress = false;
	//entered_contest = true;
	//END OVERRIDES
}

void Computer::window_resized(int new_width, int new_height)
{
	width = new_width;
	height = new_height;
	grid_width = width / DENIAL_CHAR_WIDTH;
	grid_height = height / DENIAL_CHAR_HEIGHT;
}

void Computer::setup_view() //Depth test?
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, width, 0.0, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); //Do we need to do this?
}

void Computer::draw_messages(float xtrans, float ytrans, float scale)
{
	//Presently relying on the fact that display_callback() clears the screen to black
	glDisable(GL_LIGHTING);
	for (int i = 0; i < char_grid.size(); ++i)
	{
		for (int j = 0; j < char_grid[i].size(); ++j)
		{
			texture_manager->change_texture("font.bmp");
			int absolute_pos = 0;
			char current_char = char_grid[i][j];
			if ('A' <= current_char && current_char <= 'Z')
				absolute_pos = current_char - 'A';

			else if ('a' <= current_char && current_char <= 'z')
				absolute_pos = current_char - 'a' + 26;

			else if ('0' <= current_char && current_char <= '9')
				absolute_pos = current_char - '0' + 52;

			else if (current_char == '.')
				absolute_pos = 62;

			else if (current_char == '\'')
				absolute_pos = 69;

			else if (current_char == '_')
				absolute_pos = 73;

			else if (current_char == '(')
				absolute_pos = 79;

			else if (current_char == ')')
				absolute_pos = 80;

			else if (current_char == '[')
				absolute_pos = 89;

			else if (current_char == ']')
				absolute_pos = 90;

			else if (current_char == ':')
				absolute_pos = 84;

			else if (current_char == '-')
				absolute_pos = 82;

			else if (current_char == '/')
				absolute_pos = 83;

			else if (current_char == '>')
				absolute_pos = 88;

			else if (current_char == -1)
				absolute_pos = 90;

			int y_coord = 6 - (absolute_pos / 14);
			int x_coord = absolute_pos % 14;
			float s1 = (float) (DENIAL_WIDTH_OFFSET + x_coord * DENIAL_CHAR_WIDTH) / 468.0;
			float t1 = (float) (y_coord * DENIAL_CHAR_HEIGHT) / 465.0;
			float s2 = (float) (DENIAL_WIDTH_OFFSET + (x_coord + 1) * DENIAL_CHAR_WIDTH) / 468.0;
			float t2 = (float) ((y_coord + 1) * DENIAL_CHAR_HEIGHT) / 465.0;
			float x1 = (float) (j * DENIAL_CHAR_WIDTH * scale);
			float y1 = (float) (height - (DENIAL_CHAR_HEIGHT * scale * (i + 1)));
			float x2 = (float) ((j + 1) * DENIAL_CHAR_WIDTH * scale);
			float y2 = (float) (height - (DENIAL_CHAR_HEIGHT * scale * i));
			if (current_char == ' ')
				s1 = s2 = t1 = t2 = 0.0;
			x1 += xtrans;
			x2 += xtrans;
			y1 += ytrans;
			y2 += ytrans;
			if (current_char == -1)
			{
				texture_manager->change_texture("white.png");
				s1 = t1 = 0;
				s2 = t2 = 1;
			}

			glBegin(GL_QUADS);
				glTexCoord2f(s1, t1); glVertex2f(scalex(x1), scaley(y1));
				glTexCoord2f(s1, t2); glVertex2f(scalex(x1), scaley(y2));
				glTexCoord2f(s2, t2); glVertex2f(scalex(x2), scaley(y2));
				glTexCoord2f(s2, t1); glVertex2f(scalex(x2), scaley(y1));
			glEnd();
		}
	}
}

void Computer::draw_desktop()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_TEXTURE_2D); //Still curious about why this is needed but eyelids don't need it
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	//TODO draw_string is slow right now
	/*draw_string(texture_manager, 32, "Internet", 50.0, 870.0);
	draw_string(texture_manager, 32, "Files", 50.0, 770.0);
	draw_string(texture_manager, 32, "Console", 50.0, 670.0);*/
	bool pop_matrix = false;
	bool started_browser = false;
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (play_minimizing_animation && screen_elements[i]->application == minimizing_application && screen_elements[i]->name.substr(0, 5) != "icon_") //Without the icon_ check it doesn't draw, that's weird
		{
			glBindFramebuffer(GL_FRAMEBUFFER, minimizing_framebuffer);
			glPushMatrix();
			glTranslatef(-min_x1, -min_y1, 0.0);
			pop_matrix = true;
		}

		else
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		if (screen_elements[i]->application == BROWSER && !started_browser)
			started_browser = true;

		if (started_browser && screen_elements[i]->application != BROWSER && greyed_out)
		{
			ScreenElement grey(0, 56, 1920, 1000, "greyout.png");
			grey.draw(texture_manager);
			started_browser = false;
		}

		if (screen_elements[i]->is_visible)
			screen_elements[i]->draw(texture_manager);

		if (pop_matrix)
			glPopMatrix();
	}

	if (draw_arrow_thing)
	{
		draw_arrow_thing = false;
		ScreenElement arrow_thing(scroll_lock_x - 20, scroll_lock_y - 20, scroll_lock_x + 20, scroll_lock_y + 20, "arrowthing.png");
		arrow_thing.draw(texture_manager);
	}

	//draw_string(texture_manager, 200, "Here is some text", 500, 500, true, true);

	if (play_minimizing_animation)
	{
		glBindTexture(GL_TEXTURE_2D, minimizing_texture);
		glBegin(GL_QUADS);
			glColor4f(1.0, 1.0, 1.0, minimizing_alpha);
			glTexCoord2f(0.0, 0.0); glVertex2f(cur_min_x1, cur_min_y1);
			glTexCoord2f(0.0, 1.0); glVertex2f(cur_min_x1, cur_min_y2);
			glTexCoord2f(1.0, 1.0); glVertex2f(cur_min_x2, cur_min_y2);
			glTexCoord2f(1.0, 0.0); glVertex2f(cur_min_x2, cur_min_y1);
		glEnd();
	} //TODO DELETE

	else
	{
		glDeleteTextures(1, &minimizing_texture);
		glDeleteFramebuffers(1, &minimizing_framebuffer);
	}

	//draw_string(texture_manager, 32, "cmmx " + to_string(cmmx) + " cmmy " + to_string(cmmy), 10.0, 750.0);

	/*cmmy = 1080.0 - cmmy;
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glVertex3f(cmmx, cmmy, -0.1);
	glVertex3f(cmmx + 10.0, cmmy, -0.1);
	glVertex3f(cmmx + 10.0, cmmy + 10.0, -0.1);
	glVertex3f(cmmx, cmmy + 10.0, -0.1);
	glEnd();*/

	glDisable(GL_TEXTURE_2D);
	//
	glColor4f(1.0, 0.0, 0.0, 1.0);
	if (forcing_cursor)
	{
		glBegin(GL_LINES);
		glVertex2f(-1, first_m * -1 + first_b);
		glVertex2f(1921, first_m * 1921 + first_b);
		glVertex2f(-1, second_m * -1 + second_b);
		glVertex2f(1921, second_m * 1921 + second_b);
		glVertex2f(-1, movable_m * -1 + movable_b);
		glVertex2f(1921, movable_m * 1921 + movable_b);
	}
	glColor4f(1.0, 1.0, 1.0, 1.0);
	//
	glEnable(GL_DEPTH_TEST); //was above texture
	glBegin(GL_QUADS);
		glColor4f(0.0, 0.0, 0.0, alpha);
		glVertex3f(0.0, 0.0, -0.1);
		glVertex3f(width, 0.0, -0.1);
		glVertex3f(width, height, -0.1);
		glVertex3f(0.0, height, -0.1);
	glEnd();
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

void Computer::draw()
{
	if (computer_state == STATE_STARTUP)
		draw_messages();

	else if (computer_state == STATE_DESKTOP)
		draw_desktop();
}

void Computer::animate_messages()
{
	static int frames = 0;
	frames++;
	//old place

	if (frames % 20 == 0)
	{
		auto row = char_grid[char_grid.size() - 1];
		if (row[row.size() - 1] == -1)
			char_grid[char_grid.size() - 1].pop_back();
	}

	else if (frames % 10 == 0)
		char_grid[char_grid.size() - 1].push_back(-1);

	int current_time = glutGet(GLUT_ELAPSED_TIME);
	current_wait -= (current_time - prev_time);
	prev_time = current_time;
	if (current_wait <= 0)
	{
		if (char_grid.size() != 0)
		{
			if (char_grid[char_grid.size() - 1][char_grid[char_grid.size() - 1].size() - 1] == -1)
				char_grid[char_grid.size() - 1].pop_back();
		}

		if (messages.size() == 0)
		{
			computer_state = STATE_DESKTOP;
			reset_computer();
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
			return; //need this?
		}

		Message message = messages[0];
		messages.erase(messages.begin());
		current_wait = message.delay_ms;
		vector<char> current_row;
		for (int i = 0; i < message.text.size(); ++i)
		{
			current_row.push_back(message.text[i]);
			if (current_row.size() == grid_width || i == message.text.size() - 1)
			{
				char_grid.push_back(current_row);
				current_row.clear();
				if (char_grid.size() > grid_height)
					char_grid.erase(char_grid.begin());
			}
		}
	}
}

void Computer::animate_desktop()
{
	if ( (alpha > 0.0 && alpha_increment < 0) || (alpha < 1.0 && alpha_increment > 0) )
		alpha += alpha_increment;

	if (alpha == 1.0)
		computer_done = true;
}

void Computer::animate()
{
	if (playing_escape_sound)
		play_escape_sound();
	
	if (play_minimizing_animation)
	{
		if (alpha_going_up)
		{
			minimizing_alpha += 0.05;
			cur_min_y1 += 20;
			cur_min_y2 += 25;
			int inc_x1 = copysign(20, min_x1 - cur_min_x1);
			int inc_x2 = copysign(20, min_x2 - cur_min_x2);
			if (abs(target_minimize_x - min_x1) > abs(target_minimize_x - min_x2))
				inc_x1 += copysign(5, min_x1 - cur_min_x1);

			else
				inc_x2 += copysign(5, min_x2 - cur_min_x2);

			if (copysign(1, (cur_min_x1 + inc_x1) - min_x1) == copysign(1, cur_min_x1 - min_x1))
				cur_min_x1 += inc_x1;

			if (copysign(1, (cur_min_x2 + inc_x2) - min_x2) == copysign(1, cur_min_x2 - min_x2))
				cur_min_x2 += inc_x2;

			if (minimizing_alpha >= 1.0)
				play_minimizing_animation = false;
		}

		else
		{
			minimizing_alpha -= 0.05;
			cur_min_y1 -= 20;
			cur_min_y2 -= 25;
			int inc_x1 = copysign(20, target_minimize_x - cur_min_x1);
			int inc_x2 = copysign(20, target_minimize_x - cur_min_x2);
			if (abs(target_minimize_x - cur_min_x1) > abs(target_minimize_x - cur_min_x2))
				inc_x1 += copysign(5, target_minimize_x - cur_min_x1);

			else
				inc_x2 += copysign(5, target_minimize_x - cur_min_x2);

			if (copysign(1, (cur_min_x1 + inc_x1) - target_minimize_x) == copysign(1, cur_min_x1 - target_minimize_x))
				cur_min_x1 += inc_x1;

			if (copysign(1, (cur_min_x2 + inc_x2) - target_minimize_x) == copysign(1, cur_min_x2 - target_minimize_x))
				cur_min_x2 += inc_x2;

			if (minimizing_alpha <= 0.0)
				play_minimizing_animation = false;
		}
	}
	
	if (computer_state == STATE_STARTUP)
		animate_messages();

	else if (computer_state == STATE_DESKTOP)
	{
		animate_desktop();
		for (int i = 0; i < screen_elements.size(); ++i)
			screen_elements[i]->animate();
	}
}

void Computer::go_to_desktop()
{
	//Watch out for memory leaks
	//Do you want to actually call close_application() on everything?
	computer_state = STATE_DESKTOP;
	applications_open = 0;
	screen_elements.clear();
	screen_elements.push_back(make_unique<ScreenElement>(0.0, 0.0, 1920.0, 1080.0, "background.png"));
	screen_elements.push_back(make_unique<DesktopIcon>(50.0, 950.0 - (50.0 * 1.117), 100.0, 950.0, "icon_internet_text.png", this, BROWSER));
	screen_elements.push_back(make_unique<DesktopIcon>(50.0, 850.0 - (50.0 * 1.117), 100.0, 850.0, "icon_folder_text.png", this, FILES));
	screen_elements.push_back(make_unique<DesktopIcon>(50.0, 750.0 - (50.0 * 1.117), 100.0, 750.0, "icon_console_text.png", this, CONSOLE));
	//screen_elements.push_back(make_unique<DesktopIcon>(50.0, 600.0, 100.0, 650.0, "icon_paint.png", this, PAINT));
	for (int i = 0; i < extra_apps.size(); ++i)
		screen_elements.push_back(make_unique<DesktopIcon>(50.0, (6 - i)*100.0, 100.0, (6 - i)*100.0 + 50.0, extra_apps[i].first, this, extra_apps[i].second));

	screen_elements.push_back(make_unique<ScreenElement>(0.0, 0.0, 1920.0, 56.0, "beige2.png"));
	screen_elements.push_back(make_unique<PowerSymbol>(7.0, 0.0, 57.0, 50.0, "powersymbol1.png", this));
	focus_element = screen_elements[0].get();
}

void Computer::go_to_sleep()
{
	/*if (read_email["rosecolored2bold.png"])
		looked_for_spider = true;*/ //TODO THIS WAS THE ORIGINAL CHECK

	going_to_sleep = true;
	playing_escape_sound = false;
	alpha_increment = ALPHA_INCREMENT;
}

void Computer::reset_computer()
{
	computer_done = false;
	going_to_sleep = false;
	alpha_increment = -ALPHA_INCREMENT;
	if (computer_state == STATE_DESKTOP)
		go_to_desktop();
}

//TODO maybe upclick on different element than focus element goes focusless and does not focus on "clicked" element
void Computer::mouse_clicked(int button, int state, int x, int y)
{
	if (going_to_sleep)
		return;

	bool have_info_box = false;
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (screen_elements[i].get()->application == INFO_BOX)
			have_info_box = true;
	}
	
	to_be_added.clear();
	if (read_email["rosecolored3bold.png"] && !started_crashing_browser)
	{
		std::this_thread::sleep_for(std::chrono::seconds(4));
		start_application(INFO_BOX, "Browser has encountered a problem and needs to close.");
		started_crashing_browser = true;
		have_info_box = true; //QUESTIONABLE
	}

	y = glutGet(GLUT_WINDOW_HEIGHT) - y;
	x = (int)((float)x * (1920.0 / (float)glutGet(GLUT_WINDOW_WIDTH)));
	y = (int)((float)y * (1080.0 / (float)glutGet(GLUT_WINDOW_HEIGHT)));
	bool already_clicked = false;
	for (int i = screen_elements.size() - 1; i >= 0; --i)
	{
		ScreenElement* se = screen_elements[i].get();
		bool did_scale = false;
		if (se->no_scale)
		{
			x = scalex(x);
			y = scaley(y);
			did_scale = true;
		}

		if (have_info_box && se->application != INFO_BOX)
			continue;

		if (!se->is_visible)
			continue;

		if (state == GLUT_DOWN)
		{
			if (se->x1 <= x && x <= se->x2 && se->y1 <= y && y <= se->y2 && !already_clicked && !se->never_focus)
			{
				se->mouse_clicked(button, state, x, y);
				se->give_focus();
				focus_element = se;
				already_clicked = true;
			}

			else
				se->take_focus();
		}

		else if (state == GLUT_UP)
		{
			if (se->has_focus)
			{
				if (se->x1 <= x && x <= se->x2 && se->y1 <= y && y <= se->y2)
					se->mouse_clicked(button, state, x, y);

				else
				{
					if (se->receive_out_of_bounds_clicks)
						se->mouse_clicked(button, state, x, y);

					if (se->surrender_focus_on_up_click)
					{
						se->take_focus();
						focus_element = screen_elements[0].get();
					}
				}
			}
		}

		if (did_scale)
		{
			x = unscalex(x);
			y = unscaley(y);
		}
	}

	process_control_changes();
}

void Computer::process_control_changes()
{
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (screen_elements[i]->transient)
			screen_elements[i]->marked_for_deletion = true;
	}

	this->remove_deleted();
	for (int i = 0; i < to_be_added.size(); ++i)
	{
		if (to_be_added[i]->name.substr(0, 5) == "icon_")
			screen_elements.push_back(std::move(to_be_added[i]));

		else if (to_be_added[i]->browser_child)
		{
			int j = 0;
			for (j = 0; j < screen_elements.size(); ++j)
			{
				if (dynamic_cast<ScrollPanel*>(screen_elements[j].get()))
					break;
			}

			screen_elements.insert(screen_elements.begin() + j + 1, std::move(to_be_added[i]));
		}

		else
			screen_elements.insert(screen_elements.end() - 2 - applications_open, std::move(to_be_added[i]));
	}

	to_be_added.clear();
}

void Computer::start_forcing_cursor(float fm, float fb, float sm, float sb, float mm, float mb, float em, float eb, bool up)
{
	first_m = fm;
	first_b = fb;
	second_m = sm;
	second_b = sb;
	movable_m = mm;
	movable_b = mb;
	end_m = em;
	end_b = eb;
	going_up = up;
	forcing_cursor = true;
}

void Computer::force_to(int x, int y)
{
	float m = (float)(prev_mouse_y - y) / (float)(prev_mouse_x - x);
	float b = y - m*x;
	//start_forcing_cursor(m, b - 30, m, b + 30, -1.0 / m, prev_mouse_y - (-1.0 / m)*prev_mouse_x, -1.0 / m, y - (-1.0 / m)*x, y > prev_mouse_y);
	start_forcing_cursor(m, b - 100, m, b + 100, -1.0 / m, y > prev_mouse_y ? prev_mouse_y - (-1.0 / m)*prev_mouse_x - 5 : prev_mouse_y - (-1.0 / m)*prev_mouse_x, -1.0 / m, y - (-1.0 / m)*x, y > prev_mouse_y);
}

void Computer::warp_cursor(int x, int y)
{
	int warp_x = ceil((float)(movable_b - first_b) / (float)(first_m - movable_m)) + (first_m < 0 ? 20 : -20);
	int warp_y = glutGet(GLUT_WINDOW_HEIGHT) - (movable_m * warp_x + movable_b) + (going_up ? -5 : 5);
	bool warp = false;
	int lower_y = first_m * x + first_b;
	if (y < lower_y)
		warp = true;

	int upper_y = second_m * x + second_b;
	if (y > upper_y)
		warp = true;

	int movable_y = movable_m * x + movable_b;
	int end_y = end_m * x + end_b;
	bool end_of_forcing = false;
	if (going_up)
	{
		if (y < movable_y)
			warp = true;

		if (y > end_y)
			warp = end_of_forcing = true;
	}

	else
	{
		if (y > movable_y)
			warp = true;

		if (y < end_y)
			warp = end_of_forcing = true;
	}

	if (warp)
	{
		if (end_of_forcing)
			end_of_forcing_function();

		glutWarpPointer(warp_x, warp_y);
		return;
	}

	if (abs(end_y - y) > 3 && abs(y - movable_y) >= 7)
		movable_b += (y - movable_y) - copysign(2, y - movable_y);
}

void Computer::mouse_moved(int x, int y)
{
	cmmx = x;
	cmmy = y;
	y = glutGet(GLUT_WINDOW_HEIGHT) - y;
	x = (int)((float)x * (1920.0 / (float)glutGet(GLUT_WINDOW_WIDTH)));
	y = (int)((float)y * (1080.0 / (float)glutGet(GLUT_WINDOW_HEIGHT)));
	if (sticking_cursor && x == sticking_x && y == sticking_y)
		return;

	if (forcing_cursor)
		warp_cursor(x, y);

	if (sticking_cursor)
		glutWarpPointer(sticking_x, glutGet(GLUT_WINDOW_HEIGHT) - sticking_y);

	if (looking_for_stick)
	{
		if (sqrt(pow(x - 700, 2) + pow(y - 750, 2)) > 200)
		{
			sticking_cursor = true;
			sticking_x = x;
			sticking_y = y;
			looking_for_stick = false;
		}
	}

	if (forcing_e2)
	{
		if (y < 56)
			glutWarpPointer(x, 200);

		else if (y > 1044)
			glutWarpPointer(x, 900);
	}

	bool already_moused_over = false;
	someone_set_cursor = false;
	for (int i = screen_elements.size() - 1; i >= 0; --i)
	{
		ScreenElement* se = screen_elements[i].get();
		if (!se->is_visible)
			continue;

		bool did_scale = false;
		if (se->no_scale)
		{
			x = scalex(x);
			y = scaley(y);
			did_scale = true;
		}

		if (se->x1 <= x && x <= se->x2 && se->y1 <= y && y <= se->y2 && !already_moused_over && !se->never_focus)
		{
			se->mouse_over(x, y);
			already_moused_over = true;
		}

		else se->mouse_off();
        
        if (se->has_focus)
            se->mouse_moved(x, y);

		if (did_scale)
		{
			x = unscalex(x);
			y = unscaley(y);
		}
	}

	if (!someone_set_cursor && !maintain_cursor && computer_state == STATE_DESKTOP)
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW);

	prev_mouse_x = x;
	prev_mouse_y = y;
}

//TODO info box on file dialog for paint application closes both windows when X'd out, should only close topmost one
void Computer::start_application(Application application, string info_text, bool file_dialog, bool progress_dialog, int download_action, ScrollPanel* scroll_panel)
{
	if (application != INFO_BOX)
		applications_open++;

	switch (application)
	{
	case BROWSER:
	{
		if (crashed_browser && !recovered_browser)
		{
			start_application(INFO_BOX, "Browser is already running.  Close existing instance.");
			applications_open--;
			return;
		}
		
		auto minimize = minimize_function(BROWSER, "icon_internet.png");
		to_be_added.push_back(make_unique<Button>(65.0 * applications_open, 0.0, (65.0 * applications_open) + 50.0, 50.0, "icon_internet.png", BROWSER, minimize, [](){}, "", "sheen.png", "sheen2.png"));
		unique_ptr<ScrollPanel> scroll_panel = make_unique<ScrollPanel>(0.0, 0.0, 1920.0, 1080.0, this, BROWSER);
		to_be_added.insert(to_be_added.end() - 4, std::move(scroll_panel));
		ScrollPanel* scroll_panel_pointer = reinterpret_cast<ScrollPanel*>(to_be_added[to_be_added.size() - 5].get()); //I am so sorry
		to_be_added.push_back(make_unique<ScreenElement>(1900, 56, 1920, 1044, "scrollbackground.png", BROWSER));
		unique_ptr<ScrollBar> scroll_bar = make_unique<ScrollBar>(1900.0, 0.0, 1920.0, 0.0, "scrollbar.png", BROWSER, scroll_panel_pointer);
		to_be_added.push_back(std::move(scroll_bar));
		ScrollBar* scroll_bar_pointer = reinterpret_cast<ScrollBar*>(to_be_added[to_be_added.size() - 1].get());
		scroll_panel_pointer->scroll_bar = scroll_bar_pointer;
		to_be_added.push_back(make_unique<ScreenElement>(0.0, 1044.0, 1980.0, 1080.0, "silver.png", BROWSER));
		to_be_added.push_back(make_unique<ScreenElement>(10.0, 1050.0, 35.0, 1075.0, "|icon_internet.png", BROWSER));
		to_be_added.push_back(make_unique<ScreenElement>(45, 1050, 45 + 125, 1075, "internet_name.png", BROWSER));
		to_be_added.push_back(make_unique<Button>(1771, 1009, 1875, 1029, "usefulbookmark.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->start_loading_site("useful", false);
		}, [](){}, "", "bookmarkcover.png", "bookmarkcoverdark.png", "", true));
		to_be_added.push_back(make_unique<Button>(1647, 1009, 1746, 1029, "nendabookmark.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->start_loading_site("nenda", false);
		}, [](){}, "", "bookmarkcover.png", "bookmarkcoverdark.png", "", true));
		to_be_added.push_back(make_unique<Button>(1522, 1009, 1622, 1029, "socnewsbookmark.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->start_loading_site("reddit", false);
		}, [](){}, "", "bookmarkcover.png", "bookmarkcoverdark.png", "", true));
		to_be_added.push_back(make_unique<Button>(1456, 1009, 1497, 1029, "ainbutton.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->start_loading_site("ain", false);
		}, []() {}, "", "bookmarkcover.png", "bookmarkcoverdark.png", "", true));
		to_be_added.push_back(make_unique<Button>(1368, 1009, 1431, 1029, "searchbookmark.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->start_loading_site("search", false);
		}, []() {}, "", "bookmarkcover.png", "bookmarkcoverdark.png", "", true));
		to_be_added.push_back(make_unique<ScreenElement>(1275, 1009, 1343, 1029, "bookmarks.png", BROWSER));
		//to_be_added.push_back(make_unique<XButton>(1882.0, 1047.0, 1913.0, 1078.0, "xbutton.png", this, BROWSER));
		to_be_added.push_back(make_unique<Button>(1882.0 + 5, 1047.0, 1913.0 + 5 + 2, 1078.0 + 2, "xbutton.png", BROWSER, [this]() {this->close_application(BROWSER);}, []() {}, "", "xbuttonlight.png", "xbuttondark.png"));
		to_be_added.push_back(make_unique<Button>(1846.0 + 8, 1047.0, 1877.0 + 8 + 2, 1078.0 + 2, "minimize.png", BROWSER, minimize, [](){}, "", "minimizelight.png", "minimizedark.png"));
		to_be_added.push_back(make_unique<Button>(1900.0, 1024.0, 1920.0, 1044.0, "arrow.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->release_key(KEY_UP);
		},
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->press_key(KEY_UP);
		}, "", "genlight.png", "gendark.png"));
		to_be_added.push_back(make_unique<Button>(1900.0, 56.0, 1920.0, 76.0, "arrow2.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->release_key(KEY_DOWN);
		},
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->press_key(KEY_DOWN);
		}, "", "genlight.png", "gendark.png"));
		to_be_added.push_back(make_unique<ScreenElement>(7, 1001, 45, 1040, "holder.png", BROWSER));
		to_be_added.push_back(make_unique<Button>(10.0, 1004.0, 40.0, 1034.0, "back.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->go_back();
		},
			[](){}, "", "backlight.png", "backdark.png"));
		to_be_added.push_back(make_unique<ScreenElement>(83, 1001, 45, 1040, "holder.png", BROWSER));
		to_be_added.push_back(make_unique<Button>(50.0, 1004.0, 80.0, 1034.0, "forward.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->go_forward();
		},
			[](){}, "", "forwardlight.png", "forwarddark.png"));

		//to_be_added.push_back(make_unique<Video>(300.0, 300.0, 800.0, 800.0, "blue.png", BROWSER));
		//to_be_added.push_back(make_unique<BubbleGame>(0.0, 56.0, 1920.0, 1000.0, "black.png", this, BROWSER));
		scroll_panel_pointer->change_website("search");
		break;
	}

	case FILES:
	{
		if (update_in_progress)
		{
			start_application(INFO_BOX, "Cannot use system services while system update is in progress.");
			applications_open--;
			return;
		}

		Application target;
		if (file_dialog)
		{
			target = INFO_BOX;
			applications_open--;
		}

		else
			target = FILES;
		
		auto minimize = minimize_function(FILES, "icon_folder.png");
		if (!file_dialog)
			to_be_added.push_back(make_unique<Button>(65.0 * applications_open, 0.0, (65.0 * applications_open) + 50.0, 50.0, "icon_folder.png", FILES, minimize_function(FILES, "icon_folder.png"), [](){}, "", "sheen.png", "sheen2.png"));

		to_be_added.push_back(make_unique<ScreenElement>(760.0 - 300, 744.0, 1160.0, 780.0, "silver.png", target)); //title bar
		to_be_added.push_back(make_unique<ScreenElement>(760.0 - 300 + 10, 750.0, 760 - 300 + 35, 775.0, "|icon_folder.png", target));
		to_be_added.push_back(make_unique<ScreenElement>(760 - 300 + 45, 750, 760 - 300 + 45 + 125, 775, "files_name.png", target));
		//to_be_added.push_back(make_unique<XButton>(1122.0, 747.0, 1153.0, 778.0, "xbutton.png", this, target));
		to_be_added.push_back(make_unique<Button>(1122.0, 747.0, 1153.0, 778.0, "xbutton.png", target, [this, target]() {this->close_application(target);}, []() {}, "", "xbuttonlight.png", "xbuttondark.png"));
		if (!file_dialog)
			to_be_added.push_back(make_unique<Button>(1086.0, 747.0, 1117.0, 778.0, "minimize.png", FILES, minimize, [](){}, "", "minimizelight.png", "minimizedark.png"));

		to_be_added.push_back(make_unique<ScreenElement>(760.0 - 300, file_dialog ? 394 - 120 : 394.0 - 60, 770.0 - 300, 744.0, "silver.png", target)); //left
		to_be_added.push_back(make_unique<ScreenElement>(1150.0, file_dialog ? 394 - 120 : 394.0 - 60, 1160.0, 744.0, "silver.png", target)); //right
		to_be_added.push_back(make_unique<ScreenElement>(760.0 - 300, file_dialog ? 394 - 120 : 394.0 - 60, 1160.0, file_dialog ? 744 - 60 : 744.0, "silver.png", target)); //bottom
		to_be_added.push_back(make_unique<ScreenElement>(770.0 - 300, 744 - 60, 1150, 744, "beigeold.png", target));
		if (file_dialog)
			to_be_added.push_back(make_unique<ScreenElement>(770 - 300, 394 - 120 + 8, 1150, 394 - 120 + 8 + 60, "beigeold.png", target));

		if (!file_dialog)
		{
			auto files = make_unique<Files>(770.0 - 300, 404.0 - 60, 1150.0, 744.0 - 60, "white.png", this, target);
			to_be_added.insert(to_be_added.end() - 1, std::move(files));
		}

		break;
	}

	case CONSOLE:
	{
		if (update_in_progress)
		{
			start_application(INFO_BOX, "Cannot use system services while system update is in progress.");
			applications_open--;
			return;
		}

		auto minimize = minimize_function(CONSOLE, "icon_console.png");
		to_be_added.push_back(make_unique<Button>(65.0 * applications_open, 0.0, (65.0 * applications_open) + 50.0, 50.0, "icon_console.png", CONSOLE, minimize_function(CONSOLE, "icon_console.png"), [](){}, "", "sheen.png", "sheen2.png"));
		to_be_added.push_back(make_unique<ScreenElement>(560.0, 744.0, 1360.0, 780.0, "silver.png", CONSOLE));
		to_be_added.push_back(make_unique<ScreenElement>(570.0, 750.0, 595.0, 775.0, "|icon_console.png", CONSOLE));
		to_be_added.push_back(make_unique<ScreenElement>(605, 750, 605 + 125, 775, "console_name.png", CONSOLE));
		//to_be_added.push_back(make_unique<XButton>(1322.0, 747.0, 1353.0, 778.0, "xbutton.png", this, CONSOLE));
		to_be_added.push_back(make_unique<Button>(1322.0, 747.0, 1353.0, 778.0, "xbutton.png", CONSOLE, [this]() {this->close_application(CONSOLE);}, []() {}, "", "xbuttonlight.png", "xbuttondark.png"));
		to_be_added.push_back(make_unique<Button>(1286.0, 747.0, 1317.0, 778.0, "minimize.png", CONSOLE, minimize, [](){}, "", "minimizelight.png", "minimizedark.png"));
		to_be_added.push_back(make_unique<ScreenElement>(560.0, 394.0, 570.0, 744.0, "silver.png", CONSOLE));
		to_be_added.push_back(make_unique<ScreenElement>(1350.0, 394.0, 1360.0, 744.0, "silver.png", CONSOLE));
		to_be_added.push_back(make_unique<ScreenElement>(560.0, 394.0, 1360.0, 404.0, "silver.png", CONSOLE));
		to_be_added.push_back(make_unique<Console>(570.0, 404.0, 1350.0, 744.0, "black.png", this));
		break;
	}

	case INFO_BOX:
	{
		if (scroll_panel != NULL)
		{
			to_be_added.push_back(make_unique<InfoText>(560.0, 470.0, 560 + 360, 470 + 200, "browserdialog.png", info_text));
			to_be_added.push_back(make_unique<Button>(660, 475, 660 + 150, 475 + 40, "browserok.png", INFO_BOX, [this, scroll_panel]()
			{
				this->close_application(INFO_BOX);
				this->greyed_out = false;
			}, [](){}, "", "browserheld.png", "browserpressed.png"));
			auto deltas = to_be_added[to_be_added.size() - 2]->center();
			to_be_added[to_be_added.size() - 1]->translate(deltas.first, deltas.second);
			return;
		}
		
		to_be_added.push_back(make_unique<ScreenElement>(560.0, 704.0, 1360.0, 740.0, "silver.png", INFO_BOX));
		if (!progress_dialog)
			//to_be_added.push_back(make_unique<XButton>(1322.0, 707.0, 1353.0, 738.0, "xbutton.png", this, INFO_BOX));
			to_be_added.push_back(make_unique<Button>(1322.0, 707.0, 1353.0, 738.0, "xbutton.png", INFO_BOX, [this]() {this->close_application(INFO_BOX);}, []() {}, "", "xbuttonlight.png", "xbuttondark.png"));

		to_be_added.push_back(make_unique<ScreenElement>(560.0, 460.0, 570.0, 704.0, "silver.png", INFO_BOX));
		to_be_added.push_back(make_unique<ScreenElement>(1350.0, 460.0, 1360.0, 704.0, "silver.png", INFO_BOX));
		to_be_added.push_back(make_unique<ScreenElement>(560.0, 460.0, 1360.0, 470.0, "silver.png", INFO_BOX));
		float whitex1 = 570;
		float whitex2 = 1350;
		to_be_added.push_back(make_unique<InfoText>(whitex1, 470.0, whitex2, 704.0, "white.png", info_text));
		if (!progress_dialog)
		{
			/*if (scroll_panel != NULL)
			{
				to_be_added.push_back(make_unique<Button>(800, 500, 1100, 550, "ok.png", INFO_BOX, [this, scroll_panel]()
				{
					this->close_application(INFO_BOX);
					this->greyed_out = false;
				}, []() {}));
			}

			else*/ to_be_added.push_back(make_unique<Button>((whitex1 + whitex2) / 2 - 81, 500.0, (whitex1 + whitex2) / 2 + 81, 500 + 43, "ok.png", INFO_BOX, [this]() {this->close_application(INFO_BOX);}, []() {}));
		}

		else
			to_be_added.push_back(make_unique<ProgressBar>(600.0, 500.0, 1300.0, 550.0, "beige.png", this, INFO_BOX, [this, download_action]()
		{
			if (download_action == 0)
			{
				this->extra_apps.push_back(pair<string, Application>("icon_paint_text.png", PAINT));
				this->screen_elements.insert(screen_elements.begin() + 4, make_unique<DesktopIcon>(50.0, 600.0, 100.0, 650.0, "icon_paint_text.png", this, PAINT));
			}

			else if (download_action == 1)
			{
				this->extra_apps.push_back(pair<string, Application>("icon_video.png", VIDEO_APP));
				this->screen_elements.insert(screen_elements.begin() + 4, make_unique<DesktopIcon>(50.0, 500.0, 100.0, 550.0, "icon_video.png", this, VIDEO_APP));
			}

			else if (download_action == 2)
			{
				this->extra_apps.push_back(pair<string, Application>("icon_decrypt.png", DECRYPTION_APP));
				this->screen_elements.insert(screen_elements.begin() + 4, make_unique<DesktopIcon>(50.0, 200.0, 100.0, 250.0, "icon_decrypt.png", this, DECRYPTION_APP));
			}

			else if (download_action == 3)
			{
				this->extra_apps.push_back(pair<string, Application>("icon_mmo.png", MMO_GAME));
				this->screen_elements.insert(screen_elements.begin() + 4, make_unique<DesktopIcon>(50, 400, 100, 450, "icon_mmo.png", this, MMO_GAME));
			}
		}));

		break;
	}

	case PAINT:
	{
		auto minimize = minimize_function(PAINT, "icon_paint.png");
		to_be_added.push_back(make_unique<Button>(65.0 * applications_open, 0.0, (65.0 * applications_open) + 50.0, 50.0, "icon_paint.png", PAINT, minimize_function(PAINT, "icon_paint.png"), [](){}, "", "sheen.png", "sheen2.png"));
		to_be_added.push_back(make_unique<ScreenElement>(0.0, 1044.0, 1980.0, 1080.0, "silver.png", PAINT));
		to_be_added.push_back(make_unique<ScreenElement>(0.0, 980.0, 1920.0, 1044.0, "beige.png", PAINT));
		//to_be_added.push_back(make_unique<XButton>(1882.0, 1047.0, 1913.0, 1078.0, "xbutton.png", this, PAINT));
		to_be_added.push_back(make_unique<Button>(1882.0 + 5, 1047.0, 1913.0 + 5 + 2, 1078.0 + 2, "xbutton.png", PAINT, [this]() {this->close_application(PAINT);}, []() {}, "", "xbuttonlight.png", "xbuttondark.png"));
		to_be_added.push_back(make_unique<Button>(1846.0 + 8, 1047.0, 1877.0 + 8 + 2, 1078.0 + 2, "minimize.png", PAINT, minimize, [](){}, "", "minimizelight.png", "minimizedark.png"));
		to_be_added.push_back(make_unique<ScreenElement>(0.0, 56.0, 1920.0, 980.0, "paintbackground.png", PAINT));
		//to_be_added.push_back(make_unique<Canvas>(200.0, 200.0, 800.0, 800.0, "white.png", this, PAINT));
		auto canvas = make_unique<Canvas>(20.0, 200.0, 800.0, 960.0, "white.png", this, PAINT);
		Canvas* canvas_ptr = canvas.get();
		to_be_added.push_back(std::move(canvas));
		to_be_added.push_back(make_unique<Resizer>(0, 0, 0, 0, "white.png", PAINT, canvas_ptr, 0, this));
		to_be_added.push_back(make_unique<Resizer>(0, 0, 0, 0, "white.png", PAINT, canvas_ptr, 1, this));
		to_be_added.push_back(make_unique<Resizer>(0, 0, 0, 0, "white.png", PAINT, canvas_ptr, 2, this));
		vector<unique_ptr<Button>> buttons;
		/*buttons.push_back(make_unique<Button>(60.0, 990, 60 + 50, 1040.0, "ellipse.png", PAINT, [canvas_ptr]() {canvas_ptr->change_tool(Canvas::ELLIPSE, "ellipse.png");}, []() {}, "", "genlight.png", "gendark.png", "ellipsetip.png"));
		buttons.push_back(make_unique<Button>(100.0, 1004.0, 136.0, 1040.0, "fill.png", PAINT, [canvas_ptr]() {canvas_ptr->change_tool(Canvas::FILL, "fill.png");}, []() {}, "", "genlight.png", "gendark.png", "filltip.png"));
		buttons.push_back(make_unique<Button>(140.0, 1004.0, 176.0, 1040.0, "filledrectangle.png", PAINT, [canvas_ptr]() {canvas_ptr->change_tool(Canvas::FILLED_RECTANGLE, "filledrectangle.png");}, []() {}, "", "genlight.png", "gendark.png", "fillrectangletip.png"));
		buttons.push_back(make_unique<Button>(180.0, 1004.0, 206.0, 1040.0, "line.png", PAINT, [canvas_ptr]() {canvas_ptr->change_tool(Canvas::LINE, "line.png");}, []() {}, "", "genlight.png", "gendark.png", "linetip.png"));
		buttons.push_back(make_unique<Button>(210.0, 1004.0, 246.0, 1040.0, "paintbrush.png", PAINT, [canvas_ptr]() {canvas_ptr->change_tool(Canvas::BRUSH, "paintbrush.png");}, []() {}, "", "genlight.png", "gendark.png", "brushtip.png"));
		buttons[buttons.size() - 1]->dark = true;
		buttons.push_back(make_unique<Button>(250.0, 1004.0, 286.0, 1040.0, "rectangle.png", PAINT, [canvas_ptr]() {canvas_ptr->change_tool(Canvas::RECTANGLE, "rectangle.png");}, []() {}, "", "genlight.png", "gendark.png", "rectangletip.png"));
		buttons.push_back(make_unique<Button>(290.0, 1004.0, 326.0, 1040.0, "select.png", PAINT, [canvas_ptr]() {canvas_ptr->change_tool(Canvas::SELECT, "select.png");}, []() {}, "", "genlight.png", "gendark.png", "selecttip.png"));
		buttons.push_back(make_unique<Button>(330.0, 1004.0, 366.0, 1040.0, "stickylines.png", PAINT, [canvas_ptr]() {canvas_ptr->change_tool(Canvas::STICKY_LINES, "stickylines.png");}, []() {}, "", "genlight.png", "gendark.png", "stickylinestip.png"));
		buttons.push_back(make_unique<Button>(370.0, 1004.0, 406.0, 1040.0, "save.png", PAINT, [canvas_ptr]() {canvas_ptr->save_file();}, []() {}, "", "genlight.png", "gendark.png", "savetip.png"));
		buttons.push_back(make_unique<Button>(410.0, 1004.0, 446.0, 1040.0, "open.png", PAINT, [canvas_ptr]() {canvas_ptr->load_file();}, []() {}, "", "genlight.png", "gendark.png", "opentip.png"));*/
		int x1 = 60;
		int y1 = 990;
		int y2 = 1040;
		int x2 = 110;
		vector<string> tool_names = { "paintbrush", "fill", "filledrectangle", "line", "ellipse", "rectangle", "select", "stickylines", "colorpicker", "save", "open" };
		for (int i = 0; i < tool_names.size(); ++i)
		{
			string tool_name = tool_names[i];
			buttons.push_back(make_unique<Button>(x1, y1, x2, y2, tool_names[i] + ".png", PAINT, [canvas_ptr, tool_name]() {canvas_ptr->change_tool(tool_name);}, []() {}, "", "genlight.png", "gendark.png", tool_name + "tip.png"));
			x1 += 55;
			x2 += 55;
		}

		buttons[buttons.size() - 2]->release_function = [canvas_ptr]() {canvas_ptr->save_file();};
		buttons[buttons.size() - 1]->release_function = [canvas_ptr]() {canvas_ptr->load_file();};
		buttons[0]->dark = true;
		for (int i = buttons.size() - 1; i >= 0; --i)
		{
			canvas_ptr->tools.push_back(buttons[i].get());
			to_be_added.push_back(std::move(buttons[i]));
		}

		to_be_added.push_back(make_unique<ScreenElement>(10.0, 1050.0, 35.0, 1075.0, "|icon_paint.png", PAINT));
		to_be_added.push_back(make_unique<ScreenElement>(45, 1050, 45 + 125, 1075, "paint_name.png", PAINT));

		//to_be_added.push_back(make_unique<MMO>(0.0, 56.0, 1920.0, 1044.0, "black.png", this, PAINT));
		// // auto mmo = make_unique<MMO>(0.0, 56.0, 1920.0, 1044.0, "black.png", this, PAINT);
		// // to_be_added.insert(to_be_added.end() - 3, std::move(mmo));
		// auto help_center = make_unique<HelpCenter>(0.0, 56.0, 1920.0, 1044.0, "white.png", this, PAINT);
		// to_be_added.insert(to_be_added.end() - 2, std::move(help_center));
		// unique_ptr<NumberField> number_field = make_unique<NumberField>(300.0, 300.0, 500.0, 400.0, "textfield.png", this, PAINT);
		// to_be_added.insert(to_be_added.end() - 2, std::move(number_field)); //2 is because the NumberField will add two children it needs to be beneath
		break;
	}

	case VIDEO_APP:
	{
		screen_elements.insert(screen_elements.end(), make_unique<ScreenElement>(0, 0, 1920, 1080, "black.png", VIDEO_APP));
		screen_elements.insert(screen_elements.end(), make_unique<Video>(0, 540, 960, 1080, "blue.png", VIDEO_APP, this));
		break;
	}

	case DECRYPTION_APP:
	{
		auto minimize = minimize_function(DECRYPTION_APP, "icon_decrypt.png");
		to_be_added.push_back(make_unique<Button>(65.0 * applications_open, 0.0, (65.0 * applications_open) + 50.0, 50.0, "icon_decrypt.png", DECRYPTION_APP, minimize, [](){}, "", "sheen.png", "sheen2.png"));
		to_be_added.push_back(make_unique<Decryption>(500, 500, 1000, 1000, this, DECRYPTION_APP));
		break;
	}

	case MMO_GAME:
	{
		auto mmo = make_unique<MMO>(0, 0, 1920, 1080, "black.png", "level2.txt", this, MMO_GAME);
		MMO* mmo_ptr = mmo.get();
		screen_elements.insert(screen_elements.end(), std::move(mmo));
		mmo_ptr->add_children();
		break;
	}
	}
}

std::function<void()> Computer::minimize_function(Application application, string icon_name)
{
	return [this, application, icon_name]()
	{
		min_x1 = -1; min_x2 = -1; min_y1 = -1; min_y2 = -1; //TODO is this good?
		int start = -1;
		int end;
		for (int i = 0; i < screen_elements.size(); ++i)
		{
			if (screen_elements[i]->application == application)
			{
				if (start == -1)
					start = i;
			}

			else
			{
				if (start != -1)
				{
					end = i;
					break;
				}
			}
		}

		if (end == screen_elements.size() - 2 - applications_open)
		{
			move_range(screen_elements, start, end, 0);
			alpha_going_up = false;
			minimizing_alpha = 1.0;
		}

		else
		{
			move_range(screen_elements, start, end, -2 - applications_open);
			alpha_going_up = true;
			minimizing_alpha = 0.0;
		}
		
		play_minimizing_animation = true;
		minimizing_application = application;
		for (int i = 0; i < screen_elements.size(); ++i)
		{
			auto se = screen_elements[i].get();
			if (se->application != application || se->name.substr(0, 5) == "icon_")
				continue;

			if (se->x1 < min_x1 || min_x1 == -1)
				min_x1 = se->x1;

			if (se->x2 > min_x2 || min_x2 == -1)
				min_x2 = se->x2;

			if (se->y1 < min_y1 || min_y1 == -1)
				min_y1 = se->y1;

			if (se->y2 > min_y2 || min_y2 == -1)
				min_y2 = se->y2;
		}

		int other_apps = 0;
		for (int i = screen_elements.size() - 1; i >= 0; i--)
		{
			if (screen_elements[i]->name == icon_name)
				break;

			if (screen_elements[i]->name.substr(0, 5) == "icon_")
				other_apps++;
		}

		target_minimize_x = (applications_open - other_apps) * 50 + 25;
		
		if (!alpha_going_up)
		{
			cur_min_x1 = min_x1;
			cur_min_x2 = min_x2;
			cur_min_y1 = min_y1;
			cur_min_y2 = min_y2;
		}

		else
		{
			cur_min_y1 = min_y1 - (20 * 20);
			cur_min_y2 = min_y2 - (25 * 20);
			int inc_x1 = copysign(20, target_minimize_x - min_x1);
			int inc_x2 = copysign(20, target_minimize_x - min_x2);
			if (abs(target_minimize_x - min_x1) > abs(target_minimize_x - min_x2))
				inc_x1 += copysign(5, target_minimize_x - min_x1);

			else
				inc_x2 += copysign(5, target_minimize_x - min_x2);

			if (copysign(1, (min_x1 + inc_x1 * 20) - target_minimize_x) == copysign(1, min_x1 - target_minimize_x))
				cur_min_x1 = min_x1 + inc_x1 * 20;

			else
				cur_min_x1 = target_minimize_x;

			if (copysign(1, (min_x2 + inc_x2 * 20) - target_minimize_x) == copysign(1, min_x2 - target_minimize_x))
				cur_min_x2 = min_x2 + inc_x2 * 20;

			else
				cur_min_x2 = target_minimize_x;
		}

		glGenFramebuffers(1, &minimizing_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, minimizing_framebuffer);
		glGenTextures(1, &minimizing_texture);
		glBindTexture(GL_TEXTURE_2D, minimizing_texture); //TODO I hope this doesn't break any assumptions about textures
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, min_x2 - min_x1, min_y2 - min_y1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); //TODO cuts off one pixel?  Have I done this everywhere?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //TODO check these
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, minimizing_texture, 0);
		GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, draw_buffers);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	};
}

template<typename T> void Computer::move_range(std::vector<T>& vec, int start, int end, int pos)
{
	vector<T> temp;
	temp.resize(end - start);
	std::move(vec.begin() + start, vec.begin() + end, temp.begin());
	vec.erase(vec.begin() + start, vec.begin() + end);
	auto iter = vec.begin();
	if (pos < 0)
		iter = vec.end();

	vec.insert(iter + pos, std::make_move_iterator(temp.begin()), std::make_move_iterator(temp.end()));
}

void Computer::start_playing_escape_sound()
{
	playing_escape_sound = true;
	time_since_escape_sound_started = 0;
}

void Computer::play_escape_sound()
{
	time_since_escape_sound_started++;
	if (time_since_escape_sound_started % 60 == 0)
	{
		alListener3f(AL_POSITION, 0, 0, 0);
		ALfloat orientation[] = { 0, 0, 1, 0, 1, 0 };
		alListenerfv(AL_ORIENTATION, orientation);
		alSource3f(sound_sources[0], AL_POSITION, 0, 0, 0.5);
		alSourcePlay(sound_sources[0]);
	}

	if (time_since_escape_sound_started >= 360)
	{
		std::exit(0);
	}
}

void Computer::play_sound(int index)
{

	if (is_playing(index))
		return;

	alListener3f(AL_POSITION, 0, 0, 0);
	ALfloat orientation[] = { 0, 0, 1, 0, 1, 0 };
	alListenerfv(AL_ORIENTATION, orientation);
	alSource3f(sound_sources[index], AL_POSITION, 0, 0, 0.5);
	//alSourcei(sound_sources[index], AL_GAIN, 1);
	alSourcePlay(sound_sources[index]);
}

void Computer::change_volume()
{
	alSourcei(sound_sources[0], AL_GAIN, 0);
}

void Computer::stop_sound(int index)
{
	alSourcei(sound_sources[index], AL_GAIN, 0);
	alSourceStop(sound_sources[index]);
}

bool Computer::is_playing(int index)
{
	ALenum state;
	alGetSourcei(sound_sources[index], AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

void Computer::remove_deleted()
{
	screen_elements.erase(remove_if(screen_elements.begin(), screen_elements.end(), [this](unique_ptr<ScreenElement>& elem)
	{
		if (elem->marked_for_deletion)
        {
            if (elem->has_focus)
                focus_element = screen_elements[0].get(); //TODO: is this always sane?
            
            return true;
        }
        
        return false;
	}), screen_elements.end());
}

void Computer::close_application(Application application)
{
	if (application != INFO_BOX)
		applications_open--;

	if (application == INFO_BOX && started_crashing_browser && !crashed_browser)
	{
		close_application(BROWSER);
		crashed_browser = true;
		update_in_progress = false;
	}

	if (application == VIDEO_APP)
	{
		extra_apps.erase(extra_apps.end() - 1);
		for (int i = 0; i < screen_elements.size(); ++i)
		{
			if (screen_elements[i]->name == "icon_video.png")
				screen_elements[i]->marked_for_deletion = true;
		}

		ran_trial = true;
		start_application(INFO_BOX, "The trial will now uninstall itself from your computer.");
	}

	for (int i = 0; i < screen_elements.size(); ++i)
	{
		ScreenElement* se = screen_elements[i].get();
		if (se->application == application)
			se->marked_for_deletion = true;
	}
}

void Computer::press_key(unsigned char key)
{
	//if (key == 'k')
	//	go_to_flying = true;
	
	if (focus_element != NULL)
		focus_element->press_key(key);

	process_control_changes();
}

void Computer::release_key(unsigned char key)
{
    if (focus_element != NULL)
        focus_element->release_key(key);
}

void Computer::set_computer_done(bool val)
{
	computer_done = val;
}

bool Computer::is_computer_done()
{
	return computer_done;
}