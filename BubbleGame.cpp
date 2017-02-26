#include <algorithm>
#include <time.h>
#include "BubbleGame.h"
#include "ProgressBar.h"
#include "fonts.h"

//TODO if bullets are really long, longer than aliens, just make sure bullet velocity < bullet length + alien length
//TODO hold down on keyboard, hold mouse button, tilt to the left, infinite points and breaks the game... (leave that in maybe?  you're on a timer...)

bool intersects(ScreenElement s1, ScreenElement s2)
{
	return (s1.x1 < s2.x2 && s1.x2 > s2.x1 &&
			s1.y1 < s2.y2 && s1.y2 > s2.y1);
}

BubbleGame::BubbleGame(float _x1, float _y1, float _x2, float _y2, std::string _name, Computer* _parent, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent)
{
	invincible = false;
	health = 5;
	hit_frame = 0;
	score = 0;
	score_to_give = 20;
	half_x = glutGet(GLUT_WINDOW_WIDTH) / 2.0;
	half_y = glutGet(GLUT_WINDOW_HEIGHT) / 2.0;
	timebase = glutGet(GLUT_ELAPSED_TIME);
	game_state = TITLE_SCREEN;
	show_message = false;
	velocity.first = 0;
	velocity.second = 0;
	generate_stars();
	score_table.push_back(pair<string, int>("JIM", 1000));
	score_table.push_back(pair<string, int>("JIM", 1300));
	score_table.push_back(pair<string, int>("JIM", 1500));
	score_table.push_back(pair<string, int>("JIM", 2000));
	score_table.push_back(pair<string, int>("JIM", 2500));
	parent->play_sound(2);
}

void BubbleGame::generate_stars()
{
	int window_x = glutGet(GLUT_WINDOW_WIDTH);
	int window_y = glutGet(GLUT_WINDOW_HEIGHT);
	for (int i = player_world_x - 2 * window_x; i < player_world_x + 2 * window_x; i += window_x)
	{
		for (int j = player_world_y - 2 * window_y; j < player_world_y + 2 * window_y; j += window_y)
		{
			for (int k = 0; k < 20; ++k)
			{
				int sx = rand() % window_x + i;
				int sy = rand() % window_y + j;
				//game_objects.push_back(SpaceGameObject(sx, sy, sx + 10, sy + 10, "star.png", 3));
				game_objects.insert(game_objects.begin(), SpaceGameObject(sx, sy, sx + 10, sy + 10, "star.png", 3));
			}
		}
	}
}

void BubbleGame::add_health(int value)
{
	health += value;
	if (health > 5)
		health = 5;
}

void BubbleGame::subtract_health(int value)
{
	health -= value;
	if (health < 0)
		health = 0;
}

bool BubbleGame::is_in_range(int target, int lower, int upper)
{
	return lower <= target && target <= upper;
}

void BubbleGame::mouse_clicked(int button, int state, int x, int y)
{
	if (button != GLUT_LEFT)
		return;

	if (state == GLUT_DOWN)
	{
		mouse_held = true;
		mouse_held_frames = 0;
	}

	else if (state == GLUT_UP)
		mouse_held = false;
}

void BubbleGame::press_key(unsigned char key)
{
	if (game_state == TITLE_SCREEN)
	{
		game_state = PLAYING;
		frames = 0;
		parent->stop_sound(2);
		parent->play_sound(3);
		return;
	}

	if (game_state == GAME_OVER && !still_exploding && !done)
	{
		score_table[score_index].first[cur_index] = key;
		cur_index++;
		if (cur_index == 3)
			done = true;

		return;
	}
	
	switch (key)
	{
	case 'w':
		velocity.second = SHIP_VELOCITY;
		break;

	case 'a':
		velocity.first = -SHIP_VELOCITY;
		break;

	case 's':
		velocity.second = -SHIP_VELOCITY;
		break;

	case 'd':
		velocity.first = SHIP_VELOCITY;
		break;
	}
}

