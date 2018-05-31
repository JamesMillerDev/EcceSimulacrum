#ifndef RADIO_BUTTON
#define RADIO_BUTTON

#include "Computer.h"
#include "ScrollPanel.h"

struct RadioButton : public ScreenElement
{
	Computer* parent;
	ScrollPanel* scroll_panel;
	bool selected = false;
	bool cur_sheen = false;
	bool mouse_held = false;
	RadioButton* master;
	std::vector<RadioButton*> children;
	static int consecutive_selections;
	static int previous_click_time;
	RadioButton(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application, RadioButton* _master = NULL, ScrollPanel* _scroll_panel = NULL) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent), master(_master), scroll_panel(_scroll_panel)
	{
		if (master == NULL)
			master = this;
	}

	void revoke_selection()
	{
		for (int i = 0; i < children.size(); ++i)
			children[i]->selected = false;

		this->selected = false;
	}

	void mouse_clicked(int button, int state, int x, int y)
	{
		if (button != GLUT_LEFT)
			return;
		
		if (state == GLUT_DOWN)
			mouse_held = true;

		if (state == GLUT_UP && mouse_held)
		{
			if (!has_selection())
			{
				int cur_time = glutGet(GLUT_ELAPSED_TIME);
				if (previous_click_time == -1 || cur_time - previous_click_time < 2000) //TODO change to 2000 to reactivate
				{
					previous_click_time = cur_time;
					consecutive_selections++;
				}

				if (consecutive_selections >= 5)
				{
					consecutive_selections = 0;
					if (scroll_panel != NULL)
						scroll_panel->make_popup("You're going too fast!");
				}

				previous_click_time = cur_time;
			}

			master->revoke_selection();
			selected = true;
			mouse_held = false;
		}
	}

	bool has_selection()
	{
		if (master == this)
		{
			for (int i = 0; i < children.size(); ++i)
			{
				if (children[i]->selected)
					return true;
			}

			return selected;
		}

		return master->has_selection();
	}

	void draw(TextureManager* texture_manager)
	{
		ScreenElement::draw(texture_manager);
		if (cur_sheen)
		{
			ScreenElement sheen_elem(x1, y1, x2, y2, mouse_held ? "radiobuttonpushed.png" : "radiobuttonheld.png");
			sheen_elem.draw(texture_manager);
		}

		if (selected)
		{
			ScreenElement bubble = ScreenElement(x1 + 4, y1 + 4, x2 - 5, y2 - 5, "blackdot.png", application);
			bubble.draw(texture_manager);
		}
	}

	void take_focus()
	{
		ScreenElement::take_focus();
		mouse_held = false;
	}

	void mouse_over(int x, int y)
	{
		cur_sheen = true;
	}

	void mouse_off()
	{
		cur_sheen = false;
	}
};

#endif