#include <algorithm>
#include "HelpCenter.h"
#include "ObjModel.h"
#include "fonts.h"

HelpCenter::HelpCenter(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application, bool _mmo) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent), mmo(_mmo)
{
	no_scale = true;
	word_failure = "Please select one of the words I gave you.  Type it exactly as it appears, and nothing else, so there's absolutely no ambiguity.";
	event_flag = "";
	invalidated = true;
	always_highlight = false;
	texture = 0;
	framebuffer = 0;
	rbo = 0;
	typing_time = 0;
	ifstream file_stream("helpfile");
	string line;
	while (getline(file_stream, line))
	{
		if (line == "" || line == " ")
			continue;

		auto components = split(line, '|');
		if (components.size() == 1)
			assistant_messages.push_back(AssistantMessage(pair<string, bool>(components[0], false), ""));

		else if (components.size() == 2)
			assistant_messages.push_back(AssistantMessage(pair<string, bool>(components[0], components[1] == "1" ? true : false), ""));

		else assistant_messages.push_back(AssistantMessage(pair<string, bool>(components[0], components[1] == "1" ? true : false), components[2]));
	}

	auto button = make_unique<Button>(850, 100, 950, 150, "send.png", application,
		[this]()
	{
		if (this->text_field_ptr->text != "" && !this->text_field_ptr->inactive)
		{
			this->messages.push_back("You: " + this->text_field_ptr->text);
			string word = this->text_field_ptr->text;
			this->invalidated = true;
			this->text_field_ptr->reset();
			if (this->time_to_trigger > 0)
				return;

			if (this->event_flag == "word")
			{
				auto components = split(assistant_messages[assistant_index].message.first, ' ');
				if (find(components.begin(), components.end(), word) == components.end())
				{
					this->event_flag = "word_failure";
					assistant_index -= 2;
				}

				else
				{
					this->event_flag = "";
					word_choices.push_back(word);
				}
			}

			this->trigger_assistant_animation();
		}
	}, [](){}, "", "invisible.png", "sendheld.png");

	button_ptr = button.get();
	auto text_field = make_unique<TextField>(500, 100, 830, 150, "helptextfield.png", parent, application, [this](){ button_ptr->release_function(); });
	text_field_ptr = text_field.get();
	if (!mmo)
	{
		parent->to_be_added.push_back(std::move(button));
		parent->to_be_added.push_back(std::move(text_field));
		text_field_ptr->is_visible = false;
		button_ptr->is_visible = false;
	}

	else
	{
		text_field->x1 = 0;
		text_field->x2 = 1800;
		text_field->y1 = 0;
		text_field->y2 = 100;
		button->x1 = 1800;
		button->x2 = 1920;
		button->y1 = 0;
		button->y2 = 100;
		//parent->screen_elements.insert(parent->screen_elements.end(), std::move(text_field));
		//parent->screen_elements.insert(parent->screen_elements.end(), std::move(button));
	}
}

void HelpCenter::trigger_assistant_animation()
{
	if (mmo || assistant_animating)
		return;

	assistant_animating = true;
	assistant_index++;
	if (assistant_index >= assistant_messages.size())
		assistant_index = 0;

	text_field_ptr->inactive_on();
	think_time = (rand() % 5) + 1;
	timebase = glutGet(GLUT_ELAPSED_TIME);
}

void HelpCenter::draw(TextureManager* texture_manager)
{
	if (invalidated)
	{
		GLint old_fbo;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &old_fbo);
		invalidated = false;
		if (texture != 0)
		{
			glDeleteTextures(1, &texture);
			glDeleteFramebuffers(1, &framebuffer);
			glDeleteRenderbuffers(1, &rbo);
		}

		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x2 - x1, y2 - y1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, x2 - x1, y2 - y1);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		glPushMatrix();
		glTranslatef(-x1, -y1, 0);
		ScreenElement::draw(texture_manager);
		/*glEnable(GL_STENCIL_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glStencilFunc(GL_NEVER, 1, 0xFF);
		glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
		glDisable(GL_TEXTURE_2D);
		glStencilMask(0xFF);
		glColor4f(1.0, 0.0, 0.0, 1.0);
		glRectf(x1, y1, x2, y2);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_TEXTURE_2D);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glStencilMask(0x00);
		glStencilFunc(GL_EQUAL, 1, 0xFF);*/
		ScreenElement black(x1, y1, x2, y2, "testtexture.png");
		black.no_scale = true;
		black.draw(texture_manager);
		current_y = y1 + 20; //900
		int total_lines = 0;
		int i;
		for (i = messages.size() - 1; i >= 0; --i)
		{
			total_lines += fit_string(texture_manager, messages[i], x1 + 10, x2 - 10, false);
			if (total_lines >= 20)
				break;
		}

		if (i == -1)
			i = 0;

		if (total_lines > 1)//20)
			current_y += (total_lines - 1) * 20;

		for (; i < messages.size(); ++i)
			fit_string(texture_manager, messages[i], x1 + 10, x2 - 10);
		
		/*glStencilMask(~0);
		glClear(GL_STENCIL_BUFFER_BIT);
		glDisable(GL_STENCIL_TEST);*/
		glPopMatrix();

		glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(scalex(x1), scaley(y1));
	glTexCoord2f(0.0, 1.0); glVertex2f(scalex(x1), scaley(y2));
	glTexCoord2f(1.0, 1.0); glVertex2f(scalex(x2), scaley(y2));
	glTexCoord2f(1.0, 0.0); glVertex2f(scalex(x2), scaley(y1));
	glEnd();
	
	if (show_typing_message)
		draw_string(texture_manager, 32, "Assistant is typing" + string((glutGet(GLUT_ELAPSED_TIME) / 400) % 3 + 1, '.'), x1 + 10, text_field_ptr->y2 + 5, true, always_highlight, 0);
}