void BubbleGame::release_key(unsigned char key)
{
	switch (key)
	{
	case 'w':
		if (velocity.second == SHIP_VELOCITY)
			velocity.second = 0;

		break;

	case 'a':
		if (velocity.first == -SHIP_VELOCITY)
			velocity.first = 0;

		break;

	case 's':
		if (velocity.second == -SHIP_VELOCITY)
			velocity.second = 0;

		break;

	case 'd':
		if (velocity.first == SHIP_VELOCITY)
			velocity.first = 0;

		break;
	}
}

void BubbleGame::give_focus()
{
	ScreenElement::give_focus();
	if (has_focus)
		return;

	glutSetCursor(GLUT_CURSOR_NONE);
	glutWarpPointer(half_x, glutGet(GLUT_WINDOW_HEIGHT) - half_y);
	parent->someone_set_cursor = true;
}

void BubbleGame::kill_alien(SpaceGameObject* target)
{
	target->type = 4;
	parent->play_sound(1);
	target->name = "explosion1.png";
	int target_center_x = (target->x2 - target->x1) / 2 + target->x1;
	int target_center_y = (target->y2 - target->y1) / 2 + target->y1;
	float inc_angle = 360.0 / 6;
	for (float i = 0; i < 360.0; i += inc_angle)
	{
		float xv = cos(i * PI / 180.0);
		float yv = sin(i * PI / 180.0);
		float dx1 = target_center_x + xv * 10.0;
		float dy1 = target_center_y + yv * 10.0;
		SpaceGameObject debris(dx1, dy1, dx1 + 40, dy1 + 9, "spinnerorange.png", 5);
		debris.velocity_x = xv * 6.0;
		debris.velocity_y = yv * 6.0;
		debris.angle = i;
		game_objects.push_back(debris);
	}
}

