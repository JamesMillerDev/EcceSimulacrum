#ifndef BUTTON
#define BUTTON

#include <functional>
#include "Computer.h"
#include "fonts.h"
#include "CycleNumber.h"
#include "ScrollPanel.h"

using std::function;

struct Button : public ScreenElement
{
	ScrollPanel* scroll_parent;
	bool dropdown_button;
	vector<Button*> dropdown_children;
	Button* dropdown_parent;
	bool greyed_out = false;
	bool dark = false;
	string grey_texture = "";
	string footer = "";
	bool flashing = false;
	bool is_mouse_over = false;
	bool draw_blue = false;
	bool handle_double_click = false;
	bool check_box = false;
	bool shiny = false;
	bool checked;
	bool animating;
	CycleNumber scale_factor;
	int timebase = 0;
	int time_last_clicked = 0;
	float sheen_pos;
	string alternate_name = "searchbuttonflash.png";
	string sheen_name;
	string pressed_name;
	string tooltip;
	string note;
	int cur_x, cur_y;
	bool cur_sheen = false, mouse_held = false;
	bool caps = false;
	bool replace_textures = false;
	Computer* parent;
	Button(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, function<void()> _on_release, function<void()> _on_click, string _display_text = "", string _sheen_name = "", string _pressed_name = "", string _tooltip = "", bool _caps = false, bool _replace_textures = false):
        ScreenElement(_x1, _y1, _x2, _y2, _name, _application), on_click_function(_on_click), release_function(_on_release), display_text(_display_text), sheen_name(_sheen_name), pressed_name(_pressed_name), tooltip(_tooltip), caps(_caps), draw_blue(false), time_last_clicked(0), handle_double_click(false), scale_factor(CycleNumber(0, 12, 2)), replace_textures(_replace_textures)
	{
		right_click_function = []() {};
		parent = NULL;
		dropdown_button = false;
		dropdown_parent = NULL;
		if (name == "ok.png")
		{
			sheen_name = "oklight.png";
			pressed_name = "okdark.png";
			//x2 = x1 + 162;
			//y2 = y1 + 43;
			x2 = x1 + 56;
			y2 = y1 + 30;
		}

		if (name == "browsebutton.png" || name == "uploadbutton.png")
		{
			x2 = x1 + 139;
			y2 = y1 + 26;
		}

		double_click_function = []() {};
		mouse_over_function = []() {};
		mouse_off_function = []() {};
		checked = true;
		animating = false;
		sheen_pos = y2 + 5;
	}