int HelpCenter::fit_string(TextureManager* texture_manager, string str, int start_x, int end_x, bool display)
{
	//float current_y = start_y;
	int total_lines = 0;
	while (str.length() != 0)
	{
		vector<float> string_widths = draw_string(texture_manager, FONT_SIZE, str, start_x, current_y, false);
		int last_space = 0;
		int i;
		for (i = 0; i < str.length(); ++i)
		{
			if (str[i] == ' ')
				last_space = i;

			if (string_widths[i] >= end_x - start_x)
				break;
		}

		if (last_space == 0)
			last_space = i;

		if (i == str.length())
			last_space = i;

		string line = str.substr(0, last_space + 1);
		if (display)
			draw_string(texture_manager, FONT_SIZE, line, start_x, current_y, true, always_highlight, 0, true);

		str.erase(0, last_space + 1);
		if (display)
			current_y -= 20;

		else total_lines++;
	}

	return total_lines;
}

void HelpCenter::animate()
{
	if (mmo)
	{
		animating = false;
		return;
	}

	if (time_to_trigger > 0)
	{
		time_to_trigger--;
		if (text_field_ptr->key_pressed)
			assistant_messages[assistant_index + 1].message.first = "Aaah! Why did you keep typing?!";

		if (time_to_trigger == 0)
		{
			text_field_ptr->reset();
			trigger_assistant_animation();
			text_field_ptr->key_pressed = false;
		}
	}
	
	if (assistant_animating)
	{
		int cur_time = glutGet(GLUT_ELAPSED_TIME);
		if (cur_time - timebase > 1000 * think_time && !show_typing_message)
		{
			show_typing_message = true;
			text_field_ptr->on_hold = false;
			timebase = glutGet(GLUT_ELAPSED_TIME);
			typing_time = assistant_messages[assistant_index].message.first.size() / 7 * 1000;
			if (event_flag == "word_failure")
				typing_time = word_failure.size() / 7 * 1000;
		}

		if (show_typing_message)
		{
			cur_time = glutGet(GLUT_ELAPSED_TIME);
			if (cur_time - timebase > typing_time)
			{
				if (event_flag == "word_failure")
					messages.push_back("Assistant: " + word_failure);
				
				else
				{
					event_flag = assistant_messages[assistant_index].event_flag;
					if (event_flag == "sentence")
					{
						string sentence = "";
						for (int i = 0; i < word_choices.size(); ++i)
							sentence += word_choices[i] + " ";

						messages.push_back("Assistant: " + sentence);
						word_choices.clear();
					}

					else messages.push_back("Assistant: " + assistant_messages[assistant_index].message.first);
				}

				if (event_flag == "typing")
				{
					time_to_trigger = 8 * 60;
					event_flag = "";
				}

				invalidated = true;
				show_typing_message = false;
				assistant_animating = false;
				text_field_ptr->inactive_off();
				if (assistant_messages[assistant_index].message.second || event_flag == "word_failure")
				{
					trigger_assistant_animation();
					if (event_flag == "word_failure")
						event_flag = "";
				}

				if (assistant_index == 8)
				{
					parent->sound_manager->play_sound("ambient.wav");
					think_time = 11;
					text_field_ptr->on_hold = true;
				}
			}
		}

		return;
	}
	
	if (!animating)
		return;

	//if (animated_messages.empty())
	//	animated_messages.push_back(AnimatedMessage("Here is a message!", 300, 700, 0.01));

	for (int i = 0; i < animated_messages.size(); ++i)
	{
		AnimatedMessage cur_message = animated_messages[i];
		cur_message.x += 2;
		if (cur_message.alpha >= 1.0)
			cur_message.fading_out = true;

		if (cur_message.fading_out)
			cur_message.alpha -= 0.01;

		else
			cur_message.alpha += 0.01;

		animated_messages[i] = cur_message;
	}

	animated_messages.erase(remove_if(animated_messages.begin(), animated_messages.end(), [](AnimatedMessage elem)
	{
		return elem.alpha <= 0;
	}), animated_messages.end());

	if (animated_messages.empty())
	{
		animating = false;
		button_ptr->is_visible = true;
		text_field_ptr->is_visible = true;
		trigger_assistant_animation();
	}
}