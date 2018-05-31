#include <fstream>
#include "MMO.h"
#include "ObjModel.h"
#include "BubbleGame.h"
#include "HelpCenter.h"
#include "XButton.h"
#include "InfoText.h"
#include "Canvas.h"
#include "keymappings.h"

bool inside(ScreenElement s1, ScreenElement s2)
{
	return s1.x1 >= s2.x1 && s1.x2 <= s2.x2 && s1.y1 >= s2.y1 && s1.y2 <= s2.y2;
}

//TODO probably doesn't work in-browser right now
MMO::MMO(float _x1, float _y1, float _x2, float _y2, string _name, string initial_level, Computer* _parent, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent), ladder_box(ScreenElement(0, 0, 0, 0, ""))
{
	walk_speed = 5;
	warped = false;
	chat_height = 150;
	//player = ScreenElement(950.0, 530.0, 950 + 114.0, 530.0 + 133.0, "frame1.png");
	player = Character(0, 0, 14, 22, "main");
	animated_components.push_back(ScreenElement(0.0, 100.0, 150.0, 150.0, "river1.png"));
	animated_components.push_back(ScreenElement(100.0, 200.0, 300.0, 300.0, "river2.png"));
	animated_components.push_back(ScreenElement(1000.0, 150.0, 1150.0, 200.0, "river1.png"));
	animated_components.push_back(ScreenElement(1500.0, 300.0, 1700.0, 400.0, "river2.png"));
	animated_components.push_back(ScreenElement(-200.0, 200.0, -50.0, 250.0, "river1.png"));
	if (current_level == "mmo.txt")
		no_movement = true;

	else no_movement = false;
	zoom_animation = false;

	//auto help_center = make_unique<HelpCenter>(0.0, 56.0, 1900.0, 200.0, "white.png", parent, application);
	//parent->to_be_added.insert(parent->to_be_added.end() - 2, std::move(help_center)); we insert MMO at top of list after it's constructed

	app_state = 1; //TODO switch back to 0 for password screen
	current_level = initial_level;
	load_level(initial_level);
}

void MMO::load_level(string file_name)
{
	int start_x, start_y;
	background_images.clear();
	bounding_boxes.clear();
	warp_boxes.clear();
	scrolling_messages.clear();
	ifstream file_stream(file_name);
	string line;
	bool has_sp = false;
	current_level = file_name;
	while (getline(file_stream, line))
	{
		vector<string> components = split(line, ' ');
		if (components[0] == "bg")
			background_images.push_back(ScreenElement(atof(components[1].c_str()), atof(components[2].c_str()), atof(components[3].c_str()), atof(components[4].c_str()), components[5]));

		else if (components[0] == "sp")
		{
			has_sp = true;
			start_x = atoi(components[1].c_str());
			start_y = atoi(components[2].c_str());
		}

		else if (components[0] == "warp")
			warp_boxes.push_back(ScreenElement(atof(components[1].c_str()), atof(components[2].c_str()), atof(components[3].c_str()), atof(components[4].c_str()), components[5]));

		else if (components[0] == "ladder")
			bounding_boxes.push_back(ScreenElement(atof(components[1].c_str()), atof(components[2].c_str()), atof(components[3].c_str()), atof(components[4].c_str()), "ladder"));

		else
			bounding_boxes.push_back(ScreenElement(atof(components[1].c_str()), atof(components[2].c_str()), atof(components[3].c_str()), atof(components[4].c_str()), ""));
	}

	if (file_name == "testboxes.txt")
		load_additional_boxes("testboxes.png");

	if (file_name == "underground.txt")
		load_additional_boxes("undergroundadd.png");

	background_images[0].translate(0, chat_height);
	for (int i = 0; i < warp_boxes.size(); ++i)
		warp_boxes[i].translate(0, chat_height);

	for (int i = 0; i < bounding_boxes.size(); ++i)
		bounding_boxes[i].translate(0, chat_height);

	left_bottom = &(background_images[0]);
	right_top = &(background_images[background_images.size() - 1]);
	if (has_sp)
	{
		moving_stage_x = false;
		moving_stage_y = false;
		ignore_collision = true;
		player.translate(0 - player.x1, 0 - player.y1);
		for (int i = 0; i < start_x / 15; ++i)
			process_key('d');

		for (int i = 0; i < start_y / 15; ++i)
			process_key('w');

		ignore_collision = false;
	}
}

