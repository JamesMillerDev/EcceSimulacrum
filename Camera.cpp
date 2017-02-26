#include <cmath>
#include <iostream>
#include "GameParams.h"
#include "Camera.h"
#include "keymappings.h"

using namespace std;

Camera::Camera()
{
	random_horiz_inc = 0.01;
	random_vert_inc = 0.01;
	camera_x = INITIAL_WALKING_X;
	camera_y = 5; //5
	camera_z = INITIAL_WALKING_Z;
	look_x = 0.0;//0.0; 0
	look_y = 0.0;//1.0; 0
	look_z = -1.0;//0.0; -1
	current_vert_angle = INITIAL_VERT_ANGLE;//1.57;
	current_horiz_angle = INITIAL_HORIZ_ANGLE;//1.57;
	had_mouse_motion = false;
	computer_counter = 0;
	computer_flag = false;
	positioning = false;
	go_to_computer = false;
	pos_stage_2 = false;
	backing_up = false;
	do_collision = true; //TODO change back when ready
	step_ref_x = INITIAL_WALKING_X;
	step_ref_z = INITIAL_WALKING_Z;

	vector<CameraAnimation> row;
	row.push_back(CameraAnimation(LOOK, 1.8, .3, INITIAL_VERT_ANGLE, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, .5, 0.01, INITIAL_VERT_ANGLE, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(UP, 1, 0.0, 1.0, 0.0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 1.5, 0.01, 1.57, 0));
	row.push_back(CameraAnimation(POSITION, 1.5, camera_x, 40, camera_z + 5));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 0.7, .2, 1.7, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 0.7, -0.07, 1.7, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 1, 0.01, 2, 0));
	row.push_back(CameraAnimation(POSITION, 1, camera_x, 20, camera_z));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 2, 0.01, 1.57, 0));
	row.push_back(CameraAnimation(POSITION, 2, INITIAL_WALKING_X, INITIAL_WALKING_Y, INITIAL_WALKING_Z));
	animations.push_back(row);
}

void Camera::mouse_stopped()
{
	had_mouse_motion = false;
}

void Camera::window_resized(int new_width, int new_height)
{
	aspect = (float) new_width / (float) new_height;
}

Vector3 Camera::get_look_point()
{
	Vector3 look_point;
	look_point.x = sin(current_vert_angle) * sin(current_horiz_angle) + camera_x;
	look_point.y = cos(current_vert_angle) + camera_y;
	look_point.z = sin(current_vert_angle) * cos(current_horiz_angle) + camera_z;
	return look_point;
}

void Camera::setup_view()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(CAMERA_FOVY, aspect, Z_NEAR, Z_FAR);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	Vector3 look_point = get_look_point();
	gluLookAt(camera_x, camera_y, camera_z, look_point.x, look_point.y, look_point.z, look_x, look_y, look_z);
	ALfloat orientation[] = { look_point.x, look_point.y, look_point.z, look_x, look_y, look_z };
	alListener3f(AL_POSITION, camera_x, camera_y, camera_z);
	alListenerfv(AL_ORIENTATION, orientation);
	alSource3f(footsteps_source, AL_POSITION, camera_x, 0.0, camera_y);
}

float same_sign(float x, float y)
{
	if (y == 0)
		return x;

	return fabs(x) * (y / fabs(y));
}

bool between(float x, float y, float z)
{
	return ((x < y) && (y < z)) || ((z < y) && (y < x));
}

