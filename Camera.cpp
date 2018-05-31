#include <cmath>
#include <iostream>
#include "GameParams.h"
#include "Camera.h"
#include "keymappings.h"

using namespace std;

Camera::Camera()
{
	frames = 0;
	frame_last_step = 0;
	computer_present = false;
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
	do_collision = false;
	step_threshold = 150.0;
	old_threshold = 150.0;
	if (DEBUG)
		do_collision = true;

	step_ref_x = INITIAL_WALKING_X;
	step_ref_z = INITIAL_WALKING_Z;

	vector<CameraAnimation> row;
	row.push_back(CameraAnimation(LOOK, 1.8, .5, INITIAL_VERT_ANGLE, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(PAUSE, .5, 0, 0, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 2, .5, INITIAL_VERT_ANGLE - 0.02, 0)); //duration .5
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(PAUSE, .5, 0, 0, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 2.5, 0.01, INITIAL_VERT_ANGLE, 0)); //duration .5
	animations.push_back(row);
	row.clear();
	//row.push_back(CameraAnimation(PAUSE, 3, 0, 0, 0));
	//animations.push_back(row);
	//row.clear();
	row.push_back(CameraAnimation(UP, 1, 0.0, 1.0, 0.0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(PAUSE, 1.5, 0, 0, 0));
	animations.push_back(row);
	row.clear();
	//row.push_back(CameraAnimation(PAUSE, 3, 0, 0, 0));
	//animations.push_back(row);
	//row.clear();
	row.push_back(CameraAnimation(LOOK, 1.5, 0.01, 1.57, 0)); //1.5
	row.push_back(CameraAnimation(POSITION, 1.5, camera_x, 40, camera_z + 5)); //1.5
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(PAUSE, 1.2, 0, 0, 0));
	animations.push_back(row);
	row.clear();

	/*row.push_back(CameraAnimation(LOOK, 0.3, 0.06, 1.57 + (0.0003 * 60 * 1.2), 0)); //1.5
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 0.6, -0.06, 1.57 + (0.0003 * 60 * 1.2), 0)); //1.5
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 0.3, 0.01, 1.57 + (0.0003 * 60 * 1.2), 0)); //1.5
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(PAUSE, 1.0, 0, 0, 0));
	animations.push_back(row);
	row.clear();*/

	/*row.push_back(CameraAnimation(LOOK, 0.3, -5, 6, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 0.3, 1, 2, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 0.3, 7, 5, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 0.3, -5, 6, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 0.3, -4, -1, 0));
	animations.push_back(row);
	row.clear();*/

	row.push_back(CameraAnimation(LOOK, 1.5, .4, 1.7, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(PAUSE, 0.7, 0, 0, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 1.5, -0.15, 1.9, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(PAUSE, 1.0, 0, 0, 0));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 1.0, 0.01, 2, 0));
	row.push_back(CameraAnimation(POSITION, 1.0, camera_x, 20, camera_z));
	animations.push_back(row);
	row.clear();
	row.push_back(CameraAnimation(LOOK, 1.5, 0.01, 1.57, 0));
	row.push_back(CameraAnimation(POSITION, 1.5, INITIAL_WALKING_X, INITIAL_WALKING_Y, INITIAL_WALKING_Z));
	animations.push_back(row);
}

void Camera::update_step_threshold(float distance)
{
	old_threshold = step_threshold;
	float percent = distance / WALK_SPEED;
	float steps_per_second = (percent * 3.5 + 0.5) / (10.0 / WALK_SPEED);
	step_threshold = (60.0 / steps_per_second) * distance;
}