void MMO::load_additional_boxes(string texture_name)
{
	GLubyte* pixels = parent->texture_manager->get_pixel_data(texture_name);
	int width = parent->texture_manager->get_width(texture_name);
	int height = parent->texture_manager->get_height(texture_name);
	Canvas canvas = Canvas(0, 0, width, height, "", NULL, NONE);
	canvas.image = pixels;
	ScreenElement current_box = ScreenElement(-1, -1, -1, -1, "");
	vector<ScreenElement> candidate_boxes;
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			if (i == 4365 && j == 3355)
			{
				int q = 1;
			}

			if (i == 4365 && j == 3354)
			{
				int q = 1;
			}

			int step1 = j * (canvas.x2 - canvas.x1) * 4;
			int step2 = i * 4;
			int start = step1 + step2;
			GLubyte colorx = canvas.image[start];
			GLubyte colory = canvas.image[start + 1];
			GLubyte colorz = canvas.image[start + 2];
			if (colorx == 0 && colory == 0 && colorz == 255)
			{
				if (current_box.x1 == -1)
				{
					current_box.x1 = i;
					current_box.y1 = j;
					current_box.x2 = i + 1;
					current_box.y2 = j + 1;
				}

				else
					current_box.x2++;
			}

			else
			{
				if (current_box.x1 != -1)
				{
					candidate_boxes.push_back(current_box);
					current_box.x1 = current_box.x2 = current_box.y1 = current_box.y2 = -1;
				}
			}
		}

		if (current_box.x1 != -1)
		{
			candidate_boxes.push_back(current_box);
			current_box.x1 = current_box.x2 = current_box.y1 = current_box.y2 = -1;
		}
	}

	vector<ScreenElement> final_boxes;
	for (int i = 0; i < candidate_boxes.size(); ++i)
	{
		bool extended_box = false;
		for (int j = 0; j < final_boxes.size(); ++j)
		{
			auto box = candidate_boxes[i];
			auto glob = final_boxes[j];
			if (box.x1 == glob.x1 && box.x2 == glob.x2 && box.y1 == glob.y2)
			{
				final_boxes[j].y2++;
				extended_box = true;
			}
		}

		if (!extended_box)
			final_boxes.push_back(candidate_boxes[i]);
	}

	bounding_boxes.insert(bounding_boxes.end(), final_boxes.begin(), final_boxes.end());
}

//TODO make sure you can't click on other stuff
void MMO::mouse_moved(int x, int y)
{
	if (current_level == "mmo.txt" || current_level == "underground.txt")
	{
		if (warped)
			warped = false;

		else
		{
			warped = true;
			//glutWarpPointer(player.x1 + 10, glutGet(GLUT_WINDOW_HEIGHT) - (player.y1 + 20));
			glutWarpPointer(500, 500);
		}
	}
}

