#include "TextField.h"
#include "fonts.h"
#include "keymappings.h"

using namespace std;

//TODO highlighting for longer and longer amounts of time slows framerate
//TODO very long strings slow framerate
//TODO highlighting is triggered by up click not down click, can make it so if you down click and then type very fast, letters will go missing
//TODO password box doesn't scroll properly when text gets too big for box
//TODO double click to highlight all text
//TODO should maybe lose highlighting even earlier when loading a page?

void TextField::mouse_over(int x, int y)
{
	parent->someone_set_cursor = true;
	parent->set_cursor("cursori.png");
	change_names();
	mouse_on = true;
	if (name == "helptextfield.png")
		parent->set_cursor("cursoriwhite.png");
}

void TextField::change_names()
{
	if (name == "textfield.png")
	{
		old_name = "textfield.png";
		name = "textfieldhover.png";
		if (altcap != NULL)
			altcap->name = "altcaphover.png";
	}
}

void TextField::reset_names()
{
	if (old_name == "textfield.png" && name != "textfieldselected.png")
	{
		name = old_name;
		if (altcap != NULL)
			altcap->name = "altcap.png";
	}
}

void TextField::mouse_off()
{
	if (!mouse_on)
		return;
	
	mouse_on = false;
	reset_names();
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
	if (old_name == "textfield.png") //TODO impossible to hit this without doing mouse_over() first?
	{
		name = "textfieldselected.png";
		if (altcap != NULL)
			altcap->name = "altcapselected.png";
	}
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
	if (old_name == "textfield.png")
	{
		name = "textfield.png";
		if (altcap != NULL)
			altcap->name = "altcap.png";
	}

	if (altcap != NULL && parent->clicked_element != NULL && parent->clicked_element->name == "altcapbutton.png")
		change_names();
}

void TextField::go_to_end()
{
	text_offset = 0;
	cursor_pos = 0;
	cache_widths = true;
	get_string_widths(font_size, text, widths2);
	for (;;)
	{
		int old_cursor_pos = cursor_pos;
		required_override = true;
		press_key(KEY_RIGHT);
		required_override = false;
		if (cursor_pos == old_cursor_pos)
			break;
	}

	cache_widths = false;
}

bool TextField::text_exceeds_boundaries()
{
	vector<float> string_widths = draw_string(parent->texture_manager, font_size, text, (x1 + 10.0) - text_offset, y1 + 10.0, false, false, 0, no_scale);
	return string_widths.back() > (x2 - x1 - 20);
}

void TextField::animate()
{
	if (inactive)
		text = "Please wait your turn" + string((glutGet(GLUT_ELAPSED_TIME) / 400) % 3 + 1, '.');

	if (on_hold)
		text = "Thank you for your patience" + string((glutGet(GLUT_ELAPSED_TIME) / 400) % 3 + 1, '.');
	
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
	if (inactive)
		return;
	
	if (highlighting_text)
	{
		cur_mouse_x = x;
		mouse_clicked(GLUT_LEFT, GLUT_DOWN, x, y);
		click_count--;
	}
}

