#ifndef TEXT_FIELD
#define TEXT_FIELD

#include <functional>
#include <set>
#include "Computer.h"
#include "InfoText.h"

const int FONT_POINTS = 32;

struct TextField : public ScreenElement
{
	Computer* parent;
	TextField(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application, function<void()> _enter_function = []() {}, function<void()> _on_click_function = [](){}) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent), mouse_on(false), cursor_visible(false),
        timebase(0), text_offset(0.0), cursor_pos(0), on_enter_function(_enter_function), on_click_function(_on_click_function), label_text("")
	{
		surrender_focus_on_up_click = false;
		receive_out_of_bounds_clicks = true;
		text = "";
		required_text = "";
		font_size = 32;
	}
	void mouse_over(int x, int y);
	void mouse_clicked(int button, int state, int x, int y);
	void mouse_moved(int x, int y);
	void animate();
	void give_focus();
	void take_focus();
	void press_key(unsigned char key);
	void reset();
	void type_into(string new_text);
	void go_to_end();
	virtual void draw(TextureManager* texture_manager);
	bool mouse_on;
	string text;
	string required_text;
	string label_text;
	int required_text_pos = 0;
	int click_count = 0;
	int font_size = 32;
	int cursor_pos;
	bool cursor_visible;
	bool highlighting_text = false;
	bool password = false;
	int highlight_start = -1, highlight_end = -1;
	int highlight_original;
	int cur_mouse_x = 0;
	int timebase;
	float text_offset;
	function<void()> on_click_function;
	function<void()> on_enter_function;
	function<void()> after_typing_function = [](){};
	function<void()> take_focus_function = [](){};
	std::set<unsigned char> allowed_characters;
	int maximum_length = -1;
	int frames = 0;
};

#endif