void MMO::draw(TextureManager* texture_manager)
{
	if (app_state == 0)
		return;

	/*glEnable(GL_STENCIL_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glStencilFunc(GL_NEVER, 1, 0xFF);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
	glDisable(GL_TEXTURE_2D);
	glStencilMask(0xFF);
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glRectf(0.0, scaley(56.0), scalex(1920.0), scaley(y2));
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0x00);
	glStencilFunc(GL_EQUAL, 1, 0xFF);*/
	glPushMatrix();
	parent->set_cursor("invisible.png");
	parent->someone_set_cursor = true;
	if (zoom_animation) //draw zoom
	{
		float tx1 = glutGet(GLUT_WINDOW_WIDTH) / 2 - (ladder_box.x1 + (ladder_box.x2 - ladder_box.x1) / 2.0);
		float ty1 = glutGet(GLUT_WINDOW_HEIGHT) / 2 - (ladder_box.y1 + (ladder_box.y2 - ladder_box.y1) / 2.0);
		if (player_to_center_x != 0)
			player.translate(copysign(2, player_to_center_x), 0);

		if (player_to_center_y != 0)
			player.translate(0, copysign(2, player_to_center_y));

		if (percent_of_centering != 1.0)
		{
			background_images[0].translate(tx1 * 0.02, ty1 * 0.02);
			player.translate(tx1 * 0.02, ty1 * 0.02);
		}

		glScalef(scale_x, scale_y, 1.0);
		glTranslatef(-960.0 * (scale_x - 1) / scale_x, -540.0 * (scale_y - 1) / scale_y, 0.0);
	}

	ScreenElement::draw(texture_manager);
	for (int i = 0; i < background_images.size(); ++i)
	{
		background_images[i].draw(texture_manager);
		if (current_level != "mmo.txt" && current_level != "testboxes.txt" && current_level != "underground.txt")
		{
			string old_name = background_images[i].name;
			background_images[i].name = real_split(old_name, '.')[0] + "overlay.png";
			background_images[i].draw(texture_manager);
			background_images[i].name = old_name;
		}
	}

	/*ScreenElement river = ScreenElement(0.0, 56.0, 1920.0, 616.0, "river.png");
	river.draw(texture_manager);*/
	/*for (int i = 0; i < animated_components.size(); ++i)
		animated_components[i].draw(texture_manager);*/
	if (current_level == "mmo.txt")
	{
		for (int i = 0; i < scrolling_messages.size(); ++i)
			scrolling_messages[i].draw(texture_manager);
	}

	glPopMatrix();

	//if (current_level != "mmo.txt" && current_level != "underground.txt")
	string old_name = player.name;
	player.name = "cursor.png";
	player.draw(texture_manager);
	player.name = old_name;
	
	ScreenElement loading_screen = ScreenElement(0.0, 0.0, scalex(1920.0), scaley(1080.0), "black.png");
	glColor4f(1.0, 1.0, 1.0, warping_alpha);
	loading_screen.draw(texture_manager);
	glColor4f(1.0, 1.0, 1.0, 1.0);

	glDisable(GL_TEXTURE_2D);
	glColor4f(1.0, 0.0, 0.0, 1.0);
	/*for (int i = 0; i < bounding_boxes.size(); ++i)
	{
		glRectf(bounding_boxes[i].x1, bounding_boxes[i].y1, bounding_boxes[i].x2, bounding_boxes[i].y2);
		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		draw_string(texture_manager, 32, "bb" + to_string(i), bounding_boxes[i].x1 + 10, bounding_boxes[i].y1 + 10);
		glDisable(GL_TEXTURE_2D);
		glColor4f(1.0, 0.0, 0.0, 1.0);
	}*/

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	//glDisable(GL_STENCIL_TEST);

	/*glBegin(GL_LINES);
		glColor4f(0.0, 0.0, 0.0, 1.0);
		glVertex2f(glutGet(GLUT_WINDOW_WIDTH) / 2.0, 0.0);
		glVertex2f(glutGet(GLUT_WINDOW_WIDTH) / 2.0, glutGet(GLUT_WINDOW_HEIGHT));
		glVertex2f(0.0, glutGet(GLUT_WINDOW_HEIGHT) / 2.0);
		glVertex2f(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) / 2.0);
		glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnd();*/
}

void MMO::translate_stage(float delta_x, float delta_y)
{
	for (int i = 0; i < background_images.size(); ++i)
		background_images[i].translate(delta_x, delta_y);

	for (int i = 0; i < bounding_boxes.size(); ++i)
		bounding_boxes[i].translate(delta_x, delta_y);

	for (int i = 0; i < warp_boxes.size(); ++i)
		warp_boxes[i].translate(delta_x, delta_y);

	for (int i = 0; i < scrolling_messages.size(); ++i)
		scrolling_messages[i].translate(delta_x, delta_y);
}

