#ifndef SCROLL_PANEL
#define SCROLL_PANEL

#include <vector>
#include <map>
#include "Computer.h"
#include "TextField.h"
#include "PhysicsPiece.h"
#include "CycleNumber.h"

struct ScrollBar;

struct ShoppingCenter;

struct ShoppingItem
{
	string image;
	string name;
	int price;
	float ownership;
	vector<string> properties;
	ShoppingItem(string i, string n, int p, vector<string> t) : image(i), name(n), price(p), properties(t), ownership(0.0) {}
};

struct Link
{
	Rect rect;
	std::string target;
	bool underline;
	int offset;
	bool no_cursor;
	float underline_red, underline_green, underline_blue;
	Link(Rect r, std::string t, bool u = false, int o = 0) : rect(r), target(t), underline(u), offset(o), no_cursor(false)
	{
		link_action = []() {};
		underline_red = underline_green = underline_blue = 0;
	}

	function<void()> link_action;
};

struct BrowserKey
{
	int key, frames, death_counter, velocity;
	BrowserKey(int _key, int _death_counter) : key(_key), frames(0), death_counter(_death_counter), velocity(0) {}
	BrowserKey() : key(-1), frames(0), death_counter(0), velocity(0) {}
};

struct ScrollPanel : public ScreenElement
{
	Computer* parent;
	int useful_frame = 0;
	int underlinex1, underlinex2, underliney;
	std::vector<string> textures;
	std::vector<Link> links;
	std::map<std::string, std::vector<std::pair<int, std::string>>> layout;
	std::map<std::string, std::string> alternative_links;
    std::vector<std::pair<string, string>> history;
	std::vector<ScreenElement*> children;
	std::vector<std::vector<string>> survey_questions1;
	std::vector<std::vector<string>> survey_questions2;
	std::vector<std::vector<string>> survey_questions3;
	std::map<std::string, std::string> website_names;
	std::map<std::string, std::string> special_error_display;
	vector<pair<int, int>> break_points;
	vector<PhysicsPiece> glass_textures;
	TextField* text_field_ptr;
	string error_display;
    int history_index;
	float offset;
	int prev_mouse_x;
	bool will_force_through;
	int prev_mouse_y;
    float sum_of_heights;
    ScrollBar* scroll_bar;
	ScrollPanel(float _x1, float _y1, float _x2, float _y2, Computer* _parent, Application _application, bool no_extras = false);
	void change_website(std::string website_name, bool new_branch = true, bool force_through = false, bool skip_loading = false);
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
	void go_home();
	void never_focus_children();
	void add_link_button(vector<string> components, string website_name);
	void add_children(string website_name, bool skip_loading = false);
	void remove_children();
	void start_loading_site(string website_name, bool force_through = true);
	void update_cursor(int x, int y);
	int create_radio_button_group(int start_x, int start_y, int number, bool horizontal, vector<string> labels, int change = 30);
	void create_survey_submit_button(int x, int y, function<void()> on_success, function<void()> on_failure, string name1 = "browserok.png", string name2 = "browserheld.png", string name3 = "browserpressed.png");
	void turn_off_scroll_lock();
	void take_focus();
	void do_layout(string website_name, int start_x = 387, int start_y = 1080 - 288);
	bool upload_image_file(string file_name, string folder_name);
	void resize_page(float delta);
	void restart_game();
	vector<vector<pair<int, int>>> get_pieces(int width, int height, int xoffset, int yoffset, string glass_map_name, int big_glass_width, int big_glass_height);
	int find_piece(vector<vector<pair<int, int>>> pieces, pair<int, int> point);
	void write_buffer_overlapping(GLubyte* buffer, int buffer_width, int buffer_height, GLubyte* image, int image_x, int image_y, int image_width, int image_height);
	void create_glass_textures(vector<vector<pair<int, int>>> pieces, int actual_image_width, int actual_image_height, int xoffset, int yoffset, string image_name, string glass_name, int big_glass_width, int big_glass_height, string suffix);
	bool whitelist_check(string website_name, bool force_through);
	bool currently_loading_site = false;
	bool scroll_locked = false;
	bool extend = false;
	bool is_mouse_over = false;
	bool no_scroll = false;
	bool retain_white_cover_alpha = false;
	int extension = 0;
	int scroll_lock_x, scroll_lock_y;
	string site_to_be_loaded;
	string trying_to_load;
	int time_started_loading;
	int loading_time;
	string prev_website;
	function<void()> animation_function;
	bool do_whitelist_check;
	float big_glass_width = 2090.0;
	float big_glass_height = 1200.0;
	int num_break_points;
	bool drawn_all_glass = false;
	int fall_pieces = 0;
	void generate_puzzle();
	ShoppingCenter* shopping_center;
	string replace_whitespace(string str);
	CycleNumber white_cover_alpha;
	function<void()> white_cover_apex;
	bool responds_to(int button);
	int site_change_frame = 0;
	GLfloat underline_red, underline_green, underline_blue;
	map<string, string> link_names;
	int myframes = 0;
	void ask_for_delete(ScreenElement* elem);
	BrowserKey browser_key;
	void scroll_page(int speed);
	int scroll_lock_speed = 0;
	TextField* selection_field_ptr;
	void make_popup(string text);
	int results_counter = 0;
};

#endif