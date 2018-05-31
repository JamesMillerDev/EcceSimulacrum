#ifndef COMPUTER
#define COMPUTER

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "TextureManager.h"
#include "SoundManager.h"

using std::string;
using std::vector;
using std::unique_ptr;
using std::pair;

enum ComputerState {STATE_STARTUP, STATE_DESKTOP, STATE_TITLE_SCREEN};

enum Application {NONE, BROWSER, FILES, CONSOLE, INFO_BOX, PAINT, VIDEO_APP, DECRYPTION_APP, MMO_GAME, LEVEL_EDITOR_APP};

float scalex(float x);
float scaley(float y);
float unscalex(float x);
float unscaley(float y);

struct ScrollPanel;

struct Rect
{
	float x1, y1, x2, y2;
	int xtrans, ytrans;
	Rect(float _x1, float _y1, float _x2, float _y2) : x1(_x1), y1(_y1), x2(_x2), y2(_y2) 
	{
		xtrans = ytrans = 0;
	}

	Rect() : x1(0), y1(0), x2(0), y2(0)
	{
		xtrans = ytrans = 0;
	}

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

struct Message
{
	string text;
	int delay_ms;
	Message(string _text, int _delay_ms): text(_text), delay_ms(_delay_ms) {}
};

struct File
{
	string name;
	GLubyte* image = 0;
	int xsize = 0;
	int ysize = 0;
	int type = 0;
};

struct ScreenElement
{
	float x1, y1, x2, y2;
	float alpha;
	string name;
	bool has_focus;
	bool is_visible;
	bool marked_for_deletion;
	bool transient;
	bool browser_child = false;
	bool partial = false;
	Application application;
	bool cache_texture = false;
	bool no_scale = false;
	bool never_focus;
	bool receive_out_of_bounds_clicks = false;
	bool sliding = false;
	float sliding_goal_x;
	bool surrender_focus_on_up_click = false;
	bool invert;
	bool rotated = false;
	bool fake = false;
	bool border_element = false;
	bool no_translate = false;
	bool need_resize;
	bool drawn_by_browser = false;
	bool partial_x = false;
	bool always_inform_mouse_moved = false;
	float angle = 0;
	int texture_id = 0;
	int info_box_id;
	ScreenElement(float _x1, float _y1, float _x2, float _y2, string _name, Application _application = NONE, bool _cache_texture = false, bool _border_element = false) : x1(_x1), y1(_y1), x2(_x2), y2(_y2), name(_name), has_focus(false), is_visible(true), application(_application), marked_for_deletion(false), transient(false), cache_texture(_cache_texture), never_focus(false), receive_out_of_bounds_clicks(false), surrender_focus_on_up_click(true), partial(false), invert(false), border_element(_border_element), no_translate(false),
		drawn_by_browser(false)
	{
		alpha = 1.0;
		if (x2 == -1 && y2 == -1)
			need_resize = true;

		else need_resize = false;
	}

	virtual void mouse_over(int x, int y) {}
	virtual void mouse_off() {}
	virtual void mouse_clicked(int button, int state, int x, int y) {}
    virtual void mouse_moved(int x, int y) {}
	virtual void animate() 
	{
		if (sliding)
		{
			if (x1 == sliding_goal_x)
			{
				sliding = false;
				return;
			}

			int sign = copysign(1, sliding_goal_x - x1);
			translate(sign * 5, 0);
			int new_sign = copysign(1, sliding_goal_x - x1);
			if (sign != new_sign)
			{
				translate(sliding_goal_x - x1, 0);
				sliding = false;
			}
		}
	}

