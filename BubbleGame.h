#ifndef BUBBLE_GAME
#define BUBBLE_GAME

#include <set>
#include <list>
#include <algorithm>
#include "Computer.h"
#include "ScrollPanel.h"

struct Bubble : public ScreenElement
{
	bool player_bubble;
	float trajectory_x;
	float trajectory_y;
	Bubble(float _x1, float _y1, float _x2, float _y2, std::string _name, Application _application, bool _player, float _tx, float _ty) :
		ScreenElement(_x1, _y1, _x2, _y2, _name, _application), player_bubble(_player), trajectory_x(_tx), trajectory_y(_ty) {}
};

struct FuzzyNumber
{
	int value;
	int inc;
	FuzzyNumber() : value(60), inc(2) {}
	int next()
	{
		value += inc;
		if (value >= 80)
		{
			value = 80;
			inc = -1;
		}

		if (value <= 40)
		{
			value = 40;
			inc = 1;
		}

		if (rand() % 20 == 0)
			inc *= -1;

		return value;
	}
};

struct SpaceGameObject : public ScreenElement
{
	vector<ScreenElement> circles;
	bool mulling = false;
	bool slow_alien = false;
	int type;
	int velocity_x = 0;
	int velocity_y = 0;
	int frame_counter = 0;
	int death_counter = 0;
	int hit_on_frame = 0;
	int health;
	float spinning_angle = 0;
	float move_angle;
	float move_direction;
	FuzzyNumber a, b, c, d;
	SpaceGameObject(int _x1, int _y1, int _x2, int _y2, string _name, int _type) : ScreenElement(_x1, _y1, _x2, _y2, _name), type(_type)
	{
		if (type == 0 || type == 2 || type == 3)
			cache_texture = true;

		move_angle = 0.0;
		health = 4;
		int width = glutGet(GLUT_WINDOW_WIDTH);
		int height = glutGet(GLUT_WINDOW_HEIGHT);
		if (width != 1920 || height != 1080)
		{
			x2 = x1 + (x2 - x1) * ((float)width / 1920.0);
			y2 = y1 + (y2 - y1) * ((float)height / 1080.0);
		}

		no_scale = true;
		move_direction = 1;
		/*if (type == 6)
		{
			int center_x = x1 + (x2 - x1) / 2;
			int center_y = y1 + (y2 - y1) / 2;
			for (int i = 0; i < 360; ++i)
			{
				int cx = cos(i) * 55 + center_x;
				int cy = sin(i) * 55 + center_y;
				ScreenElement circle = ScreenElement(cx - 2, cy - 2, cx + 3, cy + 3, "bullet.png");
				circle.sliding_goal_x = rand() % 2 - 1;
				circle.info_box_id = rand() % 2 - 1;
				circles.push_back(circle);
			}
		}*/
	}

	bool operator==(const SpaceGameObject& rhs) const
	{
		return name == rhs.name && x1 == rhs.x1 && y1 == rhs.y1 && x2 == rhs.x2 && y2 == rhs.y2 && 
			velocity_x == rhs.velocity_x && velocity_y == rhs.velocity_y;
	}

	virtual void draw(TextureManager* texture_manager)
	{
		if (type == 7 || type == 6)
		{
			for (int i = 0; i < circles.size(); ++i)
			{
				circles[i].no_scale = true;
				circles[i].draw(texture_manager);
			}
		}

		ScreenElement::draw(texture_manager);
		/*if (type == 6)
		{
			glPushMatrix();
			glTranslatef(x1 + (x2 - x1) / 2, y1 + (y2 - y1) / 2, 0.0);
			glRotatef(spinning_angle, 0.0, 0.0, 1.0);
			glTranslatef(-(x1 + (x2 - x1) / 2), -(y1 + (y2 - y1) / 2), 0.0);
			ScreenElement line(x1, y1, x2, y2, "spinningline.png");
			line.no_scale = true;
			line.draw(texture_manager);
			glPopMatrix();
			//for (int i = 0; i < circles.size(); ++i)
			//	circles[i].draw(texture_manager);
		}*/

		/*if (type == 7)
		{
			glPushMatrix();
			glTranslatef(x1 + (x2 - x1) / 2, y1 + (y2 - y1) / 2, 0.0);
			glRotatef(spinning_angle, 0.0, 0.0, 1.0);
			glTranslatef(-(x1 + (x2 - x1) / 2), -(y1 + (y2 - y1) / 2), 0.0);
			ScreenElement fire(x1 - a.next(), y1 - b.next(), x2 + c.next(), y2 + d.next(), "bluefire.png");
			fire.no_scale = true;
			fire.draw(texture_manager);
			glPopMatrix();
			ScreenElement window(x1 - 150, y1 - 150, x2 + 150, y2 + 150, "circlewindow.png");
			//window.draw(texture_manager);
		}*/
	}