void Camera::move(MovementKey move_key)
{
	Vector3 look_point = get_look_point();
	float look_x = look_point.x;
	float look_y = look_point.y;
	float look_z = look_point.z;
	float slope = (float) (look_z - camera_z) / (float) (look_x - camera_x);
	float velocity_x;
	float velocity_z;
	float velocity_y = 0.0;
	float current_velocity = move_key.velocity;
	if (move_key.key == 'w' || move_key.key == 's') //TODO Capital letters?  Scancodes instead of chars?
	{
		if (look_x == camera_x)
		{
			velocity_x = 0.0;
			velocity_z = same_sign(current_velocity, look_z - camera_z);
		}
		
		else
		{
			velocity_x = current_velocity / sqrt(1.0 + (slope * slope));
			velocity_z = (slope * current_velocity) / sqrt(1.0 + (slope * slope));
			velocity_x = same_sign(velocity_x, look_x - camera_x);
			velocity_z = same_sign(velocity_z, look_z - camera_z);
		}
	}

	else if (move_key.key == 'a' || move_key.key == 'd')
	{
		if (look_x == camera_x)
		{
			velocity_x = same_sign(current_velocity, -1.0 * (look_z - camera_z));
			velocity_z = 0.0;
		}

		else if (look_z == camera_z)
		{
			velocity_x = 0.0;
			velocity_z = same_sign(current_velocity, look_x - camera_x);
		}

		else
		{
			slope = -1.0 / slope;
			velocity_x = current_velocity / sqrt(1.0 + (slope * slope));
			velocity_z = (slope * current_velocity) / sqrt(1.0 + (slope * slope));
			velocity_x = same_sign(velocity_x, -1.0 * (look_z - camera_z));
			velocity_z = same_sign(velocity_z, look_x - camera_x);
		}
	}

	else return;

	if (move_key.key == 's' || move_key.key == 'a')
	{
		velocity_x *= -1.0;
		velocity_z *= -1.0;
	}

	int old_x = camera_x;
	int old_z = camera_z;
	if (camera_x + velocity_x < WALL_DISTANCE - MIN_DISTANCE_FROM_WALL &&
		camera_x + velocity_x > -WALL_DISTANCE + MIN_DISTANCE_FROM_WALL || flying_mode)
	{
		if (!rotating_quad)
			camera_x += velocity_x;

		else
		{
			rotation_angle += velocity_x / 5.0;
			if (rotation_angle >= 90)
				rotating_quad = false;

			if (rotation_angle < 0)
				rotation_angle = 0;
		}
	}

	if (camera_z + velocity_z < WALL_DISTANCE - MIN_DISTANCE_FROM_WALL &&
		camera_z + velocity_z > -WALL_DISTANCE + MIN_DISTANCE_FROM_WALL || flying_mode)
	{
		if (!rotating_quad)
			camera_z += velocity_z;
	}

	if (flying_mode && (move_key.key == 'w' || move_key.key == 's'))
	{
		float dist = sqrt(pow(camera_x - old_x, 2) + pow(camera_z - old_z, 2));
		float slope = tan(current_vert_angle - 1.57);
		velocity_y = -slope * dist;
		if (move_key.key == 's')
			velocity_y *= -1;
	}

	//TODO Only when computer is visible!
	if (do_collision)
	{
		if ((270 < camera_x && camera_x < 330) &&
			(-40 < camera_z && camera_z < 40))
		{
			camera_x = old_x;
			camera_z = old_z;
			if (camera_x <= 270)
			{
				positioning = true;
				keys_pressed.clear();
			}
		}
	}

	camera_y += velocity_y;
	alListener3f(AL_VELOCITY, velocity_x, velocity_y, velocity_z); //Velocity never hits 0 even when stationary?
	alSource3f(footsteps_source, AL_VELOCITY, velocity_x, velocity_y, velocity_z);
	if (sqrt(pow(camera_x - step_ref_x, 2) + pow(camera_z - step_ref_z, 2)) >= 150.0)
	{
		alSourcePlay(footsteps_source);
		step_ref_x = camera_x;
		step_ref_z = camera_z;
	}

	if (camera_x < -300 && camera_z < -300)
		went_to_corner = true;
}

void Camera::mouse_moved(int new_x, int new_y) //Speed of camera stays constant with different window sizes?
{
	if (positioning)
		return;

	if (rotating_quad)
		return;
	
	if (new_x == 100 && new_y == 100)
		return;

	if (!had_mouse_motion)
	{
		had_mouse_motion = true;
		glutWarpPointer(100,100);
		return;
	}

	current_horiz_angle += (new_x - 100) * -LOOK_VELOCITY;
	current_vert_angle += (new_y - 100) * LOOK_VELOCITY;
	if (current_horiz_angle < 0)
		current_horiz_angle = (2 * PI) + current_horiz_angle;

	if (current_horiz_angle >(2 * PI))
		current_horiz_angle -= (2 * PI);

	if (current_vert_angle > VERT_UPPER_BOUND)
		current_vert_angle = VERT_UPPER_BOUND;

	if (current_vert_angle < VERT_LOWER_BOUND)
		current_vert_angle = VERT_LOWER_BOUND;

	glutWarpPointer(100, 100);
}