void MMO::press_key(unsigned char key)
{
	if (key == 'k')
	{
		for (int i = 0; i < 500; ++i)
			load_level("underground.txt");

		return;
	}

	if (key == 'q')
	{
		parent->close_application(MMO_GAME);
		return;
	}

	keys_pressed[key] = true;
}

void MMO::release_key(unsigned char key)
{
	keys_pressed[key] = false;
}

//TODO bottom of map is cut off
//TODO underground.txt, get stuck on bounding box off by one, 950 and 951
//TODO clicking during zooming does weird stuff, disable sword there
void MMO::process_key(unsigned char key)
{
	if (app_state == 0)
		return;
	
	if (no_movement)
		return;
	
	bool should_swap_x = !moving_stage_x;
	bool should_swap_y = !moving_stage_y;
	float delta_x = 0.0;
	float delta_y = 0.0;
	float player_delta_x = 0.0;
	float player_delta_y = 0.0;
	string direction;
	switch (key)
	{
	case 'w':
		delta_y = round(-walk_speed);
		direction = "up";
		break;

	case 's':
		delta_y = round(walk_speed);
		direction = "down";
		break;

	case 'a':
		delta_x = round(walk_speed);
		direction = "left";
		break;

	case 'd':
		delta_x = round(-walk_speed);
		direction = "right";
		break;
	}

	if (animation_counter % 5 == 0)
		player.step(direction);

	if (moving_stage_x)
	{
		if (left_bottom->x1 + delta_x > 0.0)
		{
			player_delta_x = delta_x + left_bottom->x1;
			delta_x = -left_bottom->x1;
			moving_stage_x = false;
		}

		if (right_top->x2 + delta_x < scalex(1920.0))
		{
			player_delta_x = delta_x + right_top->x2 - scalex(1920.0);
			delta_x = -(right_top->x2 - scalex(1920.0));
			moving_stage_x = false;
		}
	}

	if (moving_stage_y)
	{
		if (left_bottom->y1 + delta_y > chat_height)
		{
			player_delta_y = delta_y + left_bottom->y1 - chat_height;
			delta_y = -(left_bottom->y1 - chat_height);
			moving_stage_y = false;
		}

		if (right_top->y2 + delta_y < scaley(1080.0))
		{
			player_delta_y = delta_y + right_top->y2 - scaley(1080.0);
			delta_y = -(right_top->y2 - scaley(1080.0));
			moving_stage_y = false;
		}
	}

	if (!moving_stage_x) //TODO need to do scaling
	{
		if (player.x1 > 950.0 && player.x1 - delta_x <= 950.0)
		{
			delta_x -= player.x1 - 950.0;
			player_delta_x = player.x1 - 950.0;
			moving_stage_x = true;
		}

		else if (player.x1 < 950.0 && player.x1 - delta_x >= 950.0)
		{
			delta_x += 950.0 - player.x1;
			player_delta_x = -(950.0 - player.x1);
			moving_stage_x = true;
		}

		else if (should_swap_x)
			std::swap(delta_x, player_delta_x);
	}

	if (!moving_stage_y)
	{
		if (player.y1 > 530.0 && player.y1 - delta_y <= 530.0)
		{
			delta_y -= player.y1 - 530.0;
			player_delta_y = player.y1 - 530.0;
			moving_stage_y = true;
		}

		else if (player.y1 < 530.0 && player.y1 - delta_y >= 530.0)
		{
			delta_y += 530.0 - player.y1;
			player_delta_y = -(530.0 - player.y1);
			moving_stage_y = true;
		}

		else if (should_swap_y)
			std::swap(delta_y, player_delta_y);
	}

	if (player.x1 - player_delta_x < 0.0)
		player_delta_x = player.x1;

	if (player.x2 - player_delta_x > 1920.0)
		player_delta_x = player.x2 - 1920.0;

	if (player.y1 - player_delta_y < chat_height)
		player_delta_y = player.y1 - chat_height;

	if (player.y2 - player_delta_y > 1080)
		player_delta_y = player.y2 - 1080;

	int start, end, inc;
	if (direction == "up") //otherwise you jump past 1-pixel boxes while walking up
	{
		start = bounding_boxes.size() - 1;
		end = -1;
		inc = -1;
	}

	else
	{
		start = 0;
		end = bounding_boxes.size();
		inc = 1;
	}

	int ladder_index = -1;
	for (int i = start; i != end; i += inc) //TODO colliding with multiple boxes at once?
	{
		bool intersected = false;
		ScreenElement bb = bounding_boxes[i];
		if (intersects(ScreenElement(bb.x1 + delta_x, bb.y1, bb.x2 + delta_x, bb.y2, ""), 
			ScreenElement(player.x1 - player_delta_x, player.y1, player.x2 - player_delta_x, player.y1 + 3, "")) && !ignore_collision)
		{
			intersected = true;
			if (player.x2 <= bb.x1)
			{
				float target = bb.x1 - player.x2;
				float to_go = abs(delta_x) + abs(player_delta_x) - target;
				delta_x += to_go;
				if (delta_x >= 0.0)
				{
					player_delta_x += delta_x;
					delta_x = 0.0;
				}
			}

			if (bb.x2 <= player.x1)
			{
				float target = player.x1 - bb.x2;
				float to_go = delta_x + player_delta_x - target;
				delta_x -= to_go;
				if (delta_x <= 0.0)
				{
					player_delta_x += delta_x;
					delta_x = 0.0;
				}
			}
		}

		if (intersects(ScreenElement(bb.x1, bb.y1 + delta_y, bb.x2, bb.y2 + delta_y, ""),
			ScreenElement(player.x1, player.y1 - player_delta_y, player.x2, player.y1 - player_delta_y + 3, "")) && !ignore_collision)
		{
			intersected = true;
			if (player.y1 + 3 <= bb.y1)
			{
				float target = bb.y1 - (player.y1 + 3);
				float to_go = abs(delta_y) + abs(player_delta_y) - target;
				delta_y += to_go;
				if (delta_y >= 0.0)
				{
					player_delta_y += delta_y;
					delta_y = 0.0;
				}
			}

			if (bb.y2 <= player.y1)
			{
				float target = player.y1 - bb.y2;
				float to_go = delta_y + player_delta_y - target;
				delta_y -= to_go;
				if (delta_y <= 0.0)
				{
					player_delta_y += delta_y;
					delta_y = 0.0;
				}
			}
		}

		if (intersected && bb.name == "ladder") //start zoom
		{
			ladder_index = i;
		}
	}

	player.translate(-player_delta_x, -player_delta_y);
	translate_stage(delta_x, delta_y);
	if (ladder_index != -1)
	{
		ladder_box = bounding_boxes[ladder_index];
		zoom_animation = true;
		no_movement = true;
		player_to_center_x = ladder_box.x1 + ((ladder_box.x2 - ladder_box.x1) / 2) - ((player.x2 - player.x1) / 2) - player.x1;
		player_to_center_y = ladder_box.y1 + ((ladder_box.y2 - ladder_box.y1) / 2) - ((player.y2 - player.y1) / 2) - player.y1;
	}

	total_distance += abs(player_delta_x == 0 ? 0 : walk_speed) + abs(player_delta_y == 0 ? 0 : walk_speed) + abs(delta_x == 0 ? 0 : walk_speed) + abs(delta_y == 0 ? 0 : walk_speed);
	if (total_distance >= 120)
	{
		parent->sound_manager->play_sound("step.wav");
		total_distance = 0;
	}

	for (int i = 0; i < warp_boxes.size(); ++i)
	{
		if (inside(player, warp_boxes[i]))
		{
			playing_warping_animation = true;
			warp_target = warp_boxes[i].name;
		}
	}

	if (player.y2 >= 700 && current_level == "underground.txt" && key == 'w')
	{
		parent->go_to_flying = true;
		keys_pressed['w'] = false;
		keys_pressed['a'] = false;
		keys_pressed['s'] = false;
		keys_pressed['d'] = false;
	}

	mouse_moved(0, 0);
}

