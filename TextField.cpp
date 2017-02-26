#include "TextField.h"
#include "fonts.h"
#include "keymappings.h"

using namespace std;

//TODO highlighting for longer and longer amounts of time slows framerate
//TODO very long strings slow framerate
//TODO highlighting is triggered by up click not down click, can make it so if you down click and then type very fast, letters will go missing
//TODO password box doesn't scroll properly when text gets too big for box

void TextField::mouse_over(int x, int y)
{
	parent->someone_set_cursor = true;
	glutSetCursor(GLUT_CURSOR_TEXT);
}

void TextField::give_focus()
{
	ScreenElement::give_focus();
	cursor_visible = true;
	/*if (text.length() != 0)
	{
		highlight_start = 0;
		highlight_end = text.length();
		cursor_pos = text.length();
	}*/

	timebase = glutGet(GLUT_ELAPSED_TIME);
}

void TextField::take_focus()
{
	ScreenElement::take_focus();
	click_count = 0;
	cursor_visible = false;
	highlighting_text = false;
	highlight_start = -1;
	highlight_end = -1;
	take_focus_function();
}

void TextField::animate()
{
	if (has_focus)
	{
		int current_time = glutGet(GLUT_ELAPSED_TIME);
		if (current_time - timebase >= TEXT_FIELD_MS_TO_FRAME_CHANGE)
		{
			cursor_visible = !cursor_visible;
			timebase = current_time;
		}
	}

	if (highlighting_text)
	{
		if (frames % 1 == 0) //lol
		{
			int old_pos = cursor_pos;
			if (cur_mouse_x > x2)
			{
				int temp_start = highlight_start;
				int temp_end = highlight_end;
				highlight_start = highlight_end = -1;
				press_key(KEY_RIGHT);
				highlight_start = temp_start;
				highlight_end = temp_end;
				if (cursor_pos != old_pos)
					highlight_end = cursor_pos;
			}

			if (cur_mouse_x < x1)
			{
				int temp_start = highlight_start;
				int temp_end = highlight_end;
				highlight_start = highlight_end = -1;
				press_key(KEY_LEFT);
				highlight_start = temp_start;
				highlight_end = temp_end;
				if (cursor_pos != old_pos)
					highlight_start = cursor_pos;
			}
		}

		frames++;
	}
}

void TextField::mouse_moved(int x, int y)
{
	if (highlighting_text)
	{
		cur_mouse_x = x;
		mouse_clicked(GLUT_LEFT, GLUT_DOWN, x, y);
	}
}

void TextField::mouse_clicked(int button, int state, int x, int y)
{
	click_count++;
	cur_mouse_x = x;
	if ((x > x2 || x < x1) && state != GLUT_UP)
		return; //Because this will be called elsewhere
	
	on_click_function();
	if (button == GLUT_LEFT && state == GLUT_UP)
	{
		highlighting_text = false;
		if (highlight_start == highlight_end && click_count == 2 && text.length() != 0)
		{
			highlight_start = 0;
			highlight_end = text.length();
			cursor_pos = text.length();
		}

		else if (highlight_start == highlight_end)
		{
			highlight_start = -1;
			highlight_end = -1;
		}

		return;
	}
	
	if (x1 <= x && x < x1 + 10.0)
		x = x1 + 10.0;

	if (x2 - 10.0 < x && x <= x2)
		x = x2 - 10.0;

	int string_pos = text_offset + (x - x1) - 10.0;
	vector<float> string_widths = draw_string(NULL, font_size, text, (x1 + 10.0) - text_offset, y1 + 10.0, false, false, 0, no_scale);
	for (int i = 0; i < string_widths.size(); ++i)
	{
		if (string_pos <= string_widths[i] - 3.0)
		{
			cursor_pos = i - 1;
			break;
		}
	}

	if (string_pos > string_widths[string_widths.size() - 1])
		cursor_pos = string_widths.size() - 1;

	int old_cursor_pos = cursor_pos;
	if (highlight_start == -1 && highlight_end == -1)
	{
		press_key(KEY_RIGHT);
		if (string_pos != string_widths[string_widths.size() - 1] && cursor_pos != old_cursor_pos)
			press_key(KEY_LEFT);
	}

	if (highlighting_text)
	{
		if (cursor_pos > highlight_original)
		{
			highlight_start = highlight_original;
			highlight_end = cursor_pos;
		}

		if (cursor_pos < highlight_original)
		{
			highlight_start = cursor_pos;
			highlight_end = highlight_original;
		}

		if (cursor_pos == highlight_original)
			highlight_start = highlight_end = highlight_original;
	}
	
	else
	{
		highlighting_text = true;
		highlight_start = cursor_pos;
		highlight_end = cursor_pos;
		highlight_original = cursor_pos;
	}
}

