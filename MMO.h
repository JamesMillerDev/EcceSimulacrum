#ifndef MMO_HEADER
#define MMO_HEADER

#include "Computer.h"
#include "ScrollingMessage.h"
#include "TextField.h"

struct MMO : public ScreenElement
{
	Computer* parent;
	vector<ScreenElement> background_images;
	vector<ScreenElement> bounding_boxes;
	vector<ScreenElement> warp_boxes;
	vector<ScreenElement> animated_components;
	vector<ScrollingMessage> scrolling_messages;
	vector<ScreenElement*> children;
	TextField* password_box;
	TextField* email_box;
	ScreenElement* left_bottom;
	ScreenElement* right_top;
	string frame1 = "frame1.png";
	string frame2 = "frame2.png";
	int animation_counter = 0;
	bool moving_stage_x = true;
	bool moving_stage_y = true;
	bool playing_warping_animation = false;
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
	ScreenElement player = ScreenElement(950.0, 530.0, 950 + 114.0, 530.0 + 133.0, "icon_decrypt.png");
	MMO(float _x1, float _y1, float _x2, float _y2, string _name, string initial_level, Computer* _parent, Application _application);
	void draw(TextureManager* texture_manager);
	void press_key(unsigned char key);
	void mouse_moved(int x, int y);
	void translate_stage(float delta_x, float delta_y);
	void load_level(string file_name);
	void animate();
	void remove_children();
	void add_children();
};

#endif