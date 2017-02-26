#ifndef BUTTON
#define BUTTON

#include <functional>
#include "Computer.h"
#include "fonts.h"

using std::function;

struct Button : public ScreenElement
{
	bool greyed_out = false;
	bool dark = false;
	string grey_texture = "";
	bool flashing = false;
	bool is_mouse_over = false;
	int timebase = 0;
	string alternate_name = "searchbuttonflash.png";
	string sheen_name;
	string pressed_name;
	string tooltip;
	int cur_x, cur_y;
	bool cur_sheen = false, mouse_held = false;
	bool caps = false;
	Button(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, function<void()> _on_release, function<void()> _on_click, string _display_text = "", string _sheen_name = "", string _pressed_name = "", string _tooltip = "", bool _caps = false):
        ScreenElement(_x1, _y1, _x2, _y2, _name, _application), on_click_function(_on_click), release_function(_on_release), display_text(_display_text), sheen_name(_sheen_name), pressed_name(_pressed_name), tooltip(_tooltip), caps(_caps)
	{
		if (name == "ok.png")
		{
			sheen_name = "oklight.png";
			pressed_name = "okdark.png";
			x2 = x1 + 162;
			y2 = y1 + 43;
		}
	}
    function<void()> on_click_function;
    function<void()> release_function;
	string display_text;
	void draw(TextureManager* texture_manager)
	{
		string old_name;
		if (greyed_out)
		{
			old_name = name;
			name = grey_texture;
		}

		ScreenElement::draw(texture_manager);
		if (display_text != "")
			draw_string(texture_manager, 32, display_text, x1 + 5, y1 + ((y2 - y1) / 2));

		if (greyed_out)
			name = old_name;

		if (cur_sheen && !dark)
		{
			ScreenElement sheen_elem(x1, y1, x2, y2, mouse_held ? pressed_name : sheen_name);
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
				ScreenElement cap1(x1 - 0, y1, x1 - 11, y2, mouse_held ? "capdark.png" : "cap.png", application);
				ScreenElement cap2(x2 + 0, y1, x2 + 11, y2, mouse_held ? "capdark.png" : "cap.png", application);
				cap1.draw(texture_manager);
				cap2.draw(texture_manager);
			}
		}

		if (dark)
		{
			ScreenElement sheen_elem(x1, y1, x2, y2, pressed_name);
			sheen_elem.draw(texture_manager);
		}

		if (is_mouse_over && tooltip != "")
		{
			ScreenElement tip(cur_x, cur_y - 25, cur_x + 100, cur_y, tooltip);
			tip.draw(texture_manager);
		}
	}

    virtual void mouse_clicked(int button, int state, int x, int y)
    {
		if (greyed_out)
			return;
		
		if (state == GLUT_DOWN)
		{
			mouse_held = true;
			on_click_function();
		}
        
		if (state == GLUT_UP && mouse_held)
		{
			mouse_held = false;
			release_function();
		}
    }
    
    void take_focus()
    {
        ScreenElement::take_focus();
		mouse_held = false;
		//TODO: I have no idea why this was here in the first place
        //release_function(); //every release_function has to be sensitive to whether we have focus or not
    }

	void start_flashing()
	{
		flashing = true;
	}

	void animate()
	{
		if (flashing)
		{
			if (glutGet(GLUT_ELAPSED_TIME) - timebase > 200)
			{
				std::swap(name, alternate_name);
				timebase = glutGet(GLUT_ELAPSED_TIME);
			}
		}
	}

	void mouse_over(int x, int y)
	{
		if (sheen_name != "")
			cur_sheen = true;

		is_mouse_over = true;
		cur_x = x;
		cur_y = y;
	}

	void mouse_off()
	{
		cur_sheen = false;
		is_mouse_over = false;
	}
};

#endif
