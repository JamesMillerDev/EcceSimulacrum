#ifndef RADIO_BUTTON
#define RADIO_BUTTON

#include "Computer.h"

struct RadioButton : public ScreenElement
{
	Computer* parent;
	bool selected = false;
	bool cur_sheen = false;
	bool mouse_held = false;
	RadioButton* master;
	std::vector<RadioButton*> children;
	static int consecutive_selections;
	static int previous_click_time;
	RadioButton(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application, RadioButton* _master = NULL) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent), master(_master) 
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
		if (state == GLUT_DOWN)
			mouse_held = true;

		if (state == GLUT_UP && mouse_held)
		{
			if (!has_selection())
			{
				int cur_time = glutGet(GLUT_ELAPSED_TIME);
				if (previous_click_time == -1 || cur_time - previous_click_time < 700) //TODO change to 2000 to reactivate
				{
					previous_click_time = cur_time;
					consecutive_selections++;
				}

				if (consecutive_selections >= 5)
				{
					consecutive_selections = 0;
					parent->start_application(INFO_BOX, "You're going too fast!");
				}
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
			ScreenElement bubble = ScreenElement(x1 + 8, y1 + 8, x2 - 8, y2 - 8, "blackdot.png", application);
			bubble.draw(texture_manager);
		}
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