#ifndef BUBBLE_GAME
#define BUBBLE_GAME

#include <set>
#include <list>
#include "Computer.h"

bool intersects(ScreenElement s1, ScreenElement s2);

struct Bubble : public ScreenElement
{
	bool player_bubble;
	float trajectory_x;
	float trajectory_y;
	Bubble(float _x1, float _y1, float _x2, float _y2, std::string _name, Application _application, bool _player, float _tx, float _ty) :
		ScreenElement(_x1, _y1, _x2, _y2, _name, _application), player_bubble(_player), trajectory_x(_tx), trajectory_y(_ty) {}
};

struct SpaceGameObject : public ScreenElement
{
	bool mulling = false;
	int type;
	int velocity_x = 0;
	int velocity_y = 0;
	int frame_counter = 0;
	int death_counter = 0;
	float spinning_angle = 0;
	SpaceGameObject(int _x1, int _y1, int _x2, int _y2, string _name, int _type) : ScreenElement(_x1, _y1, _x2, _y2, _name), type(_type)
	{
		int width = glutGet(GLUT_WINDOW_WIDTH);
		int height = glutGet(GLUT_WINDOW_HEIGHT);
		if (width != 1920 || height != 1080)
		{
			x2 = x1 + (x2 - x1) * ((float)width / 1920.0);
			y2 = y1 + (y2 - y1) * ((float)height / 1080.0);
		}

		no_scale = true;
	}

	bool operator==(const SpaceGameObject& rhs) const
	{
		return name == rhs.name && x1 == rhs.x1 && y1 == rhs.y1 && x2 == rhs.x2 && y2 == rhs.y2 && 
			velocity_x == rhs.velocity_x && velocity_y == rhs.velocity_y;
	}

	virtual void draw(TextureManager* texture_manager)
	{
		ScreenElement::draw(texture_manager);
		if (type == 6)
		{
			glPushMatrix();
				glTranslatef(x1 + (x2 - x1) / 2, y1 + (y2 - y1) / 2, 0.0);
				glRotatef(spinning_angle, 0.0, 0.0, 1.0);
				glTranslatef(-(x1 + (x2 - x1) / 2), -(y1 + (y2 - y1) / 2), 0.0);
				ScreenElement line(x1, y1, x2, y2, "spinningline.png");
				line.no_scale = true;
				line.draw(texture_manager);
			glPopMatrix();
		}
	}

	void animate()
	{
		spinning_angle += 6.0;
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
	int health;
	Computer* parent;
	BubbleGame(float _x1, float _y1, float _x2, float _y2, std::string _name, Computer* _parent, Application _application);
	enum State {TITLE_SCREEN, PLAYING, GAME_OVER};
	State game_state;
	bool show_message;
	bool still_exploding = true;
	bool show_cur_letter = true;
	bool show_game_over = false;
	bool invincible = false;
	int cur_index = 0;
	int score_index = -1;
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
	bool done = false;
	float angle = 0.0;
	float wx = 0, wy = 0;
	float d = 0;
	float mouse_moved_x = 0, mouse_moved_y = 0;
	int timebase;
	int player_world_x = 0;
	int player_world_y = 0;
	vector<SpaceGameObject> game_objects;
	vector<pair<string, int>> score_table;
	bool is_in_range(int target, int lower, int upper);
	pair<int, int> velocity;
	const int SHIP_VELOCITY = 20;
	int frames = 0;
	int mouse_held_frames = 0;
	bool mouse_held = false;
	int aliens_to_spawn = 1;
	int score = 0;
	int score_to_give = 200;
	float half_x, half_y;
	int hit_frame;
};

#endif