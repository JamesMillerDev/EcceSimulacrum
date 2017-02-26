#include <fstream>
#include "MMO.h"
#include "ObjModel.h"
#include "BubbleGame.h"
#include "HelpCenter.h"
#include "XButton.h"
#include "InfoText.h"
#include "keymappings.h"

bool inside(ScreenElement s1, ScreenElement s2)
{
	return s1.x1 >= s2.x1 && s1.x2 <= s2.x2 && s1.y1 >= s2.y1 && s1.y2 <= s2.y2;
}

//TODO probably doesn't work in-browser right now
MMO::MMO(float _x1, float _y1, float _x2, float _y2, string _name, string initial_level, Computer* _parent, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent)
{
	warped = false;
	player = ScreenElement(950.0, 530.0, 950 + 114.0, 530.0 + 133.0, "frame1.png");
	animated_components.push_back(ScreenElement(0.0, 100.0, 150.0, 150.0, "river1.png"));
	animated_components.push_back(ScreenElement(100.0, 200.0, 300.0, 300.0, "river2.png"));
	animated_components.push_back(ScreenElement(1000.0, 150.0, 1150.0, 200.0, "river1.png"));
	animated_components.push_back(ScreenElement(1500.0, 300.0, 1700.0, 400.0, "river2.png"));
	animated_components.push_back(ScreenElement(-200.0, 200.0, -50.0, 250.0, "river1.png"));
	//load_level("mmo.txt");
	current_level = initial_level;
	load_level(initial_level);
	if (current_level == "mmo.txt")
		no_movement = true;

	else no_movement = false;
	zoom_animation = false;

	/*auto help_center = make_unique<HelpCenter>(0.0, 56.0, 1900.0, 200.0, "white.png", parent, application);
	parent->to_be_added.insert(parent->to_be_added.end() - 2, std::move(help_center)); what in the ever loving fuck, why was this here?*/

	app_state = 0;
}

void MMO::load_level(string file_name)
{
	background_images.clear();
	bounding_boxes.clear();
	ifstream file_stream(file_name);
	string line;
	while (getline(file_stream, line))
	{
		vector<string> components = split(line, ' ');
		if (components[0] == "bg")
			background_images.push_back(ScreenElement(atof(components[1].c_str()), atof(components[2].c_str()), atof(components[3].c_str()), atof(components[4].c_str()), components[5]));

		else if (components[0] == "sp")
			player.translate(atof(components[1].c_str()) - player.x1, atof(components[2].c_str()) - player.y1);

		else if (components[0] == "warp")
			warp_boxes.push_back(ScreenElement(atof(components[1].c_str()), atof(components[2].c_str()), atof(components[3].c_str()), atof(components[4].c_str()), components[5]));

		else
			bounding_boxes.push_back(ScreenElement(atof(components[0].c_str()), atof(components[1].c_str()), atof(components[2].c_str()), atof(components[3].c_str()), ""));
	}

	left_bottom = &(background_images[0]);
	right_top = &(background_images[background_images.size() - 1]);
}

//TODO make sure you can't click on other stuff
void MMO::mouse_moved(int x, int y)
{
	if (current_level == "mmo.txt")
	{
		if (warped)
			warped = false;

		else
		{
			warped = true;
			glutWarpPointer(player.x1 + 10, glutGet(GLUT_WINDOW_HEIGHT) - (player.y1 + 20));
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
	glScalef(scale_x, scale_y, 1.0);
	ScreenElement::draw(texture_manager);
	for (int i = 0; i < background_images.size(); ++i)
		background_images[i].draw(texture_manager);

	/*ScreenElement river = ScreenElement(0.0, 56.0, 1920.0, 616.0, "river.png");
	river.draw(texture_manager);*/
	/*for (int i = 0; i < animated_components.size(); ++i)
		animated_components[i].draw(texture_manager);*/
	if (current_level == "mmo.txt")
	{
		for (int i = 0; i < scrolling_messages.size(); ++i)
			scrolling_messages[i].draw(texture_manager);
	}

	if (current_level != "mmo.txt")
		player.draw(texture_manager);
	ScreenElement loading_screen = ScreenElement(0.0, 0.0, scalex(1920.0), scaley(1080.0), "black.png");
	glColor4f(1.0, 1.0, 1.0, warping_alpha);
	loading_screen.draw(texture_manager);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glPopMatrix();
	//glDisable(GL_STENCIL_TEST);
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

//TODO bottom of map is cut off
void MMO::press_key(unsigned char key)
{
	if (app_state == 0)
		return;
	
	if (no_movement)
		return;
	
	animation_counter++;
	if (animation_counter % 10 == 0)
		player.name = frame1;

	else if (animation_counter % 5 == 0)
		player.name = frame2;

	bool should_swap_x = !moving_stage_x;
	bool should_swap_y = !moving_stage_y;
	float delta_x = 0.0;
	float delta_y = 0.0;
	float player_delta_x = 0.0;
	float player_delta_y = 0.0;
	switch (key)
	{
	case 'w':
		delta_y = -15.0;
		if (player.name != "frame3.png" && player.name != "frame4.png")
			player.name = "frame3.png";
			
		frame1 = "frame3.png";
		frame2 = "frame4.png";
		break;

	case 's':
		delta_y = 15.0;
		if (player.name != "frame1.png" && player.name != "frame2.png")
			player.name = "frame1.png";

		frame1 = "frame1.png";
		frame2 = "frame2.png";
		break;

	case 'a':
		delta_x = 15.0;
		if (player.name != "frame7.png" && player.name != "frame8.png")
			player.name = "frame7.png";

		frame1 = "frame7.png";
		frame2 = "frame8.png";
		break;

	case 'd':
		delta_x = -15.0;
		if (player.name != "frame5.png" && player.name != "frame6.png")
			player.name = "frame5.png";

		frame1 = "frame5.png";
		frame2 = "frame6.png";
		break;
	}

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
		if (left_bottom->y1 + delta_y > 0.0)
		{
			player_delta_y = delta_y + left_bottom->y1;
			delta_y = -left_bottom->y1;
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

	if (player.y1 - player_delta_y < 56.0)
		player_delta_y = player.y1 - 56.0;

	if (player.y2 - player_delta_y > 1044.0)
		player_delta_y = player.y2 - 1044.0;

	for (int i = 0; i < bounding_boxes.size(); ++i) //TODO multiple intersections at once?
	{
		bool intersected = false;
		ScreenElement bb = bounding_boxes[i];
		if (intersects(ScreenElement(bb.x1 + delta_x, bb.y1, bb.x2 + delta_x, bb.y2, ""), 
			ScreenElement(player.x1 - player_delta_x, player.y1, player.x2 - player_delta_x, player.y2, "")))
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
			ScreenElement(player.x1, player.y1 - player_delta_y, player.x2, player.y2 - player_delta_y, "")))
		{
			intersected = true;
			if (player.y2 <= bb.y1)
			{
				float target = bb.y1 - player.y2;
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

		if (intersected)
		{
			zoom_animation = true;
			no_movement = true;
		}
	}

	player.translate(-player_delta_x, -player_delta_y);
	translate_stage(delta_x, delta_y);
	for (int i = 0; i < warp_boxes.size(); ++i)
	{
		if (inside(player, warp_boxes[i]))
		{
			playing_warping_animation = true;
			warp_target = warp_boxes[i].name;
		}
	}

	mouse_moved(0, 0);
}

void MMO::animate()
{
	if (app_state == 0)
		return;
	
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
			scale_x += 0.0001;
			scale_y += 0.0001;
		}

		return;
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