void TextField::mouse_clicked(int button, int state, int x, int y)
{
	if (button != GLUT_LEFT)
		return;
	
	if (inactive)
		return;
	
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
			highlight_start = highlight_end = -1;
			go_to_end();
			highlight_start = 0;
			highlight_end = text.length();
			//cursor_pos = text.length();
			if (required_text != "")
			{
				highlight_start = -1;
				highlight_end = -1;
			}
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
	get_string_widths(font_size, text, widths1);
	for (int i = 0; i < text.size() + 1; ++i)
	{
		if (string_pos <= widths1[i] - 3.0)
		{
			if (required_text == "")
				cursor_pos = i - 1;

			break;
		}
	}

	float final_width = widths1[text.size()];
	if (string_pos >= final_width)
		cursor_pos = text.size();

	int old_cursor_pos = cursor_pos;
	if (highlight_start == -1 && highlight_end == -1)
	{
		press_key(KEY_RIGHT);
		if (string_pos != widths1[text.size()] && cursor_pos != old_cursor_pos)
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
	
	else if (required_text == "")
	{
		highlighting_text = true;
		highlight_start = cursor_pos;
		highlight_end = cursor_pos;
		highlight_original = cursor_pos;
	}
}

float TextField::get_text_difference(string original_text)
{
	float temp1[4];
	float temp2[3];
	if (cursor_pos == 0)
	{
		get_string_widths(font_size, original_text.substr(0, 2), temp1);
		get_string_widths(font_size, text.substr(0, 1), temp2);
		return temp1[2] - temp2[1];
	}

	else if (cursor_pos == text.size())
	{
		get_string_widths(font_size, original_text.substr(original_text.size() - 2, 2), temp1);
		get_string_widths(font_size, text.substr(text.size() - 1, 1), temp2);
		return temp1[2] - temp2[1];
	}

	else
	{
		get_string_widths(font_size, original_text.substr(cursor_pos - 1, 3), temp1);
		get_string_widths(font_size, text.substr(cursor_pos - 1, 2), temp2);
		return temp1[3] - temp2[2];
	}
}

void TextField::press_key(unsigned char key)
{
	string old_text = text;
	if (inactive)
		return;

	if (has_focus)
	{
		cursor_visible = true;
		timebase = glutGet(GLUT_ELAPSED_TIME);
	}
	
	switch (key)
	{
	case 13: //TODO this is stupid
		on_enter_function();
		break;
	
	case KEY_LEFT:
		if (required_text != "" && !required_override)
			return;
		
		if (highlight_start != -1 && highlight_end != -1)
		{
			int target = highlight_start;
			highlight_start = -1;
			highlight_end = -1;
			if (cursor_pos == target)
				return;

			int dist = cursor_pos - target;
			get_string_widths(font_size, text, widths2);
			cache_widths = true;
			for (int i = 0; i < dist; ++i)
				press_key(KEY_LEFT);

			cache_widths = false;
			return;
		}
		
		if (cursor_pos > 0)
		{
			cursor_pos--;
			if (!cache_widths)
				get_string_widths(font_size, text, widths2);

			if (widths2[cursor_pos] - text_offset < 0.0)
				text_offset = widths2[cursor_pos];
		}

		break;

	case KEY_RIGHT:
		if (required_text != "" && !required_override)
			return;

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
			if (!cache_widths)
				get_string_widths(font_size, text, widths2);

			if (widths2[cursor_pos] > text_offset + (x2 - x1) - 20.0)
				text_offset += widths2[cursor_pos] - (text_offset + (x2 - x1) - 20.0);
		}

		break;

	case KEY_BACKSPACE:
	{
		if (required_text != "" && key != required_text[required_text_pos])
			goto DEF;

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
		
		//get_string_widths(font_size, text, widths1);
		//int original_widths_size = text.size() + 1;
		string original_text = text;
		text = text.substr(0, cursor_pos == 0 ? 0 : cursor_pos - 1) + text.substr(cursor_pos, text.length());
		if (password)
			backing_string = backing_string.substr(0, cursor_pos == 0 ? 0 : cursor_pos - 1) + backing_string.substr(cursor_pos, backing_string.length());

		required_override = true;
		press_key(KEY_LEFT);
		required_override = false;
		if (text_offset > 0)
		{
			//get_string_widths(font_size, text, widths2);
			text_offset -= get_text_difference(original_text); //widths1[original_widths_size - 1] - widths2[text.size()];
			if (text_offset < 0)
				text_offset = 0;
		}

		if (required_text != "")
			required_text_pos++;

		break;
	}

	case '\t':
	{
		if (tab_target == NULL)
			return;

		tab_target->old_name = "textfield.png";
		this->take_focus();
		parent->focus_element = tab_target;
		tab_target->mouse_clicked(GLUT_LEFT, GLUT_DOWN, tab_target->x1, tab_target->y1);
		tab_target->mouse_clicked(GLUT_LEFT, GLUT_UP, tab_target->x1, tab_target->y1);
		tab_target->give_focus();
		break;
	}

	DEF:
	default:
		unsigned char backing_key;
		if (!(32 <= key && key <= 126))
			return;

		if (password && 32 <= key && key <= 126)
		{
			backing_key = key;
			key = '*';
		}
		
		if (required_text != "")
		{
			if (required_text_pos >= required_text.size() || key != required_text[required_text_pos])
				return;
		}
		
		if (allowed_characters.find(key) == allowed_characters.end() && !allowed_characters.empty())
			return;

		if (highlight_start != -1 && highlight_end != -1)
			press_key(KEY_BACKSPACE);
		
		else if ((maximum_length != -1 && text.size() == maximum_length) || text.size() >= 630)
			return;

		if (32 <= key && key <= 126)
		{
			key_pressed = true;
			text = text.substr(0, cursor_pos) + string(1, key) + text.substr(cursor_pos, text.length());
			if (password)
				backing_string = backing_string.substr(0, cursor_pos) + string(1, backing_key) + backing_string.substr(cursor_pos, backing_string.length());
			
			required_override = true;
			press_key(KEY_RIGHT);
			required_override = false;
		}

		if (required_text != "")
			required_text_pos++;
	}

	max_value_check();
	if (text != old_text)
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
	max_value_check();
	after_typing_function();
}

void TextField::inactive_on()
{
	inactive = true;
	reset();
	text = "Please wait your turn.";
	key_pressed = false;
}