	virtual void draw(TextureManager* texture_manager)
	{
		if (alpha != 1.0)
			glColor4f(1.0, 1.0, 1.0, alpha);
		
		if (need_resize)
		{
			x2 = x1 + texture_manager->get_width(name);
			y2 = y1 + texture_manager->get_height(name);
			need_resize = false;
		}
		
		bool changed_name = false;
		if (name[0] == '|')
		{
			name = name.substr(1);
			changed_name = true;
		}

		glPushMatrix();
		glTranslatef(x1, y1, 0);
		glRotatef(angle, 0.0, 0.0, 1.0);
		if (cache_texture && texture_id == 0)
			texture_id = texture_manager->textures[texture_manager->index_map[name]];

		float width = glutGet(GLUT_WINDOW_WIDTH);
		float height = glutGet(GLUT_WINDOW_HEIGHT);
		float fy, sy;
		if (!invert)
		{
			fy = 1.0;
			sy = 0.0;
		}

		else
		{
			fy = 0.0;
			sy = 1.0;
		}

		if ((width == 1920.0 && height == 1080.0) || no_scale)
		{
			if (cache_texture)
				texture_manager->change_texture(texture_id);

			else texture_manager->change_texture(name);
			float extent = partial ? (float)(y2 - y1) / (float)texture_manager->get_height(name) : fy;
			float x_extent = partial_x ? (float)(x2 - x1) / (float)texture_manager->get_width(name) : 1.0;
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, extent); glVertex2f(x1 - x1, y1 - y1);
			glTexCoord2f(0.0, sy); glVertex2f(x1 - x1, y2 - y1);
			glTexCoord2f(x_extent, sy); glVertex2f(x2 - x1, y2 - y1);
			glTexCoord2f(x_extent, extent); glVertex2f(x2 - x1, y1 - y1);
			glEnd();
		}

		else
		{
			if (cache_texture)
				texture_manager->change_texture(texture_id);
			
			else texture_manager->change_texture(name);
			float extent = partial ? (float)(y2 - y1) / (float)texture_manager->get_height(name) : fy;
			float x_extent = partial_x ? (float)(x2 - x1) / (float)texture_manager->get_width(name) : 1.0;
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, extent); glVertex2f(x1 * (width / 1920.0) - x1, y1 * (height / 1080.0) - y1);
			glTexCoord2f(0.0, sy); glVertex2f(x1 * (width / 1920.0) - x1, y2 * (height / 1080.0) - y1);
			glTexCoord2f(x_extent, sy); glVertex2f(x2 * (width / 1920.0) - x1, y2 * (height / 1080.0) - y1);
			glTexCoord2f(x_extent, extent); glVertex2f(x2 * (width / 1920.0) - x1, y1 * (height / 1080.0) - y1);
			glEnd();
		}

		glPopMatrix();
		if (changed_name)
			name = "|" + name;

		if (alpha != 1.0)
			glColor4f(1.0, 1.0, 1.0, 1.0);
	}

	virtual void translate(float delta_x, float delta_y)
	{
		x1 += delta_x;
		x2 += delta_x;
		y1 += delta_y;
		y2 += delta_y;
	}

	void smooth_translate(float delta_x, float delta_y)
	{
		sliding = true;
		sliding_goal_x = x1 + delta_x;
	}

	pair<float, float> center()
	{
		int center_x = 1920.0 / 2.0;
		int center_y = 1080.0 / 2.0;
		int delta_x = center_x - ((x2 - x1) / 2 + x1);
		int delta_y = center_y - ((y2 - y1) / 2 + y1);
		translate(delta_x, delta_y);
		return pair<int, int>(delta_x, delta_y);
	}

	void center_in(ScreenElement* s)
	{
		int mid_sx = s->x1 + (s->x2 - s->x1) / 2;
		int mid_sy = s->y1 + (s->y2 - s->y1) / 2;
		translate(mid_sx - x1, mid_sy - y1);
		translate((x2 - x1) / -2, 0);
		translate(0, (y2 - y1) / -2);
	}

	bool check_rotation(int x, int y)
	{
		glPushMatrix();
		glTranslatef(x1, y1, 0.0);
		glRotatef(-angle, 0.0, 0.0, 1.0);
		GLfloat in_vec[] = { x - x1, y - y1, 0, 1,
							0, 0, 0, 0,
							0, 0, 0, 0,
							0, 0, 0, 0 };
		GLfloat out_vec[16];
		glMultMatrixf(in_vec);
		glGetFloatv(GL_MODELVIEW_MATRIX, out_vec);
		int rx = out_vec[0];
		int ry = out_vec[1];
		return rx >= x1 && rx <= (x1 + 300) && ry >= y1 && ry <= y2;
	}

	virtual void give_focus() { has_focus = true; }
	virtual void take_focus() { has_focus = false; }
	virtual void press_key(unsigned char key) {}
	virtual void release_key(unsigned char key) {}
	bool operator==(const ScreenElement& rhs) const
	{
		return x1 == rhs.x1 && y1 == rhs.y1 && x2 == rhs.x2 && y2 == rhs.y2 && name == rhs.name;
	}

	virtual bool responds_to(int button)
	{
		if (button == GLUT_LEFT)
			return true;

		else if (button == GLUT_RIGHT_BUTTON)
			return false;

		else if (button == 3)
			return false;

		else if (button == 4)
			return false;

		else if (button == 1)
			return false;
	}

	virtual ~ScreenElement() {}
};