void Camera::expand_planes()
{
	vector<Plane> new_planes;
	for (int i = 0; i < planes.size(); ++i)
	{
		Vector3 short_norm = planes[i].normal.change_length(planes[i].expansion == -1 ? MIN_DISTANCE_FROM_WALL : planes[i].expansion);
		Vector3 edge1 = planes[i].p2 - planes[i].p1;
		Vector3 edge2 = planes[i].p3 - planes[i].p1;
		new_planes.push_back(Plane(planes[i].p1 + short_norm, planes[i].p2 + short_norm, planes[i].p3 + short_norm, planes[i].p4 + short_norm, planes[i].normal, "", false));
		new_planes.push_back(Plane(planes[i].p1 - short_norm, planes[i].p2 - short_norm, planes[i].p3 - short_norm, planes[i].p4 - short_norm, planes[i].normal * -1, "", false));
		new_planes.push_back(Plane(planes[i].p1 - short_norm, planes[i].p2 - short_norm, planes[i].p1 + short_norm, planes[i].p2 + short_norm, edge2 * -1));
		new_planes.push_back(Plane(planes[i].p3 - short_norm, planes[i].p4 - short_norm, planes[i].p3 + short_norm, planes[i].p4 + short_norm, edge2));
		new_planes.push_back(Plane(planes[i].p3 - short_norm, planes[i].p1 - short_norm, planes[i].p3 + short_norm, planes[i].p1 + short_norm, edge1 * -1));
		new_planes.push_back(Plane(planes[i].p4 - short_norm, planes[i].p2 - short_norm, planes[i].p4 + short_norm, planes[i].p2 + short_norm, edge1));
	}

	draw_planes = planes;
	planes = new_planes;
}

void Camera::mouse_stopped()
{
	had_mouse_motion = false;
}