    function<void()> on_click_function;
	function<void()> right_click_function;
    function<void()> release_function;
	function<void()> double_click_function;
	function<void()> mouse_over_function;
	function<void()> mouse_off_function;
	string display_text;
	void draw(TextureManager* texture_manager)
	{
		if (animating)
		{
			x1 -= scale_factor.current_value;
			y1 -= scale_factor.current_value;
			x2 += scale_factor.current_value;
			y2 += scale_factor.current_value;
			glEnable(GL_STENCIL_TEST);
			glEnable(GL_ALPHA_TEST);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask(GL_TRUE);
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glAlphaFunc(GL_GREATER, 0.0);
			glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
			glStencilMask(0xFF);
		}

		string old_name;
		if (draw_blue)
			glColor4f(0.0, 0.4, 1.0, 1.0);

		if (greyed_out)
		{
			old_name = name;
			name = grey_texture;
		}

		if (!cur_sheen || !replace_textures)
			ScreenElement::draw(texture_manager);

		if (display_text != "")
			draw_string(texture_manager, 32, display_text, x1 + 5, y1 + (int)((y2 - y1) / 2));

		if (greyed_out)
			name = old_name;

		if (cur_sheen || dark)
		{
			ScreenElement sheen_elem(x1, y1, x2, y2, (mouse_held || dark) ? pressed_name : sheen_name);
			sheen_elem.no_scale = no_scale;
			if (sheen_elem.name == "gendark.png")
			{
				glColor4f(0.8, 0.8, 0.8, 1.0);
				ScreenElement::draw(texture_manager);
				glColor4f(1.0, 1.0, 1.0, 1.0);
			}

			else sheen_elem.draw(texture_manager);
			if (caps)
			{
				ScreenElement cap1(x1 - 0, y1, x1 - 11, y2, (mouse_held || dark) ? "capdark.png" : "cap.png", application);
				ScreenElement cap2(x2 + 0, y1, x2 + 11, y2, (mouse_held || dark) ? "capdark.png" : "cap.png", application);
				cap1.draw(texture_manager);
				cap2.draw(texture_manager);
			}
		}

		//if (dark)
		//{
		//	ScreenElement sheen_elem(x1, y1, x2, y2, pressed_name);
		//	sheen_elem.draw(texture_manager);
		//}

		if (is_mouse_over && tooltip != "")
		{
			ScreenElement tip(cur_x + 15, cur_y - texture_manager->get_height(tooltip) - 3, cur_x + 15 + texture_manager->get_width(tooltip), cur_y - 3, tooltip);
			tip.draw(texture_manager);
		}

		glColor4f(1.0, 1.0, 1.0, 1.0);
		if (footer != "")
		{
			string to_use = "";
			if (footer.length() > 10)
			{
				to_use = footer.substr(0, 10);
				to_use[7] = '.';
				to_use[8] = '.';
				to_use[9] = '.';
			}

			else to_use = footer;
			auto lengths = draw_string(texture_manager, 32, to_use, 0, 0, false);
			draw_string(texture_manager, 32, to_use, x1 - (int)(lengths.back() / 2.0) + (int)((x2 - x1) / 2.0), y1 - 14.0);
		}

		if (check_box && checked)
		{
			ScreenElement check(x1, y1, x2, y2, "check.png");
			check.draw(texture_manager);
		}

		if (animating)
		{
			x1 += scale_factor.current_value;
			y1 += scale_factor.current_value;
			x2 -= scale_factor.current_value;
			y2 -= scale_factor.current_value;
			glDepthMask(GL_TRUE);
			glStencilMask(0x00);
			glStencilFunc(GL_EQUAL, 1, 0xFF);
			ScreenElement shimmer(x1 - 100, sheen_pos, x1 + 500, sheen_pos + 200, "buttonsheen.png");
			shimmer.draw(texture_manager);
			glStencilMask(~0);
			glClear(GL_STENCIL_BUFFER_BIT);
			glDisable(GL_STENCIL_TEST);
			glDisable(GL_ALPHA_TEST);
		}
	}

    virtual void mouse_clicked(int button, int state, int x, int y)
    {
		if (button != GLUT_LEFT && state != GLUT_DOWN)
			return;
		
		if (greyed_out)
			return;

		if (button == GLUT_RIGHT_BUTTON)
		{
			right_click_function();
			return;
		}
		
		if (state == GLUT_DOWN)
		{
			mouse_held = true;
			if (time_last_clicked != 0 && glutGet(GLUT_ELAPSED_TIME) - time_last_clicked <= 750 && handle_double_click)
			{
				double_click_function();
				draw_blue = false;
				time_last_clicked = 0;
			}
			
			else
			{
				if (shiny)
					parent->sound_manager->play_sound("thunk.wav");

				on_click_function();
			}

			time_last_clicked = glutGet(GLUT_ELAPSED_TIME);
		}
        
		if (state == GLUT_UP && mouse_held)
		{
			mouse_held = false;
			if (shiny)
				parent->sound_manager->play_sound("thunk.wav");

			if (name != "bookmarkholder.png")
			{
				if (dropdown_parent != NULL)
					dropdown_parent->remove_dropdown();

				else remove_dropdown();
			}

			checked = !checked;
			release_function();
		}
    }
    
    void take_focus()
    {
        ScreenElement::take_focus();
		mouse_held = false;
		if (parent != NULL && parent->focus_element->application != INFO_BOX)
			draw_blue = false;
		//TODO: I have no idea why this was here in the first place
        //release_function(); //every release_function has to be sensitive to whether we have focus or not
    }

	void start_flashing()
	{
		flashing = true;
	}

	void animate()
	{
		ScreenElement::animate();
		if (flashing)
		{
			if (glutGet(GLUT_ELAPSED_TIME) - timebase > 200)
			{
				std::swap(name, alternate_name);
				timebase = glutGet(GLUT_ELAPSED_TIME);
			}
		}

		if (animating)
		{
			scale_factor.step();
			sheen_pos -= 35;
			if (scale_factor.current_value <= 0)
				animating = false;
		}

		test_for_remove_dropdown();
	}

