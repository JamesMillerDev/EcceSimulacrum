#ifndef MMO_HEADER
#define MMO_HEADER

#include "Computer.h"
#include "ScrollingMessage.h"
#include "TextField.h"

#define WALK_FRAMES 1

struct Character : public ScreenElement
{
	string char_name;
	string cur_direction;
	int walk_frame = 0;
	Character(int _x1, int _y1, int _x2, int _y2, string _char_name) : ScreenElement(_x1, _y1, _x2, _y2, "black.png"), char_name(_char_name)
	{
		name = char_name + "up" + to_string(walk_frame) + ".png";
	}

	void step(string direction)
	{
		cur_direction = direction;
		walk_frame++;
		if (walk_frame > WALK_FRAMES)
			walk_frame = 0;

		name = char_name + direction + to_string(walk_frame) + ".png";
	}

	void swing()
	{
		name = char_name + cur_direction + "sword.png";
	}

	void normal()
	{
		name = char_name + cur_direction + to_string(walk_frame) + ".png";
	}
};

struct MMO : public ScreenElement
{
	Computer* parent;
	vector<ScreenElement> background_images;
	vector<ScreenElement> bounding_boxes;
	vector<ScreenElement> warp_boxes;
	vector<ScreenElement> animated_components;
	vector<ScrollingMessage> scrolling_messages;
	vector<ScreenElement*> children;
	map<unsigned char, bool> keys_pressed;
	TextField* password_box;
	TextField* email_box;
	ScreenElement* left_bottom;
	ScreenElement* right_top;
	ScreenElement ladder_box;
	int animation_counter = 0;
	int chat_height;
	bool moving_stage_x = true;
	bool moving_stage_y = true;
	bool ignore_collision = false;
	bool playing_warping_animation = false;
	bool swinging_sword = false;
	int sword_counter = 0;
	bool warped;
	bool no_movement;
	bool zoom_animation;
	int frames = 0;
	float warping_alpha = 0.0;
	float warping_delta = 0.02;
	float scale_x = 1.0;
	float scale_y = 1.0;
	int app_state;
	string warp_target;
	string current_level;
	//Character player = Character(950.0, 530.0, 950 + 114.0, 530.0 + 133.0, "main");
	Character player = Character(950.0, 530.0, 950 + 14, 530.0 + 22, "main");
	MMO(float _x1, float _y1, float _x2, float _y2, string _name, string initial_level, Computer* _parent, Application _application);
	void draw(TextureManager* texture_manager);
	void press_key(unsigned char key);
	void release_key(unsigned char key);
	void process_key(unsigned char key);
	void mouse_moved(int x, int y);
	void mouse_clicked(int button, int state, int x, int y);
	void translate_stage(float delta_x, float delta_y);
	void load_level(string file_name);
	void animate();
	void remove_children();
	void add_children();
	void add_chat();
	void load_additional_boxes(string texture_name);
	float percent_of_centering = 0.0;
	int zoom_counter = 0;
	bool animating_stage = false;
	float walk_speed;
	float total_distance = 0;
	int player_to_center_x = 0;
	int player_to_center_y = 0;
};

#endif