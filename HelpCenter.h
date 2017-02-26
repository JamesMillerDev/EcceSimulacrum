#ifndef HELP_CENTER
#define HELP_CENTER

#include "Computer.h"
#include "TextField.h"
#include "Button.h"

struct AnimatedMessage
{
	string text;
	int x;
	int y;
	float alpha;
	bool fading_out = false;
	AnimatedMessage(string _text, int _x, int _y, float _alpha) : text(_text), x(_x), y(_y), alpha(_alpha) {}
};

struct HelpCenter : public ScreenElement
{
	Computer* parent;
	vector<string> messages;
	vector<AnimatedMessage> animated_messages;
	HelpCenter(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application, bool _mmo = false);
	void draw(TextureManager* texture_manager);
	int fit_string(TextureManager* texture_manager, string str, float start_x, float end_x, bool display = true);
	const int FONT_SIZE = 32;
	bool animating = true;
	bool mmo;
	void animate();
	float current_y;
	TextField* text_field_ptr;
	Button* button_ptr;
	bool assistant_animating = false;
	bool show_typing_message = false;
	int assistant_index = -1;
	vector<string> assistant_messages;
	void trigger_assistant_animation();
	int timebase;
	int think_time;
};

#endif