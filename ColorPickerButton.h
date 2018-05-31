#ifndef COLOR_PICKER_BUTTON
#define COLOR_PICKER_BUTTON

#include "Computer.h"
#include "Canvas.h"
#include "SatLumSquare.h"
#include "HueSlider.h"
#include "InfoText.h"

struct ColorPickerButton : public ScreenElement
{
	Canvas* canvas;
	bool dialog_open = false;
	bool canvas_box;
	bool history_box;
	GLubyte self_color_red, self_color_green, self_color_blue, old_red, old_green, old_blue;
	GLubyte* color_red;
	GLubyte* color_green;
	GLubyte* color_blue;
	string label_text;
	ColorPickerButton(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Canvas* _canvas, bool _canvas_box = true, bool _history_box = false) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), 
		canvas(_canvas), canvas_box(_canvas_box), history_box(_history_box)
	{
		label_text = "";
		self_color_red = 255;
		self_color_green = 255;
		self_color_blue = 255;
		if (canvas_box)
		{
			color_red = &(canvas->color_red);
			color_green = &(canvas->color_green);
			color_blue = &(canvas->color_blue);
		}

		else
		{
			color_red = &self_color_red;
			color_green = &self_color_green;
			color_blue = &self_color_blue;
		}
	}

	bool responds_to(int button)
	{
		if (button == GLUT_LEFT)
			return true;

		else if (button == GLUT_RIGHT_BUTTON)
			return true;

		else if (button == 3)
			return false;

		else if (button == 4)
			return false;

		else if (button == 1)
			return false;
	}

	void draw(TextureManager* texture_manager)
	{
		ScreenElement button_blank(x1, y1, x2, y2, name);
		button_blank.draw(texture_manager);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_MULTISAMPLE);
		if (name == "buttonblank2.png")
		{
			/*glColor4f(0.0, 1.0, 0.0, 1.0);
			glBegin(GL_LINE_STRIP);
			glVertex2f(x1 + 2, y2 - 2);
			glVertex2f(x2 - 2, y2 - 2);
			glVertex2f(x2 - 2, y1 + 2);
			glVertex2f(x1 + 2, y1 + 2);
			glVertex2f(x1 + 2, y2 - 2);
			glEnd();*/
			glColor4f(1.0, 1.0, 1.0, 1.0);
			glRectf(scalex(x1 + 2), scaley(y1 + 2), scalex(x2 - 2), scaley(y2 - 2));
		}

		glColor4f((float)*color_red / 255.0, (float)*color_green / 255.0, (float)*color_blue / 255.0, 1.0);
		glRectf(scalex(x1 + 3), scaley(y1 + 3), scalex(x2 - 3), scaley(y2 - 3));
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		if (canvas_box)
			draw_string(texture_manager, 32, "Color", x1 + 1, y1 - 15);

		if (label_text != "")
			draw_string(texture_manager, 32, label_text, x2 + 5, y1 + 3);
	}

	void mouse_over(int x, int y)
	{
		name = "buttonblank2.png";
	}

	void mouse_off()
	{
		name = "buttonblank.png";
	}

	function<void()> make_after_typing_function(bool is_rgb_box, int max_value, TextField* box_ptr)
	{
		return [=]()
		{
			int value = atoi(box_ptr->text.c_str());
			if (value > max_value)
				box_ptr->text = to_string(max_value);

			if (is_rgb_box)
			{
				int new_r = atoi(canvas->r_box->text.c_str());
				int new_g = atoi(canvas->g_box->text.c_str());
				int new_b = atoi(canvas->b_box->text.c_str());
				Vector3int hsl = canvas->rgb_to_hsl(*color_red, *color_green, *color_blue);
				canvas->h_box->text = to_string(hsl.x);
				canvas->s_box->text = to_string(hsl.y);
				canvas->l_box->text = to_string(hsl.z);
				*color_red = new_r;
				*color_green = new_g;
				*color_blue = new_b;
			}

			else
			{
				Vector3int rgb = canvas->hsl_to_rgb(atof(canvas->h_box->text.c_str()), atof(canvas->s_box->text.c_str()), atof(canvas->l_box->text.c_str()));
				canvas->r_box->text = to_string(rgb.x);
				canvas->g_box->text = to_string(rgb.y);
				canvas->b_box->text = to_string(rgb.z);
				*color_red = rgb.x;
				*color_green = rgb.y;
				*color_blue = rgb.z;
			}
		};
	}

	void mouse_clicked(int button, int state, int x, int y)
	{
		if (dialog_open == true) //TODO should be static or disallow clicking while open or something...
			return;

		if (state != GLUT_DOWN)
			return;

		if (history_box)
		{
			std::swap(self_color_red, canvas->color_red);
			std::swap(self_color_green, canvas->color_green);
			std::swap(self_color_blue, canvas->color_blue);
			return;
		}

		if (button == GLUT_RIGHT_BUTTON && !canvas_box)
		{
			canvas->push_color();
			canvas->color_red = *color_red;
			canvas->color_green = *color_green;
			canvas->color_blue = *color_blue;
			return;
		}

		//if (canvas_box)
		//	canvas->push_color();
		old_red = *color_red;
		old_green = *color_green;
		old_blue = *color_blue;

		canvas->parent->reset_elements();
		canvas->parent->focus_application = INFO_BOX;
		canvas->parent->restore_application = PAINT;
		auto window = make_unique<ScreenElement>(760.0, 540, 1160.0 + 250, 870.0, "white.png", INFO_BOX);
		int center_x = glutGet(GLUT_WINDOW_WIDTH) / 2.0;
		int center_y = glutGet(GLUT_WINDOW_HEIGHT) / 2.0;
		int delta_x = center_x - ((window->x2 - window->x1) / 2 + window->x1);
		int delta_y = center_y - ((window->y2 - window->y1) / 2 + window->y1);
		window->translate(delta_x, delta_y);
		auto trans = pair<int, int>(delta_x, delta_y);
		auto select_color = make_unique<ScreenElement>(760, 875, 760 + 125, 872 + 25, "selectcolor.png", INFO_BOX);
		auto cover = make_unique<ScreenElement>(780 - 1, 814 - 256 - 1 + 36, 780 + 256 + 1, 814 + 1 + 36, "satlum.png", INFO_BOX);
		auto satlumsquare = make_unique<SatLumSquare>(780.0, 814 - 256 + 36, 780 + 256/*1080.0+60*/, 814 + 36/*790.0+60*/, "red.png", INFO_BOX, canvas);
		auto hue_slider = make_unique<HueSlider>(1036 + 15, 814 - 256 + 36, 1036 + 15 + 20, 814 + 36, "red.png", INFO_BOX, canvas);
		auto close_button = make_unique<Button>(780.0, 420.0 + 130, 920.0, 470.0 + 130, "ok.png", INFO_BOX, [this]()
		{
			if (this->canvas_box)
				this->canvas->push_color(true, this->old_red, this->old_green, this->old_blue);

			this->canvas->parent->close_application(INFO_BOX);
		}, [](){});
		vector<TextField**> color_boxes{ &canvas->l_box, &canvas->s_box, &canvas->h_box, &canvas->b_box, &canvas->g_box, &canvas->r_box };
		vector<unique_ptr<TextField>> color_box_unique;
		for (int i = 0; i < color_boxes.size(); ++i)
		{
			//float tx1 = (i < 3 ? 950.0 : 1065.0) + 17;
			float ty1 = 0;
			switch (i % 3)
			{
			case 0:
				ty1 = 594;
				break;

			case 1:
				ty1 = 704;
				break;

			case 2:
				ty1 = 816;
				break;
			}

			float tx1 = 1000;
			//float ty1 = 454 + (i < 3 ? i + 3 : i) * 2 * 36;
			if (i >= 3)
				tx1 -= 90;

			else
				tx1 += 70;

			auto text_box = make_unique<TextField>(tx1 + 250, ty1, tx1 + 75 + 250, ty1 + 36, "textfield.png", canvas->parent, INFO_BOX);
			set<unsigned char> allowed{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
			text_box->allowed_characters = allowed;
			text_box->maximum_length = 3;
			TextField* text_box_ptr = text_box.get();
			text_box->take_focus_function = [text_box_ptr]()
			{
				if (text_box_ptr->text == "")
				{
					text_box_ptr->text = "0";
					text_box_ptr->after_typing_function();
				}
			};

			*(color_boxes[i]) = text_box_ptr;
			string label_text;
			if (i == 0)
				label_text = "Lum:";

			if (i == 1)
				label_text = "Sat:";

			if (i == 2)
				label_text = "Hue:";

			if (i == 3)
				label_text = "Blue:";

			if (i == 4)
				label_text = "Green:";

			if (i == 5)
				label_text = "Red:";

			text_box->label_text = label_text;
			color_box_unique.push_back(std::move(text_box));
		}

		canvas->h_box->after_typing_function = make_after_typing_function(false, 359, canvas->h_box);
		canvas->s_box->after_typing_function = make_after_typing_function(false, 100, canvas->s_box);
		canvas->l_box->after_typing_function = make_after_typing_function(false, 100, canvas->l_box);
		canvas->r_box->after_typing_function = make_after_typing_function(true, 255, canvas->r_box);
		canvas->g_box->after_typing_function = make_after_typing_function(true, 255, canvas->g_box);
		canvas->b_box->after_typing_function = make_after_typing_function(true, 255, canvas->b_box);
		canvas->r_box->text = to_string(*color_red);
		canvas->g_box->text = to_string(*color_green);
		canvas->b_box->text = to_string(*color_blue);
		canvas->r_box->after_typing_function();
		canvas->r_box->tab_target = canvas->g_box;
		canvas->g_box->tab_target = canvas->b_box;
		canvas->b_box->tab_target = canvas->h_box;
		canvas->h_box->tab_target = canvas->s_box;
		canvas->s_box->tab_target = canvas->l_box;
		canvas->l_box->tab_target = canvas->r_box;
		canvas->parent->to_be_added.push_back(std::move(window));
		canvas->parent->borders_title_x(760.0, 540, 1160.0 + 250, 870.0, INFO_BOX, true, [this]()
		{
			*color_red = old_red;
			*color_green = old_green;
			*color_blue = old_blue;
		});
		canvas->parent->to_be_added.push_back(std::move(close_button));
		canvas->parent->to_be_added.push_back(std::move(cover));
		canvas->parent->to_be_added.push_back(std::move(satlumsquare));
		canvas->parent->to_be_added.push_back(std::move(hue_slider));
		canvas->parent->to_be_added.push_back(std::move(select_color));
		for (int i = 0; i < color_box_unique.size(); ++i)
			canvas->parent->to_be_added.push_back(std::move(color_box_unique[i]));

		for (int i = 0; i < canvas->parent->to_be_added.size(); ++i)
		{
			canvas->parent->to_be_added[i]->no_scale = true;
			//if (glutGet(GLUT_WINDOW_WIDTH) != 1920 || glutGet(GLUT_WINDOW_HEIGHT) != 1080)
			//{
				/*float new_x = 750.0 * ((float)glutGet(GLUT_WINDOW_WIDTH) / 1920.0);
				float new_y = 390.0 * ((float)glutGet(GLUT_WINDOW_HEIGHT) / 1080.0);
				canvas->parent->to_be_added[i]->translate(new_x - canvas->parent->to_be_added[i]->x1, new_y - canvas->parent->to_be_added[i]->y1);*/
				//canvas->parent->to_be_added[i]->translate(-100, -100);
				if (canvas->parent->to_be_added[i]->name != "white.png")
					canvas->parent->to_be_added[i]->translate(trans.first, trans.second);
			//}
		}

		canvas->parent->tag_info_box();
	}
};

#endif