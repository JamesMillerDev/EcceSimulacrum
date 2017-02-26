#ifndef COMPUTER
#define COMPUTER

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "TextureManager.h"

using std::string;
using std::vector;
using std::unique_ptr;
using std::pair;

enum ComputerState {STATE_STARTUP, STATE_DESKTOP};

enum Application {NONE, BROWSER, FILES, CONSOLE, INFO_BOX, PAINT, VIDEO_APP, DECRYPTION_APP, MMO_GAME};

float scalex(float x);
float scaley(float y);
float unscalex(float x);
float unscaley(float y);

struct ScrollPanel;

struct Message
{
	string text;
	int delay_ms;
	Message(string _text, int _delay_ms): text(_text), delay_ms(_delay_ms) {}
};

struct File
{
	string name;
	GLubyte* image;
	int xsize;
	int ysize;
};

struct ScreenElement
{
	float x1, y1, x2, y2;
	string name;
	bool has_focus;
	bool is_visible;
	bool backup_visibility;
	bool marked_for_deletion;
	bool transient;
	bool browser_child = false;
	bool partial = false;
	Application application;
	bool cache_texture = false;
	bool no_scale = false;
	bool never_focus;
	bool receive_out_of_bounds_clicks = false;
	bool surrender_focus_on_up_click;
	float angle = 0;
	int texture_id = 0;
	ScreenElement(float _x1, float _y1, float _x2, float _y2, string _name, Application _application = NONE, bool _cache_texture = false) : x1(_x1), y1(_y1), x2(_x2), y2(_y2), name(_name), has_focus(false), is_visible(true), application(_application), marked_for_deletion(false), transient(false), cache_texture(_cache_texture), never_focus(false), receive_out_of_bounds_clicks(false), surrender_focus_on_up_click(true), partial(false) {}
	virtual void mouse_over(int x, int y) {}
	virtual void mouse_off() {}
	virtual void mouse_clicked(int button, int state, int x, int y) {}
    virtual void mouse_moved(int x, int y) {}
	virtual void animate() {}
	virtual void draw(TextureManager* texture_manager)
	{
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
			texture_id = texture_manager->index_map[name];

		float width = glutGet(GLUT_WINDOW_WIDTH);
		float height = glutGet(GLUT_WINDOW_HEIGHT);
		if ((width == 1920.0 && height == 1080.0) || no_scale)
		{
			if (cache_texture)
				texture_manager->change_texture(texture_id);

			else texture_manager->change_texture(name);
			float extent = partial ? (float)(y2 - y1) / (float)texture_manager->get_height(name) : 1.0;
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, extent); glVertex2f(x1 - x1, y1 - y1);
			glTexCoord2f(0.0, 0.0); glVertex2f(x1 - x1, y2 - y1);
			glTexCoord2f(1.0, 0.0); glVertex2f(x2 - x1, y2 - y1);
			glTexCoord2f(1.0, extent); glVertex2f(x2 - x1, y1 - y1);
			glEnd();
		}

		else
		{
			if (cache_texture)
				texture_manager->change_texture(texture_id);
			
			else texture_manager->change_texture(name);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 1.0); glVertex2f(x1 * (width / 1920.0) - x1, y1 * (height / 1080.0) - y1);
			glTexCoord2f(0.0, 0.0); glVertex2f(x1 * (width / 1920.0) - x1, y2 * (height / 1080.0) - y1);
			glTexCoord2f(1.0, 0.0); glVertex2f(x2 * (width / 1920.0) - x1, y2 * (height / 1080.0) - y1);
			glTexCoord2f(1.0, 1.0); glVertex2f(x2 * (width / 1920.0) - x1, y1 * (height / 1080.0) - y1);
			glEnd();
		}

		glPopMatrix();
		if (changed_name)
			name = "|" + name;
	}

	void translate(float delta_x, float delta_y)
	{
		x1 += delta_x;
		x2 += delta_x;
		y1 += delta_y;
		y2 += delta_y;
	}

	pair<float, float> center()
	{
		int center_x = glutGet(GLUT_WINDOW_WIDTH) / 2;
		int center_y = glutGet(GLUT_WINDOW_HEIGHT) / 2;
		float delta_x = center_x - ((x2 - x1) / 2 + x1);
		float delta_y = center_y - ((y2 - y1) / 2 + y1);
		translate(delta_x, delta_y);
		return pair<int, int>(delta_x, delta_y);
	}

	virtual void give_focus() { has_focus = true; }
	virtual void take_focus() { has_focus = false; }
	virtual void press_key(unsigned char key) {}
	virtual void release_key(unsigned char key) {}
	bool operator==(const ScreenElement& rhs) const
	{
		return x1 == rhs.x1 && y1 == rhs.y1 && x2 == rhs.x2 && y2 == rhs.y2 && name == rhs.name;
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
	ScreenElement* focus_element;
	Application minimizing_application;
	GLfloat minimizing_alpha;
	bool alpha_going_up;
	bool play_minimizing_animation;
	GLuint minimizing_framebuffer;
	GLuint minimizing_texture;
	int min_x1 = -1, min_y1 = -1, min_x2 = -1, min_y2 = -1;
	int cur_min_x1, cur_min_y1, cur_min_x2, cur_min_y2, target_minimize_x;
	float first_m, first_b, second_m, second_b, movable_m, movable_b, end_m, end_b;
	bool going_up;

public:
	ComputerState computer_state;
	int prev_mouse_x, prev_mouse_y;
	int sticking_x, sticking_y;
	TextureManager* texture_manager;
	vector<vector<char> > char_grid;
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
	int scroll_lock_x, scroll_lock_y;
	ScreenElement* exempt_from_forcing = NULL;
	int time_since_escape_sound_started;
    void window_resized(int new_width, int new_height);
	void mouse_clicked(int button, int state, int x, int y);
	void mouse_moved(int x, int y);
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
	void start_application(Application application, string info_text = "", bool file_dialog = false, bool progress_dialog = false, int download_action = 0, ScrollPanel* scroll_panel = NULL);
	void close_application(Application application);
	void draw_messages(float xtrans = 0.0, float ytrans = 0.0, float scale = 1.0);
	void start_playing_escape_sound();
	void play_escape_sound();
	void start_forcing_cursor(float fm, float fb, float sm, float sb, float mm, float mb, float em, float eb, bool up);
	void force_to(int x, int y);
	void play_sound(int index);
	void stop_sound(int index);
	bool is_playing(int index);
	void change_volume();
	std::function<void()> minimize_function(Application application, string icon_name);
	template<typename T> void move_range(std::vector<T>& vec, int start, int end, int pos);
	Computer(TextureManager* manager);
	function<void()> end_of_forcing_function;
	map<string, bool> read_email;
	map<string, bool> visited_site;
	vector<string> emails;
	vector<unique_ptr<ScreenElement>> to_be_added;
	vector<unique_ptr<ScreenElement>> screen_elements;
	map<string, vector<File>> file_system;
	File uploaded_image;
	vector<pair<string, Application>> extra_apps;
	vector<ALuint> sound_sources;
	string mmo_password = "";
	vector<string> whitelist;

	bool no_rosecolored1 = true;
	bool visited_elsewhere_after_rosecolored1 = false;
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
	bool reached_help_center = false;
	bool reached_psych_article = false;
	bool no_rosecolored5 = true;
	bool entered_contest = false;
	bool entered_right_email_address = false;
	bool no_centerracom_trial_link = true;
	bool ran_trial = false;
	bool no_centerracom_survey_link = true;

	float cmmx = 0, cmmy = 0;
};

#endif