void TextField::inactive_off()
{
	inactive = false;
	on_hold = false;
	reset();
}

void TextField::max_value_check()
{
	if (maximum_value == 0)
		return;

	int value = atoi(text.c_str());
	if (value > maximum_value)
		text = to_string(maximum_value);
}

bool TextField::validate_email_address()
{
	int stage = 0;
	int char_count = 0;
	for (int i = 0; i < text.size(); ++i)
	{
		if (stage == 0)
		{
			if (text[i] == '@')
				stage++;
		}

		else if (stage == 1)
		{
			if (text[i] == '.')
				stage++;
		}

		else if (stage == 2)
		{
			if (text[i] == '.')
			{
				if (char_count != 2)
					return false;

				else
				{
					stage++;
					char_count = 0;
				}
			}

			else char_count++;
		}

		else if (stage == 3)
		{
			if (text[i] == '.')
			{
				if (char_count != 2)
					return false;

				else
				{
					stage++;
					char_count = 0;
				}
			}

			else char_count++;
		}

		else if (stage == 4)
		{
			if (i == text.size() - 1)
			{
				if (char_count == 2)
					return true;

				else return false;
			}

			else char_count++;
		}
	}

	return false;
}

bool TextField::validate_credit_card()
{
	if (text.size() != 16)
		return false;;

	int sum = 0;
	for (int i = 0; i < text.size(); ++i)
	{
		int val = atoi(text.substr(i, 1).c_str());
		if (i % 2 == 0)
		{
			val *= 2;
			if (val > 9)
				val = 1 + (val % 10);
		}

		sum += val;
	}

	return (sum % 10) == 0;
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
	/*if (password)
	{
		temp_text = text;
		//text = string('*', text.size()); TODO find out WTF is going on with this
		for (int i = 0; i < text.size(); ++i)
			text[i] = '*';
	}*/

	bool do_filler = false;
	if (!has_focus && text == "" && filler_text != "")
	{
		do_filler = true;
		text = filler_text;
	}

	int old_x1 = x1;
	int old_x2 = x2;
	if (name != "searchbar.png" && name != "helptextfield.png" && name != "textfieldblocky.png" && name != "systemtextfield.png" && name != "white.png")
	{
		x1 += 2;
		x2 -= 2;
	}

	ScreenElement::draw(texture_manager);
	x1 = old_x1;
	x2 = old_x2;
	ScreenElement cap1(x1, y1, x1 + 2, y2, "cap" + name);
	cap1.no_scale = no_scale;
	if (draw_first_cap && name != "searchbar.png" && name != "helptextfield.png" && name != "textfieldblocky.png" && name != "systemtextfield.png" && name != "white.png")
		cap1.draw(texture_manager);

	ScreenElement cap2(x2, y1, x2 - 2, y2, "cap" + name);
	cap2.no_scale = no_scale;
	if (name != "searchbar.png" && name != "helptextfield.png" && name != "textfieldblocky.png" && name != "systemtextfield.png" && name != "white.png")
		cap2.draw(texture_manager);

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
	if (do_filler)
		glColor4f(1.0, 1.0, 1.0, 0.5);

	draw_string(texture_manager, font_size, text, (x1 + 10.0) - text_offset, y1 + 10.0, true, always_highlight, 0, no_scale, false, false); //60
	get_string_widths(font_size, text, widths2);
	if (do_filler)
		glColor4f(1.0, 1.0, 1.0, 1.0);

	if (highlight_start != -1 && highlight_end != -1)
		draw_string(texture_manager, font_size, text.substr(highlight_start, highlight_end - highlight_start), (x1 + 10.0) + widths2[highlight_start] - text_offset, y1 + 10.0, true, true, y2 - y1 - 10, no_scale, false, false);
	glStencilMask(~0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glDisable(GL_STENCIL_TEST);
	if (cursor_visible && !inactive)
	{
		float cursor_x = widths2[cursor_pos] + x1 + 10.0 - text_offset;
		glDisable(GL_TEXTURE_2D);
		glColor4f(0.0, 0.0, 0.0, 1.0);
		if (always_highlight)
			glColor4f(1.0, 1.0, 1.0, 1.0);

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

	if (inactive)
	{
		glDisable(GL_TEXTURE_2D);
		glColor4f(0.0, 0.0, 0.0, 0.2);
		glBegin(GL_QUADS);
		glVertex2f(scalex(x1), scaley(y1));
		glVertex2f(scalex(x1), scaley(y2));
		glVertex2f(scalex(x2), scaley(y2));
		glVertex2f(scalex(x2), scaley(y1));
		glEnd();
		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0, 1.0, 1.0, 1.0);
	}

	/*if (password)
		text = temp_text;*/

	if (do_filler)
		text = "";
}