void Camera::window_resized(int new_width, int new_height)
{
	aspect = (float)new_width / (float)new_height;
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
	if (do_file_animation)
	{
		int frame = file_animation.frame;
		auto position = file_animation.position;
		auto at = file_animation.at;
		auto up = file_animation.up;
		gluLookAt(position[frame].x, position[frame].y, position[frame].z, at[frame].x, at[frame].y, at[frame].z, up[frame].x, up[frame].y, up[frame].z);
		if (current_file == "anim2")
		{
			if (frame == 20)
				sound_manager->play_sound("rustle1.wav");

			else if (frame == 592)
				sound_manager->play_sound("rustle2.wav");

			else if (frame == 1420)
				sound_manager->play_sound("rustle2.wav");
		}

		return;
	}

	Vector3 look_point = get_look_point();
	gluLookAt(camera_x, camera_y, camera_z, look_point.x, look_point.y, look_point.z, look_x, look_y, look_z);
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

bool Camera::move(MovementKey move_key)
{
	bool retval = false;
	Vector3 look_point = get_look_point();
	float look_x = look_point.x;
	float look_y = look_point.y;
	float look_z = look_point.z;
	float slope = (float)(look_z - camera_z) / (float)(look_x - camera_x);
	float velocity_x;
	float velocity_z;
	float velocity_y = 0.0;
	float current_velocity = move_key.velocity;
	if (move_key.key == 'w' || move_key.key == 's' || move_key.key == 'W' || move_key.key == 'S') //TODO Scancodes instead of chars?
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

	else if (move_key.key == 'a' || move_key.key == 'd' || move_key.key == 'A' || move_key.key == 'D')
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

	else return true;

	if (move_key.key == 's' || move_key.key == 'a' || move_key.key == 'S' || move_key.key == 'A')
	{
		velocity_x *= -1.0;
		velocity_z *= -1.0;
	}

	int old_x = camera_x;
	int old_z = camera_z;
	//if (camera_x + velocity_x < WALL_DISTANCE - MIN_DISTANCE_FROM_WALL &&
	//	camera_x + velocity_x > -WALL_DISTANCE + MIN_DISTANCE_FROM_WALL || flying_mode)
	//{
	retval = true;
	if (!rotating_quad)
		//camera_x += velocity_x;
		delta_camera_x += velocity_x;

	else
	{
		rotation_angle += velocity_x / 5.0;
		if (rotation_angle >= 90)
			rotating_quad = false;

		if (rotation_angle <= 0)
		{
			rotation_angle = 0;
			return_from_flying = true;
		}
	}
	//}

	//if (camera_z + velocity_z < WALL_DISTANCE - MIN_DISTANCE_FROM_WALL &&
	//	camera_z + velocity_z > -WALL_DISTANCE + MIN_DISTANCE_FROM_WALL || flying_mode)
	//{
	retval = true;
	if (!rotating_quad)
		//camera_z += velocity_z;
		delta_camera_z += velocity_z;
	//}

	if (flying_mode && (move_key.key == 'w' || move_key.key == 's' || move_key.key == 'W' || move_key.key == 'S'))
	{
		float dist = sqrt(pow(delta_camera_x, 2) + pow(delta_camera_z, 2));
		float slope = tan(current_vert_angle - 1.57);
		velocity_y = -slope * dist;
		if (velocity_y > 10)
			velocity_y = 10;

		if (velocity_y < -10)
			velocity_y = -10;

		if (move_key.key == 's' || move_key.key == 'S')
			velocity_y *= -1;
	}

	//TODO Only when computer is visible!
	/*if (do_collision && computer_present)
	{
	bool collision = false;
	if (computer_x == 300 && computer_z == 0)
	collision = 270 < camera_x && camera_x < 330 && -40 < camera_z && camera_z < 40;

	if (computer_x == -300 && computer_z == 0)
	collision = -270 > camera_x && camera_x > -330 && -40 < camera_z && camera_z < 40;

	if (computer_x == 0 && computer_z == 300)
	collision = -40 < camera_x && camera_x < 40 && 270 < camera_z && camera_z < 300;

	if (computer_x == 0 && computer_z == -300)
	collision = -40 < camera_x && camera_x < 40 && -270 > camera_z && camera_z > -300;

	if (collision)
	{
	camera_x = old_x;
	camera_z = old_z;
	if (camera_x <= 270 && camera_x >= -270 && camera_z <= 270 && camera_z >= -270)
	{
	positioning = true;
	keys_pressed.clear();
	}
	}
	}*/

	delta_camera_y += velocity_y;
	//alListener3f(AL_VELOCITY, velocity_x, velocity_y, velocity_z); //Velocity never hits 0 even when stationary?
	//sound_manager->change_source_velocity("step.wav", velocity_x, velocity_y, velocity_z);
	if (sqrt(pow(camera_x - step_ref_x, 2) + pow(camera_z - step_ref_z, 2)) > step_threshold && /*step_threshold >= old_threshold &&*/ step_threshold != 0 && (frames - frame_last_step) >= 15)
	{
		Vector3 look_point = get_look_point();
		ALfloat orientation[] = { look_point.x, look_point.y, look_point.z, look_x, look_y, look_z };
		sound_manager->play_sound("step.wav"/*, 80, camera_x, 0, camera_z, camera_x, camera_y, camera_z, orientation*/);
		step_ref_x = camera_x;
		step_ref_z = camera_z;
		frame_last_step = frames;
	}

	if (camera_x < -300 && camera_z < -300)
		went_to_corner = true;

	/*if (flying_mode && camera_x >= -10 && camera_x <= 10 && camera_z >= -10 && camera_z <= 10)
	{
	return_from_flying = true;
	flying_mode = false;
	go_to_breaking = true;
	}*/

	return retval;
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
		glutWarpPointer(100, 100);
		return;
	}

	current_horiz_angle += (new_x - 100) * -LOOK_VELOCITY;
	current_vert_angle += (new_y - 100) * LOOK_VELOCITY;
	if (current_horiz_angle < 0)
		current_horiz_angle = (2 * PI) + current_horiz_angle;

	if (current_horiz_angle > (2 * PI))
		current_horiz_angle -= (2 * PI);

	if (current_vert_angle > VERT_UPPER_BOUND)
		current_vert_angle = VERT_UPPER_BOUND;

	if (current_vert_angle < VERT_LOWER_BOUND)
		current_vert_angle = VERT_LOWER_BOUND;

	if (current_horiz_angle >= PI)
		looked_behind = true;

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

	if (key == 'm')
	{
		current_vert_angle = 2.0;
		current_horiz_angle = 4.63;
		camera_y = 20;
		camera_x = -300;
		camera_z = 481;
		return;
	}
	
	if (positioning)
		return;

	if (key == 'A')
		key = 'a';

	if (key == 'S')
		key = 's';

	if (key == 'D')
		key = 'd';

	if (key == 'W')
		key = 'w';

	for (int i = 0; i < keys_pressed.size(); ++i)
	{
		if (keys_pressed[i].key == key)
			return;
	}

	keys_pressed.push_back(MovementKey(key, true, WALK_ACCELERATION));
}