//0 = alien bullet, 1 = alien, 2 = player bullet
void BubbleGame::animate()
{
	if (game_state == TITLE_SCREEN)
	{
		if (frames % 30 == 0)
			show_message = !show_message;

		frames++;
		return;
	}
	
	if (done)
		return;

	if (game_state == GAME_OVER)
	{
		if (!parent->is_playing(1) && !show_game_over && still_exploding)
		{
			show_game_over = true;
			parent->play_sound(4);
		}

		int cur_time = glutGet(GLUT_ELAPSED_TIME);
		if (still_exploding && cur_time - timebase > 2000 && show_game_over)
		{
			still_exploding = false;
			show_game_over = false;
			for (int i = score_table.size() - 1; i >= 0; --i)
			{
				if (score > score_table[i].second)
				{
					score_index = i;
					score_table[i].first = "AAA";
					score_table[i].second = score;
					break;
				}
			}

			if (score_index == -1)
				done = true;
		}

		if (!still_exploding)
		{
			if (frames % 30 == 0)
				show_cur_letter = !show_cur_letter;
		}

		frames++;
		return;
	}

	if (invincible && frames - hit_frame >= 90)
		invincible = false;

	player_world_x += velocity.first;
	player_world_y += velocity.second;
	for (int i = 0; i < game_objects.size(); ++i)
	{
		auto it = &(game_objects[i]);
		//MOVE ALIEN
		if (it->type == 1)
		{
			float dist = sqrt(pow(player_world_x - it->x1, 2) + pow(player_world_y - it->y2, 2));
			if (dist > 500)
			{
				float slope = ((float)(it->y1 - player_world_y)) / ((float)(it->x1 - player_world_x)); //Please don't divide by 0!
				float tx = sqrt(pow(1, 2) / (1 + pow(slope, 2)));
				float ty = tx * slope;
				if (it->x1 > player_world_x)
				{
					tx *= -1.0;
					ty *= -1.0;
				}

				it->velocity_x = tx * 10;
				it->velocity_y = ty * 10;
				it->mulling = false;
			}

			if (dist < 300)
				it->mulling = true;

			if (rand() % 60 == 0 && it->mulling)
			{
				it->velocity_x = rand() % 30 - 15;
				it->velocity_y = rand() % 30 - 15;
			}

			it->translate(it->velocity_x, it->velocity_y);
		}

		if (it->type == 2)
			it->translate(it->velocity_x, it->velocity_y);

		if (it->type == 0)
			it->translate(it->velocity_x, it->velocity_y);

		if (it->type == 6)
			it->animate();

		if (it->type == 5)
		{
			it->translate(it->velocity_x, it->velocity_y);
			it->frame_counter++;
		}

		if (it->type == 4)
		{
			it->death_counter++;
			if (it->name == "explosion1.png")
				it->name = "explosion2.png";

			else it->name = "explosion1.png";
		}
	}

	float player_size = 50.0 * ((float)glutGet(GLUT_WINDOW_WIDTH) / 1920.0);
	ScreenElement player_element(player_world_x - player_size, player_world_y - player_size, player_world_x + player_size, player_world_y + player_size, "spaceship.png");
	vector<SpaceGameObject> to_remove;
	bool remove_stars = false;
	if (frames % 60 == 0)
		remove_stars = true;

	for (int i = 0; i < game_objects.size(); ++i)
	{
		auto it = &(game_objects[i]);
		if (it->type == 1 && intersects(player_element, *it) && !invincible)
		{
			kill_alien(it);
			subtract_health(3);
			parent->play_sound(1);
			if (health == 0)
			{
				game_state = GAME_OVER;
				timebase = glutGet(GLUT_ELAPSED_TIME);
				parent->stop_sound(3);
			}

			else
			{
				invincible = true;
				hit_frame = frames;
			}
		}

		if (it->type == 0 && intersects(player_element, *it) && !invincible)
		{
			to_remove.push_back(*it);
			subtract_health(1);
			parent->play_sound(1);
			if (health == 0)
			{
				game_state = GAME_OVER;
				timebase = glutGet(GLUT_ELAPSED_TIME);
				parent->stop_sound(3);
			}

			else
			{
				invincible = true;
				hit_frame = frames;
			}
		}

		if (it->type == 6 && intersects(player_element, *it))
		{
			to_remove.push_back(*it);
			add_health(2);
		}

		if (it->type == 3 && remove_stars)
			to_remove.push_back(*it);

		if (it->type == 5)
		{
			if (it->frame_counter > 30)
				to_remove.push_back(*it);
		}

		if (it->type == 1)
		{
			for (int j = 0; j < game_objects.size(); ++j) //TODO optimize by checking for fewer intersections?
			{
				auto other_it = &(game_objects[j]);
				if (other_it->type == 2 && intersects(*it, *other_it))
				{
					for (int i = 0; i < game_objects.size(); ++i)
					{
						auto target = &(game_objects[i]);
						if (*target == *it)
						{
							score += score_to_give;
							score_to_give = 20;
							kill_alien(target);
							if (rand() % 100 == 0)
							{
								SpaceGameObject health_pickup(target->x1, target->y1, target->x2, target->y2, "energy.png", 6);
								game_objects.push_back(health_pickup);
							}
						}
					}

					to_remove.push_back(*other_it);
				}
			}
		}
	}

	for (int i = 0; i < game_objects.size(); ++i)
	{
		auto it = &(game_objects[i]);
		if (it->type == 2 || it->type == 0)
		{
			if (!intersects(*it, ScreenElement(player_world_x - half_x, player_world_y - half_y, player_world_x + half_x, player_world_y + half_y, "")))
			{
				to_remove.push_back(*it);
				if (it->type == 2 && score_to_give > 0)
					score_to_give--;
			}
		}

		if (it->type == 4)
		{
			if (it->death_counter > 30)
				to_remove.push_back(*it);
		}
	}

	game_objects.erase(remove_if(game_objects.begin(), game_objects.end(), [this, to_remove](SpaceGameObject& obj)
	{
		for (int i = 0; i < to_remove.size(); ++i)
		{
			if (obj == to_remove[i])
				return true;
		}

		return false;
	}), game_objects.end());

	if (remove_stars)
		generate_stars();

	if (frames % 120 == 0)
	{
		//SPAWN ALIEN
		for (int i = 0; i < aliens_to_spawn; ++i)
		{
			int alien_x = player_world_x;
			int alien_y = player_world_y;
			alien_x += (rand() % 2 == 0) ? half_x + 100 : -(half_x + 100);
			alien_y += (rand() % 2 == 0) ? half_y + 100 : -(half_y + 100);
			SpaceGameObject alien(alien_x, alien_y, alien_x + 100, alien_y + 100, "alien.png", 1);
			alien.velocity_x = rand() % 30 - 15;
			alien.velocity_y = rand() % 30 - 15;
			alien.frame_counter = rand() % 120;
			game_objects.push_back(alien);
		}
	}

	if (frames % 600 == 0)
		aliens_to_spawn++;

	if (mouse_held)
	{
		if (mouse_held_frames % 10 == 0)
		{
			int center_x = cos((angle + 90) * (PI / 180)) * 100 + player_world_x;
			int center_y = sin((angle + 90) * (PI / 180)) * 100 + player_world_y;
			int bx1 = center_x - 10;
			int by1 = center_y - 10;
			int bx2 = center_x + 10;
			int by2 = center_y + 10;
			SpaceGameObject bullet(bx1 + 4, by1 - 10, bx2 - 4, by2 + 10, "bullet.png", 2);
			bullet.angle = angle;
			float slope = ((float)(center_y - player_world_y)) / ((float)(center_x - player_world_x)); //Please don't divide by 0!
			float tx = sqrt(pow(1, 2) / (1 + pow(slope, 2)));
			float ty = tx * slope;
			if (center_x < player_world_x)
			{
				tx *= -1.0;
				ty *= -1.0;
			}

			bullet.velocity_x = tx * 30;
			bullet.velocity_y = ty * 30;
			game_objects.push_back(bullet); //TODO adding to game_objects only gives 1 frame input lag?
			parent->play_sound(5);
		}

		mouse_held_frames++;
	}

	for (int i = 0; i < game_objects.size(); ++i)
	{
		auto it = &(game_objects[i]);
		if (it->type == 1)
		{
			it->frame_counter++;
			if (it->frame_counter % 120 == 0)
			{
				int bx1 = it->x1;
				int by1 = it->y1;
				int bx2 = it->x1 + 20;
				int by2 = it->y1 + 20;
				SpaceGameObject bullet(bx1 + 4, by1 - 10, bx2 - 4, by2 + 10, "alienbullet.png", 0);
				float slope = ((float)(it->y1 - player_world_y)) / ((float)(it->x1 - player_world_x)); //Please don't divide by 0!
				float tx = sqrt(pow(1, 2) / (1 + pow(slope, 2)));
				float ty = tx * slope;
				if (it->x1 > player_world_x)
				{
					tx *= -1.0;
					ty *= -1.0;
				}

				bullet.velocity_x = tx * 20;
				bullet.velocity_y = ty * 20;
				float arctan = atan2(bullet.velocity_y, bullet.velocity_x); //TODO don't divide by 0!
				bullet.angle = (180 / PI) * arctan + 90;
				game_objects.push_back(bullet);
			}
		}
	}

	frames++;
}

