#ifndef HELP_CENTER
#define HELP_CENTER

#include "Computer.h"
#include "TextField.h"
#include "Button.h"

//TODO game runs at like 5 fps when you have a screen's worth of messages displaying (especially long ones that span multiple lines)
//TODO anything that uses stencil buffer does not minimize properly (probably need to create and attach stencil buffer to fbo/texture)

struct AnimatedMessage
{
	string text;
	int x;
	int y;
	float alpha;
	bool fading_out = false;
	AnimatedMessage(string _text, int _x, int _y, float _alpha) : text(_text), x(_x), y(_y), alpha(_alpha) {}
};

struct AssistantMessage
{
	pair<string, bool> message;
	string event_flag;
	AssistantMessage(pair<string, bool> _message, string _event_flag) : message(_message), event_flag(_event_flag) {}
};

struct HelpCenter : public ScreenElement
{
	Computer* parent;
	vector<string> messages;
	vector<AnimatedMessage> animated_messages;
	HelpCenter(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application, bool _mmo = false);
	void draw(TextureManager* texture_manager);
	int fit_string(TextureManager* texture_manager, string str, int start_x, int end_x, bool display = true);
	const int FONT_SIZE = 32;
	bool animating = true;
	bool mmo;
	void animate();
	int current_y;
	int typing_time;
	TextField* text_field_ptr;
	Button* button_ptr;
	bool assistant_animating = false;
	bool show_typing_message = false;
	int assistant_index = -1;
	vector<AssistantMessage> assistant_messages;
	void trigger_assistant_animation();
	int timebase;
	int think_time;
	bool invalidated;
	bool always_highlight;
	GLuint framebuffer;
	GLuint texture;
	GLuint rbo;
	string event_flag;
	string word_failure;
	vector<string> word_choices;
	int time_to_trigger = 0;
};

#endif