class Computer
{
private:
	void draw_desktop();
	void animate_messages();
	void animate_desktop();
	void go_to_desktop();
	void go_to_title_screen();
	void go_to_continue_screen();
	void remove_deleted();
	void warp_cursor(int x, int y);
	bool computer_done;
	bool going_to_sleep;
	int width;
	int height;
	int grid_width;
	int grid_height;
	int current_wait;
	int prev_time;
	float alpha_increment;
	GLfloat alpha;
	vector<Message> messages;
	Application minimizing_application;
	GLfloat minimizing_alpha;
	bool alpha_going_up;
	bool play_minimizing_animation;
	GLuint minimizing_framebuffer;
	GLuint minimizing_texture;
	GLuint minimizing_rbo;
	string cursor;
	int min_x1 = -1, min_y1 = -1, min_x2 = -1, min_y2 = -1;
	int target_minimize_x;
	float cur_minimize_x1, cur_minimize_y1;
	float first_m, first_b, second_m, second_b, movable_m, movable_b, end_m, end_b;
	int forcing_to_x, forcing_to_y;
	bool going_up;
	int info_box_id = 0;
	int minimize_mode = 0;
	void get_extent(Application application, vector<unique_ptr<ScreenElement>>& target);
	void setup_minimizing_buffer();
	void start_open_animation(Application application);
	void start_close_animation(Application application);

public:
	void delist(string website);
	string required_target;
	std::map<std::string, std::vector<string>> websites;
	ScreenElement* cursor_maintainer;
	std::map<std::string, std::vector<string>> sequences;
	map<string, map<int, string>> link_overrides;
	bool no_mouse;
	ComputerState computer_state;
	SoundManager* sound_manager;
	int prev_mouse_x, prev_mouse_y;
	int sticking_x, sticking_y;
	TextureManager* texture_manager;
	vector<vector<char> > char_grid;
	ScreenElement* focus_element;
	ScreenElement* clicked_element;
	Application focus_application;
	int applications_open;
	bool someone_set_cursor;
	bool maintain_cursor = false;
	bool playing_escape_sound = false;
	bool forcing_cursor;
	bool sticking_cursor = false;
	bool looking_for_stick = false;
	bool forcing_e2 = false;
	bool forcing_mmo_site = false;
	bool go_to_flying = false;
	bool greyed_out = false;
	bool draw_arrow_thing = false;
	bool decrypt_forcing = false;
	bool everything_stuck = false;
	bool done_cracking = false;
	int queue_click_x, queue_click_y;
	int scroll_lock_x, scroll_lock_y;
	ScreenElement* exempt_from_forcing = NULL; //TODO what was this for?
	int time_since_escape_sound_started;
    void window_resized(int new_width, int new_height);
	void mouse_clicked(int button, int state, int x, int y);
	void mouse_moved(int x, int y, ScreenElement* do_not_inform = 0, bool no_scale = false);
	void go_to_sleep();
	void setup_view();
	void animate();
	void draw();
	void press_key(unsigned char key);
	void release_key(unsigned char key);
	void set_computer_done(bool val);
	bool is_computer_done();
	void reset_computer();
	void process_control_changes();
	void start_application(Application application, string info_text = "", bool file_dialog = false, bool progress_dialog = false, int download_action = 0, ScrollPanel* scroll_panel = NULL, bool no_move = false);
	void close_application(Application application);
	void draw_messages(float xtrans = 0.0, float ytrans = 0.0, float scale = 1.0);
	void start_playing_escape_sound(string escape_sound);
	void start_forcing_cursor(float fm, float fb, float sm, float sb, float mm, float mb, float em, float eb, bool up);
	void force_to(int x, int y);
	void close_for_decrypt();
	void update_focus();
	std::function<void()> minimize_function(Application application, string icon_name, bool play_animation = true);
	function<void()> tray_function(Application application, string icon_name);
	template<typename T> void move_range(std::vector<T>& vec, int start, int end, int pos);
	Computer(TextureManager* manager);
	function<void()> end_of_forcing_function;
	function<void()> dialogue_confirm_function;
	map<string, bool> read_email;
	map<string, bool> visited_site;
	vector<string> emails;
	vector<unique_ptr<ScreenElement>> to_be_added;
	vector<unique_ptr<ScreenElement>> screen_elements;
	string save_slot;
	File uploaded_image;
	vector<pair<string, Application>> extra_apps;
	string mmo_password = "";
	vector<string> whitelist;
	vector<string> folder_names;
	string get_cur_dir_name(int index);
	File read_image_file(string file_name, string folder_name);
	void delete_file(string file_name, string folder_name);
	vector<File> get_folder_index(string folder_name);
	bool files_present();
	string get_path_name(string folder_name);
	int get_folder_size(string folder_name);
	bool write_file_to_folder(File file, string folder_name);
	void borders_title_x(int ex1, int ey1, int ex2, int ey2, Application application, bool do_x = true, function<void()> on_close = []() {}, bool no_move = false);
	void translate_application(vector<unique_ptr<ScreenElement>>& vec, float delta_x, float delta_y, Application application, bool direct = false);
	void reset_elements();
	void tag_info_box();
	bool file_present(string folder, string file_name);
	bool application_open(Application application);
	void make_everything_stuck();
	int minimize_bucket, close_bucket, url_bucket, scroll_bucket, power_bucket, tray_bucket;
	bool buckets_full = false;
	int breaking_stage = 0;
	int flash_counter;
	void increment_breaking_stage();
	Rect active_fake;
	bool have_active_fake = false;
	void make_fake(Rect region, bool do_black = true);
	vector<ScreenElement*> faked_out;
	void move_fake();
	bool animate_fake = false;
	void restore_browser();
	void go_to_breaking();
	int get_desktop_index();
	bool foreground_if_obscured(Application application, string icon_name);
	void end_minimizing_animation();
	void give_focus_to(ScreenElement* se);
	void stop_website_sounds();
	bool draw_underline = false;
	vector<pair<string, int>> score_table;
	int day_number;
	float mmo_stage_x1, mmo_stage_y1, mmo_player_x1, mmo_player_y1;
	int last_tick = 0;
	int elapsed_time = 0;
	int freeze_time = 0;
	Application restore_application;
	int email_counter = 0;
	bool been_to_title_screen = false;
	bool cursor_lock = false;
	void set_cursor(string new_cursor);
	~Computer();
	void delete_save_file(int file_number);
	void display_pause_screen();
	int title_delay = 0;
	int get_internet_icon();