void Camera::release_key(unsigned char key)
{
	if (key == 'A')
		key = 'a';

	if (key == 'S')
		key = 's';

	if (key == 'D')
		key = 'd';

	if (key == 'W')
		key = 'w';

	for (int i = 0; i < keys_pressed.size(); ++i)
	{
		if (keys_pressed[i].key == key)
			keys_pressed[i].active = false;
	}
}

void Camera::flying_movement()
{
	float lowest_d = -1;
	Plane plane;
	int plane_index = 0;
	for (int i = 0; i < planes.size(); ++i)
	{
		float bottom = Vector3(delta_camera_x, delta_camera_y, delta_camera_z).dot(planes[i].normal);
		if (bottom != 0)
		{
			float top = (planes[i].p1 - Vector3(camera_x, camera_y, camera_z)).dot(planes[i].normal);
			float d = top / bottom;
			if (0 <= d && d <= 1 && planes[i].point_in_bounds(Vector3(camera_x, camera_y, camera_z) + (Vector3(delta_camera_x, delta_camera_y, delta_camera_z) * d)))
			{
				if (d != 0 || !planes[i].same_side(Vector3(camera_x + delta_camera_x, camera_y + delta_camera_y, camera_z + delta_camera_z), planes[i].p1 + planes[i].normal))
				{
					if ((lowest_d == -1 || d < lowest_d) && find(prev_planes.begin(), prev_planes.end(), &(planes[i])) == prev_planes.end())
					{
						lowest_d = d;
						plane = planes[i];
						plane_index = i;
					}
				}
			}

			else
			{
				int q = 1;
			}
		}
	}

	if (lowest_d == -1)
	{
		camera_x += delta_camera_x;
		camera_y += delta_camera_y;
		camera_z += delta_camera_z;
	}

	else
	{
		prev_planes.push_back(&(planes[plane_index]));
		float top = (plane.p1 - Vector3(camera_x + delta_camera_x, camera_y + delta_camera_y, camera_z + delta_camera_z)).dot(plane.normal);
		float bottom = plane.normal.dot(plane.normal);
		float d = top / bottom;
		Vector3 to_plane = plane.normal * d;
		delta_camera_x += to_plane.x;
		delta_camera_y += to_plane.y;
		delta_camera_z += to_plane.z;
		flying_movement();
		float t = (plane.p1 - Vector3(camera_x, camera_y, camera_z)).dot(plane.normal);
		if (t != 0) //t will be a very small rounding error
		{
			camera_x += 2 * abs(t) * plane.normal.x;
			camera_y += 2 * abs(t) * plane.normal.y;
			camera_z += 2 * abs(t) * plane.normal.z;
		}
	}
}

