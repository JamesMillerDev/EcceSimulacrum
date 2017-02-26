#ifndef CAMERA
#define CAMERA

#include <vector>
#include <algorithm>
#include "Vector3.h"
#include "GameParams.h"
#include "ExpandableImage.h"

using std::vector;

enum AnimationType {PAUSE, LOOK, POSITION, UP};

struct CameraAnimation
{
	AnimationType type;
	float duration;
	float to_x, to_y, to_z;
	int frames;
	CameraAnimation(AnimationType _type, float _duration, float _to_x, float _to_y, float _to_z) : type(_type), duration(_duration), to_x(_to_x), to_y(_to_y), to_z(_to_z), frames(0) {}
};

struct MovementKey
{
	char key;
	bool active;
	float velocity;
	MovementKey(char _key, bool _active, float _velocity) : key(_key), active(_active), velocity(_velocity) {}
};

class Camera
{
private:
	float prev_mouse_x;
	float prev_mouse_y;
	//float current_acceleration;
	//float current_velocity;
	bool had_mouse_motion;
	bool computer_flag;
	bool do_collision;
	bool positioning;
	bool pos_stage_2;
	int computer_counter;
	vector<MovementKey> keys_pressed;
	vector<vector<CameraAnimation>> animations;
	vector<CameraAnimation> current_row;
	Vector3 get_look_point();
	float step_ref_x;
	float step_ref_z;
	int current_anim_index = 0;
	float cur_x_inc, cur_y_inc, cur_z_inc, cur_horiz_inc, cur_vert_inc;
	ExpandableImage xex, yex, zex, horizex, vertex;

public:
	Camera();
	float camera_x;
	float camera_y;
	float camera_z;
	float aspect;
	void window_resized(int new_width, int new_height);
	void mouse_moved(int new_x, int new_y);
	void mouse_stopped();
	void move(MovementKey move_key);
	void setup_view();
	void press_key(unsigned char key);
	void release_key(unsigned char key);
	void animate();
	void tilt_look_vector(float theta);
	bool play_animations();
	bool computer_ready();
	bool go_to_computer;
	bool backing_up;
	bool went_to_corner = false;
	bool flying_mode = false;
	bool rotating_quad = false;
	float rotation_angle = 0.0;
	ALuint footsteps_source;
	float look_x;
	float look_y;
	float look_z;
	float current_vert_angle;
	float current_horiz_angle;
	float random_horiz_inc;
	float random_vert_inc;
};

#endif