	void animate()
	{
		spinning_angle += 3.0;
		for (int i = 0; i < circles.size(); ++i)
		{
			circles[i].translate(circles[i].sliding_goal_x, circles[i].info_box_id);
			if ((circles[i].x1 < x1 || circles[i].x1 > x2) || (circles[i].y1 < y1 || circles[i].y1 > y2))
				circles[i].alpha -= 0.3;
		}

		int cx = x1 + (x2 - x1) / 2;
		int cy = y1 + (y2 - y1) / 2;
		ScreenElement circle = ScreenElement(cx - 2, cy - 2, cx + 3, cy + 3, type == 6 ? "greenbullet.png" : "bullet.png");
		circle.sliding_goal_x = rand() % 20 - 10;
		circle.info_box_id = (10 - abs(circle.sliding_goal_x)) * (rand() % 2 == 0 ? -1 : 1);
		circles.push_back(circle);
		circles.erase(remove_if(circles.begin(), circles.end(), [](ScreenElement circle) {return circle.alpha <= 0.0;}), circles.end());
		/*int center_x = x1 + (x2 - x1) / 2;
		int center_y = y1 + (y2 - y1) / 2;
		for (int i = 0; i < circles.size(); ++i)
		{
			circles[i].translate(circles[i].sliding_goal_x, circles[i].info_box_id);
			float distance = sqrt(pow(circles[i].x1 - center_x, 2) + pow(circles[i].y1 - center_y, 2));
			if (distance < 45 || distance > 65)
			{
				circles[i].sliding_goal_x *= -1;
				circles[i].info_box_id *= -1;
			}
		}*/
	}
};

/*struct HealthPickup : public SpaceGameObject
{
	float spinning_angle;
	HealthPickup(int _x1, int _y1, int _x2, int _y2, string _name) : SpaceGameObject(_x1, _y1, _x2, _y2, _name, 6), spinning_angle(0) {}
	void animate()
	{
		spinning_angle += 6.0;
	}

	void draw(TextureManager* texture_manager)
	{
		SpaceGameObject::draw(texture_manager);
		ScreenElement line(x1, y1, x2, y2, "spinningline.png");
		line.angle = spinning_angle;
		line.draw(texture_manager);
	}
};*/

struct BubbleGame : public ScreenElement
{
	ScrollPanel* scroll_panel;
	int health;
	int spawn_interval;
	string prev_name;
	Computer* parent;
	BubbleGame(float _x1, float _y1, float _x2, float _y2, std::string _name, Computer* _parent, Application _application);
	~BubbleGame();
	enum State {TITLE_SCREEN, PLAYING, GAME_OVER};
	State game_state;
	bool show_message;
	bool extending;
	bool still_exploding = true;
	bool show_cur_letter = true;
	bool show_game_over = false;
	bool invincible = false;
	int cur_index = 0;
	int score_index = -1;
	int extend_counter;
	void draw(TextureManager* texture_manager);
	void mouse_clicked(int button, int state, int x, int y);
	void press_key(unsigned char key);
	void release_key(unsigned char key);
	void mouse_moved(int x, int y);
	void give_focus();
	void animate();
	void add_health(int value);
	void subtract_health(int value);
	void kill_alien(SpaceGameObject* target);
	void generate_stars();
	void draw_score_table(TextureManager* texture_manager);
	void initiate_game();
	void create_debris(int target_center_x, int target_center_y);
	void process_hit();
	bool done = false;
	bool score_mode;
	float angle = 0.0;
	float wx = 0, wy = 0;
	float d = 0;
	float mouse_moved_x = 0, mouse_moved_y = 0;
	int timebase;
	int player_world_x = 0;
	int player_world_y = 0;
	vector<SpaceGameObject> game_objects;
	vector<SpaceGameObject> prev_objects;
	vector<SpaceGameObject> shot_particles;
	vector<SpaceGameObject> flame_particles;
	//vector<pair<string, int>> score_table;
	bool is_in_range(int target, int lower, int upper);
	pair<int, int> velocity;
	int SHIP_VELOCITY = 14;//20;
	int fuel;
	int frames = 0;
	int mouse_held_frames = 0;
	bool mouse_held = false;
	int aliens_to_spawn = 1;
	int score = 0;
	int score_to_give = 200;
	float half_x, half_y;
	int hit_frame;
	bool do_glow_animation = false;
	bool green_glow = false;
	float glow_alpha = 0;
	int glow_sign;
	float flame_length = 0;
	bool do_flame_animation = false;
	float flame_target_length = 0;
	int flame_frame = 1;
	int flame_frame_increment = 1;
	bool spawn_slow_aliens = false;
	void spawn_aliens(int num_to_spawn);
	int slow_aliens_killed = 0;
	vector<SpaceGameObject> to_add;
	int fire_interval;
	int max_health;
	bool rapid_fire = false;
	bool responds_to(int button);
};

#endif