void Camera::animate()
{
	frames++;
	if (do_file_animation)
	{
		if (!file_animation.next())
		{
			do_file_animation = false;
			camera_x = file_animation.position.back().x;
			camera_y = file_animation.position.back().y;
			camera_z = file_animation.position.back().z;
			current_horiz_angle = PI;
			current_vert_angle = PI / 2.0;
			look_x = 0.0;
			look_y = 1.0;
			look_z = 0.0;
		}

		return;
	}

	prev_planes.clear();
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
				looked_behind = false;
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

		++it;
	}

	float x_component = 0.0;
	float z_component = 0.0;
	for (int i = 0; i < keys_pressed.size(); ++i)
	{
		if (keys_pressed[i].key == 'a' || keys_pressed[i].key == 'A')
			x_component -= keys_pressed[i].velocity;

		else if (keys_pressed[i].key == 'd' || keys_pressed[i].key == 'D')
			x_component += keys_pressed[i].velocity;

		else if (keys_pressed[i].key == 's' || keys_pressed[i].key == 'S')
			z_component -= keys_pressed[i].velocity;

		else if (keys_pressed[i].key == 'w' || keys_pressed[i].key == 'W')
			z_component += keys_pressed[i].velocity;
	}

	float distance_to_travel = sqrt(pow(x_component, 2) + pow(z_component, 2));
	vector<float> old_velocities;
	if (distance_to_travel > WALK_SPEED)
	{
		float ratio = WALK_SPEED / distance_to_travel;
		for (int i = 0; i < keys_pressed.size(); ++i)
		{
			old_velocities.push_back(keys_pressed[i].velocity);
			keys_pressed[i].velocity *= ratio;
		}
	}

	float prev_camera_x = camera_x;
	float prev_camera_z = camera_z;
	for (vector<MovementKey>::iterator it = keys_pressed.begin(); it != keys_pressed.end(); ++it)
	{
		move(*it);
		if (positioning)
			break;
	}

	/*if (camera_x + delta_camera_x < WALL_DISTANCE - MIN_DISTANCE_FROM_WALL &&
	camera_x + delta_camera_x > -WALL_DISTANCE + MIN_DISTANCE_FROM_WALL)
	camera_x += delta_camera_x;

	if (camera_z + delta_camera_z < WALL_DISTANCE - MIN_DISTANCE_FROM_WALL &&
	camera_z + delta_camera_z > -WALL_DISTANCE + MIN_DISTANCE_FROM_WALL)
	camera_z += delta_camera_z;*/

	float old_x = camera_x;
	float old_z = camera_z;
	if (true)
	{
		flying_movement();
		if (goal_x - 300 <= camera_x && camera_x <= goal_x && goal_z <= camera_z && camera_z <= goal_z + 300 && 0 <= camera_y && camera_y <= 300)
		{
			int q = 1;
		}
	}

	else
	{
		if (camera_x + delta_camera_x > WALL_DISTANCE - MIN_DISTANCE_FROM_WALL && !flying_mode)
			camera_x = WALL_DISTANCE - MIN_DISTANCE_FROM_WALL;

		else if (camera_x + delta_camera_x < -WALL_DISTANCE + MIN_DISTANCE_FROM_WALL && !flying_mode)
			camera_x = -WALL_DISTANCE + MIN_DISTANCE_FROM_WALL;

		else camera_x += delta_camera_x;

		if (camera_z + delta_camera_z > WALL_DISTANCE - MIN_DISTANCE_FROM_WALL && !flying_mode)
			camera_z = WALL_DISTANCE - MIN_DISTANCE_FROM_WALL;

		else if (camera_z + delta_camera_z < -WALL_DISTANCE + MIN_DISTANCE_FROM_WALL && !flying_mode)
			camera_z = -WALL_DISTANCE + MIN_DISTANCE_FROM_WALL;

		else camera_z += delta_camera_z;
	}

	delta_camera_x = 0.0;
	delta_camera_z = 0.0;
	delta_camera_y = 0.0;
	update_step_threshold(sqrt(pow(camera_x - old_x, 2) + pow(camera_z - old_z, 2)));

	for (int i = 0; i < old_velocities.size(); ++i)
	{
		keys_pressed[i].velocity = old_velocities[i];
	}

	if (do_collision && computer_present && !flying_mode)
	{
		bool collision = false;
		if (computer_x == 300 && computer_z == 0)
			collision = 270 < camera_x && camera_x < 330 && -40 < camera_z && camera_z < 40;

		if (computer_x == -300 && computer_z == 0)
			collision = -270 > camera_x && camera_x > -330 && -40 < camera_z && camera_z < 40;

		if (computer_x == 0 && computer_z == 300)
			collision = -40 < camera_x && camera_x < 40 && 270 < camera_z && camera_z < 300;

		if (computer_x == 0 && computer_z == -300)
			collision = -40 < camera_x && camera_x < 40 && -270 > camera_z && camera_z > -300;

		if (collision)
		{
			if (old_x <= 270 && old_x >= -270 && old_z <= 270 && old_z >= -270)
			{
				positioning = true;
				keys_pressed.clear();
			}
		}
	}
}