void Camera::press_key(unsigned char key)
{
	/*if (key == 'u')
		camera_y += 1.0;

	if (key == 'j')
		camera_y -= 1.0;

	if (key == 'i')
		look_x += 0.5;

	if (key == 'k')
		look_x -= 0.5;

	if (key == 'o')
		look_y += 0.5;

	if (key == 'l')
		look_y -= 0.5;

	if (key == 'p')
		look_z += 0.5;

	if (key == ';')
		look_z -= 0.5;

	if (key == 'm')
	{
		camera_y = 80.0;
		look_x = 0;
		look_y = 1;
		look_z = 0;
	}*/

	if (positioning)
		return;
	
	for (int i = 0; i < keys_pressed.size(); ++i)
	{
		if (keys_pressed[i].key == key)
			return;
	}

	keys_pressed.push_back(MovementKey(key, true, WALK_ACCELERATION));
}

void Camera::release_key(unsigned char key)
{
	for (int i = 0; i < keys_pressed.size(); ++i)
	{
		if (keys_pressed[i].key == key)
			keys_pressed[i].active = false;
	}
}

void Camera::animate()
{
	if (backing_up)
	{
		camera_x -= 2.0;
		if (camera_x <= 250.0)
			backing_up = false;

		return;
	}

	if (positioning)
	{
		bool finished_xz_movement = false;
		if (camera_x == 250.0)
		{
			camera_z = 0.0;
			finished_xz_movement = true;
		}

		else if (!pos_stage_2)
		{
			float slope = (0.0 - camera_z) / (250.0 - camera_x);
			float delta_x = sqrt(pow(1, 2) / (1 + pow(slope, 2)));
			float delta_z = delta_x * slope;
			float old_x = camera_x;
			float old_z = camera_z;
			camera_x -= delta_x;
			camera_z -= delta_z;
			if (between(old_x, 250.0, camera_x) || between(old_z, 0.0, camera_z))
			{
				camera_x = 250.0;
				camera_z = 0.0;
			}
		}

		if (current_horiz_angle == PI / 2.0)
		{
			current_vert_angle = PI / 2.0;
			if (finished_xz_movement)
				pos_stage_2 = true;
		}

		else
		{
			float slope = (PI / 2 - current_vert_angle) / (PI / 2 - current_horiz_angle);
			float delta_horiz = sqrt(pow(.05, 2) / (1 + pow(slope, 2)));
			float delta_vert = delta_horiz * slope;
			if (current_horiz_angle > PI / 2)
			{
				delta_horiz *= -1.0;
				delta_vert *= -1.0;
			}

			float old_horiz = current_horiz_angle;
			float old_vert = current_vert_angle;
			current_horiz_angle += delta_horiz;
			current_vert_angle += delta_vert;
			if (between(old_horiz, PI / 2, current_horiz_angle) || between(old_vert, PI / 2, current_vert_angle))
			{
				current_horiz_angle = PI / 2;
				current_vert_angle = PI / 2;
			}
		}

		if (pos_stage_2)
		{
			camera_x += 2.0;
			if (camera_x >= 290.0)
			{
				go_to_computer = true;
				positioning = false;
				pos_stage_2 = false;
			}
		}

		return;
	}
	
	if (keys_pressed.empty())
	{
		step_ref_x = camera_x;
		step_ref_z = camera_z;
	}

	for (vector<MovementKey>::iterator it = keys_pressed.begin(); it != keys_pressed.end();)
	{
		if (it->active)
			it->velocity += WALK_ACCELERATION;

		else it->velocity -= WALK_ACCELERATION;
		if (it->velocity > WALK_SPEED)
			it->velocity = WALK_SPEED;

		else if (it->velocity <= 0.0)
		{
			it = keys_pressed.erase(it);
			continue;
		}

		move(*it);
		if (positioning)
			break;

		++it;
	}
}

void Camera::tilt_look_vector(float theta)
{
	look_x = cos(theta);
	look_y = sin(theta);
}

bool Camera::computer_ready()
{
	Vector3 look_point = get_look_point();
	if (!computer_flag)
	{
		if (camera_x < 0.0 && look_point.x < camera_x)
		{
			computer_counter++;
			computer_flag = true;
		}
	}

	else
	{
		if ( !(camera_x < 0.0) && !(look_point.x < camera_x) )
			computer_flag = false;
	}

	if (computer_counter >= NUM_LOOKS)
	{
		do_collision = true;
		return true;
	}

	return false;
}

