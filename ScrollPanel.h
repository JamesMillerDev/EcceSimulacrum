#ifndef SCROLL_PANEL
#define SCROLL_PANEL

#include <vector>
#include <map>
#include "Computer.h"
#include "TextField.h"

struct ScrollBar;

struct Rect
{
	float x1, y1, x2, y2;
	Rect(float _x1, float _y1, float _x2, float _y2) : x1(_x1), y1(_y1), x2(_x2), y2(_y2) {}
	bool point_in_rect_strict(float x, float y)
	{
		int left_x = x1;
		int right_x = x2;
		int bottom_y = y1;
		int top_y = y2;
		if (right_x < left_x)
			std::swap(left_x, right_x);

		if (top_y < bottom_y)
			std::swap(bottom_y, top_y);

		return left_x < x && bottom_y < y && x < right_x && y < top_y;
	}
};

struct Link
{
	Rect rect;
	std::string target;
	Link(Rect r, std::string t) : rect(r), target(t) {}
};

struct ScrollPanel : public ScreenElement
{
	Computer* parent;
	std::vector<string> textures;
	std::vector<Link> links;
	std::map<std::string, std::vector<string>> websites;
	std::map<std::string, std::vector<string>> sequences;
	std::map<std::string, std::vector<std::pair<int, std::string>>> layout;
	std::map<std::string, std::string> alternative_links;
    std::vector<string> history;
	std::vector<ScreenElement*> children;
	std::vector<std::vector<string>> survey_questions1;
	std::vector<std::vector<string>> survey_questions2;
	TextField* text_field_ptr;
    int history_index;
	float offset;
    int timebase;
	int prev_mouse_x;
	int prev_mouse_y;
    int sum_of_heights;
    bool going_up;
    ScrollBar* scroll_bar;
	ScrollPanel(float _x1, float _y1, float _x2, float _y2, Computer* _parent, Application _application, bool no_extras = false);
	void change_website(std::string website_name, bool new_branch = true, bool force_through = true, string append_text = "");
	virtual void draw(TextureManager* texture_manager);
	void press_key(unsigned char key);
	void move_links(float delta);
	void mouse_over(int x, int y);
	void mouse_clicked(int button, int state, int x, int y);
	void mouse_off();
	void mouse_moved(int x, int y);
    void release_key(unsigned char key);
    void animate();
    void tick_scrollbar(int ticks);
    void go_back();
    void go_forward();
	void add_children(string website_name);
	void remove_children();
	void start_loading_site(string website_name, bool force_through = true);
	int create_radio_button_group(int start_x, int start_y, int number, bool horizontal, vector<string> labels);
	void create_survey_submit_button(int x, int y, function<void()> on_success, function<void()> on_failure);
	void turn_off_scroll_lock();
	void take_focus();
	void do_layout(string website_name, int start_x = 387, int start_y = 1080 - 288);
	bool whitelist_check(string website_name, bool force_through);
	bool currently_loading_site = false;
	bool scroll_locked = false;
	bool extend = false;
	int extension = 0;
	int scroll_lock_x, scroll_lock_y;
	string site_to_be_loaded;
	int time_started_loading;
	int loading_time;
	float scroll_speed = 40.0;
	string prev_website;
	string required_target;
	function<void()> animation_function;
	bool do_whitelist_check;
};

#endif