void Camera::wall_planes()
{
	auto backup_planes = planes;
	planes.clear();
	planes.push_back(Plane(Vector3(WALL_DISTANCE, 0, WALL_DISTANCE), Vector3(WALL_DISTANCE, 0, -WALL_DISTANCE), Vector3(WALL_DISTANCE, WALL_HEIGHT, WALL_DISTANCE), Vector3(WALL_DISTANCE, WALL_HEIGHT, -WALL_DISTANCE), Vector3(-1, 0, 0), "invisible.png"));
	planes.push_back(Plane(Vector3(-WALL_DISTANCE, 0, WALL_DISTANCE), Vector3(-WALL_DISTANCE, 0, -WALL_DISTANCE), Vector3(-WALL_DISTANCE, WALL_HEIGHT, WALL_DISTANCE), Vector3(-WALL_DISTANCE, WALL_HEIGHT, -WALL_DISTANCE), Vector3(1, 0, 0), "invisible.png"));
	planes.push_back(Plane(Vector3(WALL_DISTANCE, 0, WALL_DISTANCE), Vector3(-WALL_DISTANCE, 0, WALL_DISTANCE), Vector3(WALL_DISTANCE, WALL_HEIGHT, WALL_DISTANCE), Vector3(-WALL_DISTANCE, WALL_HEIGHT, WALL_DISTANCE), Vector3(0, 0, -1), "invisible.png"));
	planes.push_back(Plane(Vector3(WALL_DISTANCE, 0, -WALL_DISTANCE), Vector3(-WALL_DISTANCE, 0, -WALL_DISTANCE), Vector3(WALL_DISTANCE, WALL_HEIGHT, -WALL_DISTANCE), Vector3(-WALL_DISTANCE, WALL_HEIGHT, -WALL_DISTANCE), Vector3(0, 0, 1), "invisible.png"));
	expand_planes();
	planes.insert(planes.end(), backup_planes.begin(), backup_planes.end());
}

void Camera::computer_planes()
{
	auto backup_planes = planes;
	planes.clear();
	planes.push_back(Plane(Vector3(330, 0, -40), Vector3(330, 0, 40), Vector3(330, 100, -40), Vector3(330, 100, 40), Vector3(1, 0, 0), "red.png", true, 2));
	planes.push_back(Plane(Vector3(270, 0, -40), Vector3(330, 0, -40), Vector3(270, 100, -40), Vector3(330, 100, -40), Vector3(0, 0, -1), "blue.png", true, 2));
	planes.push_back(Plane(Vector3(270, 0, 40), Vector3(330, 0, 40), Vector3(270, 100, 40), Vector3(330, 100, 40), Vector3(0, 0, 1), "black.png", true, 2));
	expand_planes();
	planes.insert(planes.end(), backup_planes.begin(), backup_planes.end());
}