	int forcing_bound_x1;
	int forcing_bound_x2;
	int forcing_bound_y1;
	int forcing_bound_y2;
	int current_forcing_x;
	int current_forcing_y;

	bool submitted_poll_of_the_day = false;
	bool no_rosecolored1 = true;
	bool visited_elsewhere_after_rosecolored1 = false;
	bool visited_elsewhere_before_rosecolored1 = false;
	bool no_rosecolored2 = true;
	bool looked_for_spider = false;
	bool missed_spider = false;
	bool no_rosecolored_missed_spider = true;
	bool no_rosecolored3 = true;
	bool started_crashing_browser = false;
	bool crashed_browser = false;
	bool update_in_progress = true;
	bool recovered_browser = false;
	bool killed_process = false;
	bool tried_to_view_encrypted_folder = false;
	bool no_rosecolored4 = true;
	bool no_rfh3 = true;
	bool reached_help_center = false;
	bool reached_psych_article = false;
	bool no_rosecolored5 = true;
	bool entered_contest = false;
	bool entered_right_email_address = false;
	bool no_centerracom_trial_link = true;
	bool ran_trial = false;
	bool no_centerracom_survey_link = true;
	bool decryption_broken = false;
	bool the_end = false;
	bool answered_search_question = false;
	bool reached_second_day = false;
	bool reached_psychologist = false;
	bool contest_finished = false;
	bool no_contest_finished_email = true;
	bool showed_walter = false;
	bool reached_space_game = false;
	bool beat_space_game = false;
	bool no_rfh7 = true;

	float cmmx = 0, cmmy = 0;
};

template<typename T> void Computer::move_range(std::vector<T>& vec, int start, int end, int pos)
{
	vector<T> temp;
	temp.resize(end - start);
	std::move(vec.begin() + start, vec.begin() + end, temp.begin());
	vec.erase(vec.begin() + start, vec.begin() + end);
	auto iter = vec.begin();
	if (pos < 0)
		iter = vec.end();

	vec.insert(iter + pos, std::make_move_iterator(temp.begin()), std::make_move_iterator(temp.end()));
}

bool intersects(ScreenElement s1, ScreenElement s2);

#endif