void TextField::press_key(unsigned char key)
{
	switch (key)
	{
	case 13: //TODO this is stupid
		on_enter_function();
		break;
	
	case KEY_LEFT:
		if (highlight_start != -1 && highlight_end != -1)
		{
			int target = highlight_start;
			highlight_start = -1;
			highlight_end = -1;
			if (cursor_pos == target)
				return;

			for (int i = 0; i < cursor_pos - target; ++i)
				press_key(KEY_LEFT);

			return;
		}
		
		if (cursor_pos > 0)
		{
			cursor_pos--;
			vector<float> string_widths = draw_string(NULL, font_size, text, (x1 + 10.0) - text_offset, y1 + 10.0, false, false, 0, no_scale);
			if (string_widths[cursor_pos] - text_offset < 0.0)
				text_offset = string_widths[cursor_pos];
		}

		break;

	case KEY_RIGHT:
		if (highlight_start != -1 && highlight_end != -1)
		{
			int target = highlight_end;
			highlight_start = -1;
			highlight_end = -1;
			if (cursor_pos == target)
				return;

			for (int i = 0; i < cursor_pos - target; ++i)
				press_key(KEY_RIGHT);

			return;
		}

		if (cursor_pos < text.length())
		{
			cursor_pos++;
			vector<float> string_widths = draw_string(NULL, font_size, text, (x1 + 10.0) - text_offset, y1 + 10.0, false, false, 0, no_scale); //size 60 has to change to be in sync with all sizes
			if (string_widths[cursor_pos] > text_offset + (x2 - x1) - 20.0)
				text_offset += string_widths[cursor_pos] - (text_offset + (x2 - x1) - 20.0);
		}

		break;

	case KEY_BACKSPACE:
	{
		if (highlight_start != -1 && highlight_end != -1)
		{
			int times = highlight_end - highlight_start;
			cursor_pos = highlight_end;
			highlight_start = -1;
			highlight_end = -1;
			for (int i = 0; i < times; ++i)
				press_key(KEY_BACKSPACE);

			return;
		}
		
		vector<float> original_widths = draw_string(NULL, font_size, text, (x1 + 10.0) - text_offset, y1 + 10.0, false, false, 0, no_scale);
		text = text.substr(0, cursor_pos == 0 ? 0 : cursor_pos - 1) + text.substr(cursor_pos, text.length());
		press_key(KEY_LEFT);
		if (text_offset > 0)
		{
			vector<float> new_widths = draw_string(NULL, font_size, text, (x1 + 10.0) - text_offset, y1 + 10.0, false, false, 0, no_scale);
			text_offset -= original_widths[original_widths.size() - 1] - new_widths[new_widths.size() - 1];
		}

		break;
	}

	default:
		if (required_text != "")
		{
			if (required_text_pos >= required_text.size() || key != required_text[required_text_pos])
				return;
		}
		
		if (allowed_characters.find(key) == allowed_characters.end() && !allowed_characters.empty())
			return;

		if (highlight_start != -1 && highlight_end != -1)
			press_key(KEY_BACKSPACE);
		
		else if (maximum_length != -1 && text.size() == maximum_length)
			return;

		if (32 <= key && key <= 126)
		{
			text = text.substr(0, cursor_pos) + string(1, key) + text.substr(cursor_pos, text.length());
			press_key(KEY_RIGHT);
		}

		if (required_text != "")
			required_text_pos++;
	}

	after_typing_function();
}

void TextField::reset()
{
	text = "";
	cursor_pos = 0;
	text_offset = 0;
	highlight_start = highlight_end = -1;
}

void TextField::type_into(string new_text)
{
	text = new_text;
	after_typing_function();
}

void TextField::go_to_end()
{
	cursor_pos = text.size();
}

void TextField::draw(TextureManager* texture_manager)
{
	if (label_text != "")
	{
		InfoText info_text(x1 - 70, y1, x1 - 40, y2, "invisible.png", label_text, NONE);
		info_text.no_scale = true;
		info_text.draw(texture_manager);
	}

	//text = "Hello, world!";
	string temp_text;
	if (password)
	{
		temp_text = text;
		//text = string('*', text.size()); TODO find out WTF is going on with this
		for (int i = 0; i < text.size(); ++i)
			text[i] = '*';
	}

	ScreenElement::draw(texture_manager);
	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glStencilFunc(GL_NEVER, 1, 0xFF);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
	glDisable(GL_TEXTURE_2D);
	glStencilMask(0xFF);
	glColor4f(1.0, 0.0, 0.0, 1.0);
	if (!no_scale)
		glRectf(scalex(x1 + 10.0), scaley(y1), scalex(x2 - 10.0), scaley(y2));

	else
		glRectf(x1 + 10.0, y1, x2 - 10.0, y2);

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0x00);
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	vector<float> string_widths = draw_string(texture_manager, font_size, text, (x1 + 10.0) - text_offset, y1 + 10.0, true, false, 0, no_scale); //60
	if (highlight_start != -1 && highlight_end != -1)
		draw_string(texture_manager, font_size, text.substr(highlight_start, highlight_end - highlight_start), (x1 + 10.0) + string_widths[highlight_start] - text_offset, y1 + 10.0, true, true, y2 - y1 - 10, no_scale);
	glStencilMask(~0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glDisable(GL_STENCIL_TEST);
	if (cursor_visible)
	{
		float cursor_x = string_widths[cursor_pos] + x1 + 10.0 - text_offset;
		glDisable(GL_TEXTURE_2D);
		glColor4f(0.0, 0.0, 0.0, 1.0);
		if (!no_scale)
		{
			glBegin(GL_QUADS);
			glVertex2f(scalex(cursor_x), scaley(y2 - 5.0));
			glVertex2f(scalex(cursor_x), scaley(y1 + 5.0));
			glVertex2f(scalex(cursor_x + 1.0), scaley(y1 + 5.0));
			glVertex2f(scalex(cursor_x + 1.0), scaley(y2 - 5.0));
			glEnd();
		}

		else
		{
			glBegin(GL_QUADS);
			glVertex2f(cursor_x, y2 - 5.0);
			glVertex2f(cursor_x, y1 + 5.0);
			glVertex2f(cursor_x + 1.0, y1 + 5.0);
			glVertex2f(cursor_x + 1.0, y2 - 5.0);
			glEnd();
		}

		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0, 1.0, 1.0, 1.0);
	}

	if (password)
		text = temp_text;
}