void Camera::load_maze(string name)
{
	int room_size = 300;
	vector<string> lines;
	string line;
	ifstream file_stream(name);
	while (getline(file_stream, line))
		lines.push_back(line);

	int current_x = 0;
	int current_z = 0;
	for (int i = lines.size() - 1; i >= 0; i--)
	{
		for (int j = 0; j < lines[i].size(); ++j)
		{
			if (lines[i][j] == '1' || lines[i][j] == '2' || lines[i][j] == '3')
			{
				planes.push_back(Plane(Vector3(current_x, 0, current_z), Vector3(current_x - room_size, 0, current_z), Vector3(current_x, 0, current_z + room_size), Vector3(current_x - room_size, 0, current_z + room_size), Vector3(0, 1, 0)));
				planes.push_back(Plane(Vector3(current_x, room_size, current_z), Vector3(current_x - room_size, room_size, current_z), Vector3(current_x, room_size, current_z + room_size), Vector3(current_x - room_size, room_size, current_z + room_size), Vector3(0, 1, 0)));
				if (j == 0 || lines[i][j - 1] == '0')
					planes.push_back(Plane(Vector3(current_x, 0, current_z), Vector3(current_x, 0, current_z + room_size), Vector3(current_x, room_size, current_z), Vector3(current_x, room_size, current_z + room_size), Vector3(1, 0, 0), "yellow.png"));

				if (j == lines[i].size() - 1 || lines[i][j + 1] == '0')
					planes.push_back(Plane(Vector3(current_x - room_size, 0, current_z), Vector3(current_x - room_size, 0, current_z + room_size), Vector3(current_x - room_size, room_size, current_z), Vector3(current_x - room_size, room_size, current_z + room_size), Vector3(1, 0, 0), "yellow.png"));

				if (i == 0 || lines[i - 1][j] == '0')
					planes.push_back(Plane(Vector3(current_x, 0, current_z + room_size), Vector3(current_x - room_size, 0, current_z + room_size), Vector3(current_x, room_size, current_z + room_size), Vector3(current_x - room_size, room_size, current_z + room_size), Vector3(0, 0, 1), "red.png"));

				if (lines[i][j] != '2' && (i == lines.size() - 1 || lines[i + 1][j] == '0'))
					planes.push_back(Plane(Vector3(current_x, 0, current_z), Vector3(current_x - room_size, 0, current_z), Vector3(current_x, room_size, current_z), Vector3(current_x - room_size, room_size, current_z), Vector3(0, 0, 1), "red.png"));

				if (lines[i][j] == '3')
				{
					goal_x = current_x;
					goal_y = 0;
					goal_z = current_z;
				}
			}

			current_x -= room_size;
		}

		current_z += room_size;
		current_x = 0;
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
		if (camera_x < 0.0 && 3.92 < current_horiz_angle && current_horiz_angle < 5.49)
		{
			computer_counter++;
			computer_flag = true;
		}
	}

	else
	{
		if (!(camera_x < 0.0) && !(3.92 < current_horiz_angle && current_horiz_angle < 5.49))
			computer_flag = false;
	}

	if (computer_counter >= NUM_LOOKS)
	{
		do_collision = true;
		computer_present = true;
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
		if (current_anim_index == 0)
			sound_manager->play_sound("rustle1.wav");

		if (current_anim_index == animations.size() - 2)
			sound_manager->play_sound("rustle2.wav");

		if (current_anim_index == 7)
			sound_manager->play_sound("rustle2.wav");

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

			else if (current_row[i].type == PAUSE)
			{
				pausex = ExpandableImage(0, 100, 0, 100, "", NONE, 100, current_row[i].duration, true);
				pausex.toggle();
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

	bool anim = xex.animating || yex.animating || zex.animating || horizex.animating || vertex.animating || pausex.animating;
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

	if (pausex.animating)
	{
		pausex.animate();
		current_horiz_angle += 0.0003;
		current_vert_angle += 0.0003;
	}

	if (!anim)
	{
		current_row.clear();
		current_anim_index++;
	}

	return true;
}

void Camera::start_file_animation(string file_name)
{
	do_file_animation = true;
	file_animation = CameraFileAnimation(file_name);
	current_file = file_name;
}