bool Camera::play_animations()
{
	if (current_anim_index >= animations.size())
		return false;

	if (current_row.empty())
	{
		current_row = animations[current_anim_index];
		for (int i = 0; i < current_row.size(); ++i)
		{
			if (current_row[i].type == LOOK)
			{
				horizex = ExpandableImage(0, current_horiz_angle, 0, current_row[i].to_x, "", NONE, abs(current_row[i].to_x - current_horiz_angle), current_row[i].duration, true);
				vertex = ExpandableImage(0, current_vert_angle, 0, current_row[i].to_y, "", NONE, abs(current_row[i].to_y - current_vert_angle), current_row[i].duration, true);
				horizex.toggle();
				vertex.toggle();
				horizex.speed_sign = copysign(1, current_row[i].to_x - current_horiz_angle);
				vertex.speed_sign = copysign(1, current_row[i].to_y - current_vert_angle);
			}

			else if (current_row[i].type == POSITION)
			{
				xex = ExpandableImage(0, camera_x, 0, current_row[i].to_x, "", NONE, abs(current_row[i].to_x - camera_x), current_row[i].duration, true);
				yex = ExpandableImage(0, camera_y, 0, current_row[i].to_y, "", NONE, abs(current_row[i].to_y - camera_y), current_row[i].duration, true);
				zex = ExpandableImage(0, camera_z, 0, current_row[i].to_z, "", NONE, abs(current_row[i].to_z - camera_z), current_row[i].duration, true);
				xex.toggle();
				yex.toggle();
				zex.toggle();
				xex.speed_sign = copysign(1, current_row[i].to_x - camera_x);
				yex.speed_sign = copysign(1, current_row[i].to_y - camera_y);
				zex.speed_sign = copysign(1, current_row[i].to_z - camera_z);
			}
		}
	}

	for (int i = 0; i < current_row.size(); ++i)
	{
		if (current_row[i].type == UP)
		{
			look_x = current_row[i].to_x;
			look_y = current_row[i].to_y;
			look_z = current_row[i].to_z;
		}
	}

	bool anim = xex.animating || yex.animating || zex.animating || horizex.animating || vertex.animating;
	if (xex.animating)
	{
		xex.animate();
		camera_x = xex.y1;
	}

	if (yex.animating)
	{
		yex.animate();
		camera_y = yex.y1;
	}

	if (zex.animating)
	{
		zex.animate();
		camera_z = zex.y1;
	}

	if (horizex.animating)
	{
		horizex.animate();
		current_horiz_angle = horizex.y1;
	}

	if (vertex.animating)
	{
		vertex.animate();
		current_vert_angle = vertex.y1;
	}

	if (!anim)
	{
		current_row.clear();
		current_anim_index++;
	}

	return true;
}

/*bool Camera::play_animations()
{
	if (current_anim_index >= animations.size())
		return false;

	//if (rand() % 60 == 0)
	//	random_horiz_inc = (float)(rand() % 60) / 600.0;

	//if (rand() % 60 == 0)
	//	random_vert_inc = (float)(rand() % 60) / 600.0;

	//current_horiz_angle += random_horiz_inc;
	//current_vert_angle += random_vert_inc;

	if (current_row.empty())
	{
		current_row = animations[current_anim_index];
		for (int i = 0; i < current_row.size(); ++i)
		{
			if (current_row[i].type == LOOK)
			{
				cur_horiz_inc = (current_row[i].to_x - current_horiz_angle) / (current_row[i].duration * 60.0);
				cur_vert_inc = (current_row[i].to_y - current_vert_angle) / (current_row[i].duration * 60.0);
			}

			else if (current_row[i].type == POSITION)
			{
				cur_x_inc = (current_row[i].to_x - camera_x) / (current_row[i].duration * 60.0);
				cur_y_inc = (current_row[i].to_y - camera_y) / (current_row[i].duration * 60.0);
				cur_z_inc = (current_row[i].to_z - camera_z) / (current_row[i].duration * 60.0);
			}
		}
	}

	bool anim = false;
	for (int i = 0; i < current_row.size(); ++i)
	{
		if (current_row[i].frames < current_row[i].duration * 60)
		{
			if (current_row[i].type == LOOK)
			{
				current_horiz_angle += cur_horiz_inc;
				current_vert_angle += cur_vert_inc;
			}

			else if (current_row[i].type == POSITION)
			{
				camera_x += cur_x_inc;
				camera_y += cur_y_inc;
				camera_z += cur_z_inc;
			}

			else if (current_row[i].type == UP)
			{
				look_x = current_row[i].to_x;
				look_y = current_row[i].to_y;
				look_z = current_row[i].to_z;
				current_row[i].frames = current_row[i].duration * 60;
			}

			current_row[i].frames++;
			anim = true;
		}
	}

	if (!anim)
	{
		current_row.clear();
		current_anim_index++;
	}

	return true;
}*/