void MMO::mouse_clicked(int button, int state, int x, int y)
{
	if (!swinging_sword && current_level != "underground.txt" && current_level != "mmo.txt")
	{
		swinging_sword = true;
		sword_counter = 0;
		no_movement = true;
		player.swing();
	}
}

void MMO::animate()
{
	if (app_state == 0)
		return;

	if (animating_stage)
	{
		translate_stage(0, 10);
		if (background_images[0].y1 >= 0)
		{
			animating_stage = false;
			no_movement = false;
			moving_stage_x = true;
			moving_stage_y = true;
		}
	}
	
	if (current_level == "mmo.txt")
	{
		frames++;
		if (frames > 120 && scrolling_messages.empty())
		{
			scrolling_messages.push_back(ScrollingMessage(700, 700, 700, 700, 0));
			scrolling_messages.push_back(ScrollingMessage(900, 900, 900, 900, 30));
		}

		if (frames > 300 && no_movement && !zoom_animation)
			no_movement = false;

		for (int i = 0; i < scrolling_messages.size(); ++i)
			scrolling_messages[i].animate();

		if (zoom_animation)
		{
			if (player_to_center_x != 0)
				player_to_center_x -= copysign(abs(player_to_center_x) < 2 ? abs(player_to_center_x) : 2, player_to_center_x);

			if (player_to_center_y != 0)
				player_to_center_y -= copysign(abs(player_to_center_y) < 2 ? abs(player_to_center_y) : 2, player_to_center_y);

			scale_x += 0.007;
			scale_y += 0.007;
			percent_of_centering += 0.02;
			if (percent_of_centering >= 1.0)
				percent_of_centering = 1.0;

			zoom_counter++;
			if (zoom_counter >= 7 * 60)
			{
				zoom_animation = false;
				animating_stage = true;
				load_level("underground.txt");
				translate_stage(0, -1300);
			}
		}
	}

	if (swinging_sword)
	{
		sword_counter++;
		if (sword_counter > 9)
		{
			swinging_sword = false;
			player.normal();
			no_movement = false;
		}
	}
	
	if (playing_warping_animation)
	{
		warping_alpha += warping_delta;
		if (warping_alpha >= 1.0)
		{
			warping_alpha = 1.0;
			warping_delta *= -1.0;
			load_level(warp_target);
		}

		if (warping_alpha <= 0.0)
		{
			warping_alpha = 0.0;
			warping_delta *= -1.0;
			playing_warping_animation = false;
		}
	}

	for (int i = 0; i < animated_components.size(); ++i)
	{
		animated_components[i].translate(8, 0);
		if (animated_components[i].x1 >= 2000)
		{
			int delta = animated_components[i].x2 - animated_components[i].x1;
			animated_components[i].x1 = -500;
			animated_components[i].x2 = -500 + delta;
		}
	}

	animation_counter++;
	int yvector = (keys_pressed['w'] ? 1 : 0) + (keys_pressed['s'] ? -1 : 0);
	int xvector = (keys_pressed['d'] ? 1 : 0) + (keys_pressed['a'] ? -1 : 0);
	int old_speed = walk_speed;
	if (yvector != 0 && xvector != 0)
		walk_speed /= sqrt(2.0);

	if (keys_pressed['w'])
		process_key('w');

	if (keys_pressed['a'])
		process_key('a');

	if (keys_pressed['s'])
		process_key('s');

	if (keys_pressed['d'])
		process_key('d');

	walk_speed = old_speed;
	parent->mmo_stage_x1 = background_images[0].x1;
	parent->mmo_stage_y1 = background_images[0].y1;
	parent->mmo_player_x1 = player.x1;
	parent->mmo_player_y1 = player.y1;
}