void BubbleGame::mouse_moved(int x, int y)
{
	x = scalex(x);
	y = scaley(y);
	if (game_state == GAME_OVER)
	{
		//kill_counter++; what in the world was this for?
		return;
	}
	
	mouse_moved_x = x;
	mouse_moved_y = y;
	
	if (x == half_x && y == half_y)
		return;

	float slope = (float)(y - half_y) / (float)(x - half_x);
	angle = atan(slope) * (180.0 / PI) + 90.0;
	if (x >= half_x)
		angle += 180.0;
	
	float distance = sqrt(pow(x - half_x, 2) + pow(y - half_y, 2));
	d = distance;
	if (distance > 100)
	{
		float radians = (angle + 90) * (PI / 180.0);
		float nx = cos(radians) * 50 + half_x;
		float ny = ((sin(radians) * 50) + half_y);
		float nny = glutGet(GLUT_WINDOW_HEIGHT) - ny;
		wx = nx;
		wy = nny;
		glutWarpPointer(cos(radians) * 98 + half_x, glutGet(GLUT_WINDOW_HEIGHT) - ((sin(radians) * 98) + half_y));
	}

	glutSetCursor(GLUT_CURSOR_NONE);
	parent->someone_set_cursor = true;
	return;
}

void BubbleGame::draw(TextureManager* texture_manager)
{
	ScreenElement::draw(texture_manager);
	glPushMatrix();
		glTranslatef(half_x - player_world_x, half_y - player_world_y, 0);
		for (auto it = game_objects.begin(); it != game_objects.end(); ++it)
			it->draw(texture_manager);
	glPopMatrix();
	float player_size = 50.0 * ((float)glutGet(GLUT_WINDOW_WIDTH) / 1920.0);
	ScreenElement player(-player_size, -player_size, player_size, player_size, "ship.png", BROWSER);
	player.no_scale = true;
	glPushMatrix();
	glTranslatef(half_x, half_y, 0);
	glRotatef(angle, 0.0, 0.0, 1.0);
	if (game_state == GAME_OVER && still_exploding)
		player.name = "explosion1.png";

	if (still_exploding && !show_game_over)
	{
		if (invincible)
		{
			if (frames % 4 == 0 || frames % 4 == 1)
				glColor4f(1.0, 1.0, 1.0, 0.7);

			else glColor4f(1.0, 1.0, 1.0, 0.0);
		}

		player.draw(texture_manager);
		glColor4f(1.0, 1.0, 1.0, 1.0);
	}

	glPopMatrix();
	glPushMatrix();
		glTranslatef(half_x, half_y, 0);
		if (game_state == TITLE_SCREEN)
		{
			ScreenElement title(-300, -150, 300, 150, "conquest.png", BROWSER);
			title.draw(texture_manager);
			if (show_message)
			{
				ScreenElement message(-300, -120 - 300, 300, 120 - 300, "pressanykey.png", BROWSER);
				message.draw(texture_manager);
			}
		}
	glPopMatrix();
	if (game_state == PLAYING)
	{
		glColor4f(1.0, 0.0, 0.0, 1.0);
		draw_string(texture_manager, 90, "Score: ", 50, 850, true, true);
		draw_string(texture_manager, 90, to_string(score), 200, 850, true, true);
		draw_string(texture_manager, 90, "Life: ", 50, 810, true, true);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		ProgressBar health_bar(200, 810, 350, 840, "red.png", parent, NONE, []() {}, true);
		health_bar.progress = (float)health / 5.0;
		health_bar.draw(texture_manager);
	}

	if (game_state == GAME_OVER)
	{
		if (show_game_over)
		{
			ScreenElement game_over(half_x - 350, half_y - 200, half_x + 350, half_y + 200, "gameover.png");
			game_over.center();
			game_over.no_scale = true;
			game_over.draw(texture_manager);
		}
		
		if (!still_exploding)
		{
			ScreenElement table(0, 0, 400, 600, "highscoretable.png");
			table.center();
			table.no_scale = true;
			table.draw(texture_manager);
			int cur_y = 500;
			glColor4f(1.0, 0.0, 0.0, 1.0);
			draw_string(texture_manager, 90, "High Scores", 800, 750, true, true);
			for (int i = 0; i < score_table.size(); ++i)
			{
				glColor4f(1.0, 0.0, 0.0, 1.0);
				if (i != score_index)
					draw_string(texture_manager, 90, to_string(score_table[i].second) + " " + score_table[i].first, 800, cur_y, true, true);

				else
				{
					string display_string = score_table[i].first;
					if (!show_cur_letter)
						display_string[cur_index] = ' ';

					draw_string(texture_manager, 90, to_string(score_table[i].second) + " " + display_string, 800, cur_y, true, true);
				}

				glColor4f(1.0, 1.0, 1.0, 1.0);
				cur_y += 50;
			}
		}
	}
}