	void mouse_over(int x, int y)
	{
		if (sheen_name != "")
			cur_sheen = true;

		if (shiny && !is_mouse_over)
		{
			scale_factor.restart();
			sheen_pos = y2 + 5;
			animating = true;
			parent->sound_manager->play_sound("beep.wav");
		}

		is_mouse_over = true;
		cur_x = x;
		cur_y = y;
		mouse_over_function();
		if (dropdown_button)
		{
			if (dropdown_children.empty())
			{
				auto holder = make_unique<Button>(1738, 1080 - 227, -1, -1, "bookmarkholder.png", application, []() {}, []() {});
				holder->dropdown_parent = this;
				dropdown_children.push_back(holder.get());
				parent->to_be_added.push_back(std::move(holder));

				auto bm1 = make_unique<Button>(1762, 1080 - 95, -1, -1, "searchbookmark.png", application, [this]() {this->scroll_parent->start_loading_site("search", false); if (!scroll_parent->whitelist_check("search", false)) { scroll_parent->error_display = scroll_parent->website_names["search"]; }}, []() {}, "", "searchbookmarkdark.png", "searchbookmarkdark.png", "", false, true);
				bm1->dropdown_parent = this;
				dropdown_children.push_back(bm1.get());
				parent->to_be_added.push_back(std::move(bm1));

				auto bm2 = make_unique<Button>(1762, 1080 - 95 - 28, -1, -1, "ainbookmark.png", application, [this]() {this->scroll_parent->start_loading_site("ain", false); if (!scroll_parent->whitelist_check("ain", false)) { scroll_parent->error_display = scroll_parent->website_names["ain"]; }}, []() {}, "", "ainbookmarkdark.png", "ainbookmarkdark.png", "", false, true);
				bm2->dropdown_parent = this;
				dropdown_children.push_back(bm2.get());
				parent->to_be_added.push_back(std::move(bm2));

				auto bm3 = make_unique<Button>(1762, 1080 - 95 - (28 * 2), -1, -1, "socnewsbookmark.png", application, [this]() {this->scroll_parent->start_loading_site("reddit", false); 
				if (!scroll_parent->whitelist_check("reddit", false)) 
				{ 
					scroll_parent->error_display = scroll_parent->website_names["reddit"]; 
				}}, []() {}, "", "socnewsbookmarkdark.png", "socnewsbookmarkdark.png", "", false, true);
				bm3->dropdown_parent = this;
				dropdown_children.push_back(bm3.get());
				parent->to_be_added.push_back(std::move(bm3));

				auto bm4 = make_unique<Button>(1762, 1080 - 95 - (28 * 3), -1, -1, "nendabookmark.png", application, [this]() {this->scroll_parent->start_loading_site("nenda", false); if (!scroll_parent->whitelist_check("nenda", false)) { scroll_parent->error_display = scroll_parent->website_names["nenda"]; }}, []() {}, "", "nendabookmarkdark.png", "nendabookmarkdark.png", "", false, true);
				bm4->dropdown_parent = this;
				dropdown_children.push_back(bm4.get());
				parent->to_be_added.push_back(std::move(bm4));

				auto bm5 = make_unique<Button>(1762, 1080 - 95 - (28 * 4), -1, -1, "usefulbookmark.png", application, [this]() {this->scroll_parent->start_loading_site("useful", false); if (!scroll_parent->whitelist_check("useful", false)) { scroll_parent->error_display = scroll_parent->website_names["useful"]; }}, []() {}, "", "usefulbookmarkdark.png", "usefulbookmarkdark.png", "", false, true);
				bm5->dropdown_parent = this;
				dropdown_children.push_back(bm5.get());
				parent->to_be_added.push_back(std::move(bm5));
			}
		}
	}

	void mouse_off()
	{
		mouse_off_function();
		cur_sheen = false;
		is_mouse_over = false;
	}

	void test_for_remove_dropdown()
	{
		if (is_mouse_over)
			return;

		for (int i = 0; i < dropdown_children.size(); ++i)
		{
			if (dropdown_children[i]->is_mouse_over)
				return;
		}

		remove_dropdown();
	}

	void remove_dropdown()
	{
		for (int i = 0; i < dropdown_children.size(); ++i)
			dropdown_children[i]->marked_for_deletion = true;

		dropdown_children.clear();
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
};

#endif