void MMO::add_chat()
{
	parent->screen_elements.insert(parent->screen_elements.end(), make_unique<HelpCenter>(0.0, 0.0, 1920.0, chat_height, "edges.bmp", parent, MMO_GAME, true));
}

void MMO::add_children()
{
	auto splash_screen = make_unique<ScreenElement>(0, 0, 1920, 1080, "mmosplash.png", MMO_GAME);
	auto xbutton = make_unique<XButton>(900, 900, 1000, 1000, "xbutton.png", parent, MMO_GAME);
	auto name_field = make_unique<TextField>(800, 700, 1100, 800, "textfield.png", parent, MMO_GAME);
	auto password_field = make_unique<TextField>(800, 600, 1100, 700, "textfield.png", parent, MMO_GAME);
	password_field->password = true;
	password_box = password_field.get();
	auto login_button = make_unique<Button>(800, 500, 900, 600, "ok.png", MMO_GAME, [this]()
	{
		if (parent->mmo_password != "" && password_box->text == parent->mmo_password)
		{
			app_state++;
			remove_children();
			parent->screen_elements.insert(parent->screen_elements.end(), make_unique<HelpCenter>(0.0, 100.0, 1920.0, 400.0, "edges.bmp", parent, MMO_GAME, true));
		}

		//TODO wanna make sure this doesn't repeat and add more screen elements?
		else
		{
			auto invalid_password = make_unique<InfoText>(1130, 600, 1130, 600, "white.png", "Invalid password", MMO_GAME);
			children.push_back(invalid_password.get());
			parent->screen_elements.insert(parent->screen_elements.end(), std::move(invalid_password));
		}
	}, []() {});
	auto forgot_password_button = make_unique<Button>(1000, 500, 1100, 600, "ok.png", MMO_GAME, [this]()
	{
		auto retrieve_password = make_unique<InfoText>(800, 400, 800, 400, "white.png", "Please enter your email address to receive your password", MMO_GAME);
		auto email_field = make_unique<TextField>(800, 300, 1100, 350, "textfield.png", parent, MMO_GAME);
		email_box = email_field.get();
		auto retrieve_button = make_unique<Button>(1150, 300, 1250, 350, "ok.png", MMO_GAME, [this]()
		{
			if (email_box->text == "rosecoloredavarice@nenda.en.os")
				parent->emails.insert(parent->emails.begin(), "recoveremailbold.png");

			auto email_sent = make_unique<InfoText>(800, 200, 800, 200, "white.png", "Email sent", MMO_GAME);
			children.push_back(email_sent.get());
			parent->screen_elements.insert(parent->screen_elements.end(), std::move(email_sent));
		}, []() {});

		children.push_back(retrieve_password.get());
		children.push_back(email_field.get());
		children.push_back(retrieve_button.get());
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(retrieve_password));
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(email_field));
		parent->screen_elements.insert(parent->screen_elements.end(), std::move(retrieve_button));
	}, []() {});
	children.push_back(splash_screen.get());
	children.push_back(xbutton.get());
	children.push_back(name_field.get());
	children.push_back(password_field.get());
	children.push_back(login_button.get());
	children.push_back(forgot_password_button.get());
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(splash_screen));
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(xbutton));
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(name_field));
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(password_field));
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(login_button));
	parent->screen_elements.insert(parent->screen_elements.end(), std::move(forgot_password_button));
}

void MMO::remove_children()
{
	for (int i = 0; i < parent->screen_elements.size(); ++i)
	{
		for (int j = 0; j < children.size(); ++j)
		{
			if (children[j] == parent->screen_elements[i].get())
				parent->screen_elements[i]->marked_for_deletion = true;
		}
	}

	children.clear();
}