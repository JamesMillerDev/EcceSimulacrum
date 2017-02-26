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
	ColorPickerButton(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Canvas* _canvas) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), canvas(_canvas) {}
	void draw(TextureManager* texture_manager)
	{
		ScreenElement button_blank(x1, y1, x2, y2, "buttonblank.png");
		button_blank.draw(texture_manager);
		glDisable(GL_TEXTURE_2D);
		glColor4f((float)canvas->color_red / 255.0, (float)canvas->color_green / 255.0, (float)canvas->color_blue / 255.0, 1.0);
		glRectf(scalex(x1 + 3), scaley(y1 + 3), scalex(x2 - 3), scaley(y2 - 3));
		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		draw_string(texture_manager, 32, "Color", x1 + 1, y1 - 15);
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
				Vector3int hsl = canvas->rgb_to_hsl(canvas->color_red, canvas->color_green, canvas->color_blue);
				canvas->h_box->text = to_string(hsl.x);
				canvas->s_box->text = to_string(hsl.y);
				canvas->l_box->text = to_string(hsl.z);
				canvas->color_red = new_r;
				canvas->color_green = new_g;
				canvas->color_blue = new_b;
			}

			else
			{
				Vector3int rgb = canvas->hsl_to_rgb(atof(canvas->h_box->text.c_str()), atof(canvas->s_box->text.c_str()), atof(canvas->l_box->text.c_str()));
				canvas->r_box->text = to_string(rgb.x);
				canvas->g_box->text = to_string(rgb.y);
				canvas->b_box->text = to_string(rgb.z);
				canvas->color_red = rgb.x;
				canvas->color_green = rgb.y;
				canvas->color_blue = rgb.z;
			}
		};
	}

	void mouse_clicked(int button, int state, int x, int y)
	{
		if (dialog_open == true)
			return;

		auto window = make_unique<ScreenElement>(760.0, 400.0, 1160.0 + 250, 870.0, "beigeold.png", PAINT);
		auto left_edge = make_unique<ScreenElement>(750, 400, 760, 870, "silver.png", PAINT);
		auto right_edge = make_unique<ScreenElement>(1160 + 250, 400, 1160 + 250 + 10, 870, "silver.png", PAINT);
		auto bottom_edge = make_unique<ScreenElement>(750, 390, 1160 + 250 + 10, 400, "silver.png", PAINT);
		auto top_edge = make_unique<ScreenElement>(750, 870, 1160 + 250 + 10, 870 + 36, "silver.png", PAINT);
		auto select_color = make_unique<ScreenElement>(760, 875, 760 + 130, 872 + 17, "selectcolor.png", PAINT);
		auto satlumsquare = make_unique<SatLumSquare>(780.0, 490.0, 1080.0+60, 790.0+60, "white.png", PAINT, canvas);
		auto hue_slider = make_unique<HueSlider>(1080 + 60 + 10, 490, 1080 + 60 + 30, 790 + 60, "white.png", PAINT, canvas);
		auto close_button = make_unique<Button>(780.0, 420.0, 920.0, 470.0, "ok.png", PAINT, [this]()
		{
			for (int i = 0; i < this->canvas->child_components.size(); ++i)
				this->canvas->child_components[i]->marked_for_deletion = true;

			canvas->h_box->marked_for_deletion = true;
			canvas->l_box->marked_for_deletion = true;
			canvas->s_box->marked_for_deletion = true;
			canvas->r_box->marked_for_deletion = true;
			canvas->g_box->marked_for_deletion = true;
			canvas->b_box->marked_for_deletion = true;
			dialog_open = false;
			canvas->child_components.clear();
		}, [](){});
		canvas->child_components.push_back(window.get());
		canvas->child_components.push_back(close_button.get());
		canvas->child_components.push_back(satlumsquare.get());
		canvas->child_components.push_back(hue_slider.get());
		canvas->child_components.push_back(left_edge.get());
		canvas->child_components.push_back(right_edge.get());
		canvas->child_components.push_back(bottom_edge.get());
		canvas->child_components.push_back(top_edge.get());
		canvas->child_components.push_back(select_color.get());
		vector<TextField**> color_boxes{ &canvas->l_box, &canvas->s_box, &canvas->h_box, &canvas->b_box, &canvas->g_box, &canvas->r_box };
		vector<unique_ptr<TextField>> color_box_unique;
		for (int i = 0; i < color_boxes.size(); ++i)
		{
			/*float tx1 = (i < 3 ? 950.0 : 1065.0) + 17;
			float ty1;
			switch (i % 3)
			{
			case 0:
				ty1 = 490;
				break;

			case 1:
				ty1 = 180 - 30 + 490;
				break;

			case 2:
				ty1 = 360 - 60 + 490;
				break;
			}*/

			float tx1 = 1000;
			float ty1 = 454 + i * 2 * 36;
			auto text_box = make_unique<TextField>(tx1 + 250, ty1, tx1 + 75 + 250, ty1 + 36, "textfield.png", canvas->parent, application);
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

		canvas->h_box->after_typing_function = make_after_typing_function(false, 360, canvas->h_box);
		canvas->s_box->after_typing_function = make_after_typing_function(false, 100, canvas->s_box);
		canvas->l_box->after_typing_function = make_after_typing_function(false, 100, canvas->l_box);
		canvas->r_box->after_typing_function = make_after_typing_function(true, 255, canvas->r_box);
		canvas->g_box->after_typing_function = make_after_typing_function(true, 255, canvas->g_box);
		canvas->b_box->after_typing_function = make_after_typing_function(true, 255, canvas->b_box);
		canvas->r_box->text = to_string(canvas->color_red);
		canvas->g_box->text = to_string(canvas->color_green);
		canvas->b_box->text = to_string(canvas->color_blue);
		canvas->r_box->after_typing_function();
		canvas->parent->to_be_added.push_back(std::move(window));
		canvas->parent->to_be_added.push_back(std::move(close_button));
		canvas->parent->to_be_added.push_back(std::move(satlumsquare));
		canvas->parent->to_be_added.push_back(std::move(hue_slider));
		canvas->parent->to_be_added.push_back(std::move(bottom_edge));
		canvas->parent->to_be_added.push_back(std::move(top_edge));
		canvas->parent->to_be_added.push_back(std::move(left_edge));
		canvas->parent->to_be_added.push_back(std::move(right_edge));
		canvas->parent->to_be_added.push_back(std::move(select_color));
		for (int i = 0; i < color_box_unique.size(); ++i)
			canvas->parent->to_be_added.push_back(std::move(color_box_unique[i]));

		for (int i = 0; i < canvas->parent->to_be_added.size(); ++i)
		{
			canvas->parent->to_be_added[i]->no_scale = true;
			if (glutGet(GLUT_WINDOW_WIDTH) != 1920 || glutGet(GLUT_WINDOW_HEIGHT) != 1080)
			{
				/*float new_x = 750.0 * ((float)glutGet(GLUT_WINDOW_WIDTH) / 1920.0);
				float new_y = 390.0 * ((float)glutGet(GLUT_WINDOW_HEIGHT) / 1080.0);
				canvas->parent->to_be_added[i]->translate(new_x - canvas->parent->to_be_added[i]->x1, new_y - canvas->parent->to_be_added[i]->y1);*/
				canvas->parent->to_be_added[i]->translate(-100, -100);
			}
		}
		
		dialog_open = true;
	}
};

#endif