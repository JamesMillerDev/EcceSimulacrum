#ifndef PROGRESS_BAR
#define PROGRESS_BAR

#include <functional>
#include "Computer.h"
#include "fonts.h"

using std::function;

struct ProgressAction
{
	int type;
	double time, distance;
	ProgressAction(int _type, double _time, double _distance) : type(_type), time(_time), distance(_distance) {}
};

struct ProgressBar : public ScreenElement
{
	Computer* parent;
	float progress = 0;
	float white_alpha = 0.0;
	float increment = 0.01;
	vector<ProgressAction> actions;
	int action_index = 0;
	float cur_progress_increment;
	int wait_frames = 0;
	float frames_left;
	bool no_text = false;
	bool do_animate;
	function<void()> on_complete;
	ProgressBar(float _x1, float _y1, float _x2, float _y2, std::string _name, Computer* _parent, Application _application, function<void()> _on_complete, bool _no_text = false) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent), on_complete(_on_complete), no_text(_no_text)
	{
		actions.push_back(ProgressAction(1, 5, .5));
		actions.push_back(ProgressAction(0, 3, 0));
		actions.push_back(ProgressAction(1, 2, 1));
		frames_left = (5 + 3 + 2 + 1) * 60;
		get_progress_increment();
		progress = 0;
		do_animate = true;
	}

	void get_progress_increment()
	{
		cur_progress_increment = (actions[action_index].distance - progress) / actions[action_index].time / 60;
	}

	void draw(TextureManager* texture_manager)
	{
		x1 += 6;
		x2 -= 6;
		ScreenElement::draw(texture_manager);
		ScreenElement bar(x1, y1, (x2 - x1) * progress + x1, y2, "shinygreen.png");
		bar.draw(texture_manager);
		string cap1name, cap2name;
		if (progress == 0)
			cap1name = "cap" + name;

		else cap1name = "capshinygreen.png";
		if (progress == 1)
			cap2name = "capshinygreen.png";

		else cap2name = "cap" + name;
		ScreenElement cap1(x1 - 6, y1, x1, y2, cap1name);
		cap1.draw(texture_manager);
		ScreenElement cap2(x2 + 6, y1, x2, y2, cap2name);
		cap2.draw(texture_manager);
		ScreenElement white(x1 - 6, y1, (x2 - x1) * progress + x1, y2, "white.png");
		glColor4f(1.0, 1.0, 1.0, white_alpha);
		white.draw(texture_manager);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		if (!no_text)
		{
			draw_string(texture_manager, 32, "Now downloading...", x1 + 70, y2 + 90);
			draw_string(texture_manager, 32, "Time remaining: " + to_string((int)floor((float)frames_left / 60.0)) + " seconds", x1 + 70, y2 + 50);
		}

		x1 -= 6;
		x2 += 6;
	}

	void animate()
	{
		if (!do_animate)
			return;
		
		frames_left--;
		if (progress >= 1.0)
		{
			on_complete();
			parent->close_application(INFO_BOX);
			parent->process_control_changes();
		}

		else
		{
			//progress += 0.005;
			if (actions[action_index].type == 0)
			{
				wait_frames++;
				if (wait_frames >= actions[action_index].time * 60)
				{
					action_index++;
					if (action_index < actions.size())
					{
						get_progress_increment();
						wait_frames = 0;
					}
				}
			}

			else
			{
				if (progress >= actions[action_index].distance)
				{
					action_index++;
					if (action_index < actions.size())
					{
						get_progress_increment();
						wait_frames = 0;
					}
				}

				else
					progress += cur_progress_increment;
			}
		}

		white_alpha += increment;
		if (white_alpha >= 0.5 || white_alpha <= 0.0)
			increment *= -1.0;
	}
};

#endif
