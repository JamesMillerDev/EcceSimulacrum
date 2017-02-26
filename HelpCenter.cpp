#include <algorithm>
#include "HelpCenter.h"
#include "fonts.h"

//TODO: RED ALERT
//WHAT IF HELPCENTER IS ADDED BY KEYBOARD?
//CONTROLS DON'T GET ADDED BECAUSE USING TO_BE_ADDED AND THAT'S ONLY USED IN MOUSE_CLICKED IN COMPUTER?
//TODO slow minimizing when lots of messages
HelpCenter::HelpCenter(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application, bool _mmo) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent), mmo(_mmo)
{
	assistant_messages.push_back("Hello!  May I have your name?");
	assistant_messages.push_back("May I have your address?");
	assistant_messages.push_back("Can I have a phone number?");
	assistant_messages.push_back("Date of birth?");
	assistant_messages.push_back("Thank you for your cooperation so far.  What is your problem today?");
	assistant_messages.push_back("I'm sorry, could you describe your problem in a little more detail?");
	assistant_messages.push_back("I'm sorry, I'm still having difficulty understanding the problem, do you think you could try again?");
	assistant_messages.push_back("Ok, I think I understand a little bit better now.  What have you tried so far in attempting to solve your problem?");
	assistant_messages.push_back("Do you feel happy with the progress you've made so far?");
	auto button = make_unique<Button>(850, 100, 950, 150, "ok.png", application,
		[this]()
	{
		if (this->text_field_ptr->text != "")
		{
			this->messages.push_back("You: " + this->text_field_ptr->text);
			this->text_field_ptr->reset();
			this->trigger_assistant_animation();
		}
	}, [](){});

	button_ptr = button.get();
	auto text_field = make_unique<TextField>(500, 100, 830, 150, "textfield.png", parent, application, [this](){ button_ptr->release_function(); });
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
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(text_field));
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(button));
	}
}

void HelpCenter::trigger_assistant_animation()
{
	if (mmo)
		return;

	assistant_animating = true;
	assistant_index++;
	if (assistant_index >= assistant_messages.size())
		assistant_index = 0;

	think_time = (rand() % 5) + 1;
	timebase = glutGet(GLUT_ELAPSED_TIME);
}

void HelpCenter::draw(TextureManager* texture_manager)
{
	ScreenElement::draw(texture_manager);
	glEnable(GL_STENCIL_TEST);
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
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	if (false)//(animating)
	{
		for (int i = 0; i < animated_messages.size(); ++i)
		{
			AnimatedMessage message = animated_messages[i];
			glColor4f(1.0, 1.0, 1.0, message.alpha);
			draw_string(texture_manager, 32, message.text, message.x, message.y);
			glColor4f(1.0, 1.0, 1.0, 1.0);
		}

		return;
	}
	//draw_string(texture_manager, 32, "Hello!", 1000.0, 1000.0);
	//fit_string(texture_manager, "Hello, world!  I'm going to type in a very long string here.  Hopefully, it will be long enough that it will cause some visible line breaks, and I can ensure that my function is working correctly!  Apoligies for the overly-long message, but I really do need to run this test to ensure that everything is in good working order!", 700.0, 900.0, 1000.0);
	current_y = y1 + 20; //900
	int total_lines = 0;
	for (int i = 0; i < messages.size(); ++i)
		total_lines += fit_string(texture_manager, messages[i], x1 + 10, x2 - 10, false);

	if (total_lines > 1)//20)
		current_y += (total_lines - 1) * 20;

	for (int i = 0; i < messages.size(); ++i)
		fit_string(texture_manager, messages[i], x1 + 10, x2 - 10);

	glStencilMask(~0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glDisable(GL_STENCIL_TEST);
	if (show_typing_message)
		draw_string(texture_manager, 32, "Assistant is typing...", x1 + 10, text_field_ptr->y2 + 5);
}

int HelpCenter::fit_string(TextureManager* texture_manager, string str, float start_x, float end_x, bool display)
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
			draw_string(texture_manager, FONT_SIZE, line, start_x, current_y);

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
	
	if (assistant_animating)
	{
		int cur_time = glutGet(GLUT_ELAPSED_TIME);
		if (cur_time - timebase > 1000 * think_time && !show_typing_message)
		{
			show_typing_message = true;
			timebase = glutGet(GLUT_ELAPSED_TIME);
		}

		if (show_typing_message)
		{
			cur_time = glutGet(GLUT_ELAPSED_TIME);
			if (cur_time - timebase > 2000)
			{
				messages.push_back("Assistant: " + assistant_messages[assistant_index]);
				show_typing_message = false;
				assistant_animating = false;
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