#include <algorithm>
#include <time.h>
#include "BubbleGame.h"
#include "ProgressBar.h"
#include "Button.h"
#include "fonts.h"

//TODO if bullets are really long, longer than aliens, just make sure bullet velocity < bullet length + alien length
//TODO hold down on keyboard, hold mouse button, tilt to the left, infinite points and breaks the game... (leave that in maybe?  you're on a timer...)

BubbleGame::BubbleGame(float _x1, float _y1, float _x2, float _y2, std::string _name, Computer* _parent, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent)
{
	max_health = 10;
	fire_interval = 20;
	frames = 1;
	invincible = false;
	extending = false;
	mouse_held = false;
	score_mode = false;
	spawn_interval = 60;
	extend_counter = 0;
	health = max_health;
	hit_frame = 0;
	glow_sign = 1;
	fuel = 6000;
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
	parent->sound_manager->play_sound("title.wav", 80, 0, 0, 0, 0, 0, 0, 0, true);
	//parent->play_sound(6);
	game_objects.push_back(SpaceGameObject(100, 100, 200, 200, "fuel.png", 7));
}

string rank_string(int num)
{
	if (num == 1)
		return "1ST";

	else if (num == 2)
		return "2ND";

	else if (num == 3)
		return "3RD";

	else return to_string(num) + "TH";
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
				game_objects.insert(game_objects.begin(), SpaceGameObject(sx, sy, sx + 30, sy + 30, "star.png", 3));
			}
		}
	}
}

void BubbleGame::add_health(int value)
{
	health += value;
	if (health > max_health)
		health = max_health;

	do_glow_animation = true;
	green_glow = true;
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
	if (button != GLUT_LEFT && button != GLUT_RIGHT_BUTTON)
		return;

	if (button == GLUT_LEFT)
		rapid_fire = true;

	else rapid_fire = false;

	if (state == GLUT_DOWN)
	{
		mouse_held = true;
		mouse_held_frames = 0;
	}

	else if (state == GLUT_UP)
		mouse_held = false;
}

bool BubbleGame::responds_to(int button)
{
	if (button == GLUT_LEFT)
		return true;

	else if (button == GLUT_RIGHT_BUTTON)
		return true;

	else if (button == 3)
		return false;

	else if (button == 4)
		return false;

	else if (button == 1)
		return false;
}

void BubbleGame::initiate_game()
{
	extending = true;
	parent->sound_manager->stop_sound("title.wav");
	parent->sound_manager->play_sound("takeoff.wav");
	parent->queue_click_x = 0; //TODO make sure never any links at pixel 0
	parent->queue_click_y = 700;
	mouse_moved(0, 0);
	parent->maintain_cursor = true;
}

void BubbleGame::press_key(unsigned char key)
{
	if (game_state == TITLE_SCREEN)
	{
		/*game_state = PLAYING;
		frames = 0;
		parent->stop_sound(2);
		parent->play_sound(3);*/
		return;
	}

	if (game_state == GAME_OVER && !still_exploding && !done)
	{
		parent->score_table[score_index].first[cur_index] = key;
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

	//case 'i':
	//	invincible = false;
	//	break;
	}

	if (velocity.first != 0 || velocity.second != 0)
	{
		flame_target_length = (fuel == 0) ? 40 : 60;
		do_flame_animation = true;
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

	if (velocity.first == 0 && velocity.second == 0)
	{
		flame_target_length = 0;
		do_flame_animation = true;
	}
}

void BubbleGame::give_focus()
{
	ScreenElement::give_focus();
	if (has_focus)
		return;
}

void BubbleGame::create_debris(int target_center_x, int target_center_y)
{
	float inc_angle = 360.0 / 6;
	for (float i = 0; i < 360.0; i += inc_angle)
	{
		float xv = cos(i * PI / 180.0);
		float yv = sin(i * PI / 180.0);
		float dx1 = target_center_x + xv * 10.0;
		float dy1 = target_center_y + yv * 10.0;
		SpaceGameObject debris(dx1, dy1, dx1 + 9 * 5, dy1 + 40 * 5, "debris1.png", 5);
		debris.velocity_x = xv * 6.0;
		debris.velocity_y = yv * 6.0;
		debris.angle = i;
		to_add.push_back(debris);
	}
}

void BubbleGame::kill_alien(SpaceGameObject* target)
{
	target->type = 4;
	parent->sound_manager->play_sound("bomb.wav");
	target->name = "fireframe1.png";
	int target_center_x = (target->x2 - target->x1) / 2 + target->x1;
	int target_center_y = (target->y2 - target->y1) / 2 + target->y1;
	create_debris(target_center_x, target_center_y);
	if (target->slow_alien)
		slow_aliens_killed++;
}

void BubbleGame::spawn_aliens(int num_to_spawn)
{
	for (int i = 0; i < num_to_spawn; ++i)
	{
		int alien_x = player_world_x;
		int alien_y = player_world_y;
		if (rand() % 2 == 0)
		{
			alien_x += (rand() % 2 == 0) ? half_x + (spawn_slow_aliens ? 150 : 100) : -(half_x + (spawn_slow_aliens ? 150 : 100));
			alien_y += rand() % (2 * ((int)half_y + (spawn_slow_aliens ? 150 : 100))) - (half_y + (spawn_slow_aliens ? 150 : 100));
		}

		else
		{
			alien_y += (rand() % 2 == 0) ? half_y + 100 : -(half_y + 100);
			alien_x += rand() % (2 * ((int)half_x + 100)) - (half_x + 100);
		}

		SpaceGameObject alien(alien_x, alien_y, alien_x + 100, alien_y + 100, "alien.png", 1); //100
		alien.velocity_x = rand() % 30 - 15;
		alien.velocity_y = rand() % 30 - 15;
		alien.frame_counter = rand() % 120;
		if (spawn_slow_aliens)
		{
			alien.mulling = true;
			alien.slow_alien = true;
			alien.velocity_x = 5;
			alien.velocity_y = 5;
		}

		to_add.push_back(alien);
	}
}

void BubbleGame::process_hit()
{
	parent->sound_manager->play_sound("bomb.wav");
	if (health == 0)
	{
		game_state = GAME_OVER;
		timebase = glutGet(GLUT_ELAPSED_TIME);
		parent->sound_manager->stop_sound("main.wav");
		to_add.push_back(SpaceGameObject(player_world_x - 50, player_world_y - 50, player_world_x + 50, player_world_y + 50, "fireframe1.png", 4));
		create_debris(player_world_x, player_world_y); //TODO careful, this adds objects
		do_glow_animation = false;
		glow_alpha = 0.0;
	}

	else
	{
		invincible = true;
		hit_frame = frames;
	}
}

//0 = alien bullet, 1 = alien, 2 = player bullet
void BubbleGame::animate()
{
	if (game_state == TITLE_SCREEN)
	{
		if (extending)
		{
			extend_counter++;
			if (extend_counter >= 80)
			{
				game_state = PLAYING;
				mouse_held = false;
				frames = 0;
				parent->sound_manager->stop_sound("takeoff.wav");
				parent->sound_manager->play_sound("main.wav", 80, 0, 0, 0, 0, 0, 0, 0, true);
			}

			return;
		}
		
		if (frames % 30 == 0)
			show_message = !show_message;

		frames++;
		return;
	}
	
	if (game_state == GAME_OVER)
	{
		if (!parent->sound_manager->is_playing("bomb.wav") && !show_game_over && still_exploding)
		{
			show_game_over = true;
			parent->maintain_cursor = false;
			parent->set_cursor("cursor.png");
			parent->sound_manager->play_sound("gameover.wav");
		}

		int cur_time = glutGet(GLUT_ELAPSED_TIME);
		if (still_exploding && cur_time - timebase > 2000 && show_game_over)
		{
			still_exploding = false;
			show_game_over = false;
			for (int i = parent->score_table.size() - 1; i >= 0; --i)
			{
				if (score > parent->score_table[i].second)
				{
					score_index = i;
					parent->score_table[i].first = "AAA";
					parent->score_table[i].second = score;
					break;
				}
			}

			auto retry = make_unique<Button>(0, 0, 430, 105, "retrygame.png", BROWSER, [this]() {this->scroll_panel->restart_game();}, []() {}, "", "invisible.png", "gendark.png");
			retry->center();
			retry->translate(0, -400);
			retry->shiny = true;
			retry->parent = parent;
			scroll_panel->children.push_back(retry.get());
			parent->to_be_added.push_back(std::move(retry));
			parent->process_control_changes();
			if (score_index == -1)
				done = true;
		}

		if (!still_exploding)
		{
			if (frames % 15 == 0)
				show_cur_letter = !show_cur_letter;
		}

		frames++;
		//return;
	}

	if (frames % 2 == 0)
	{
		flame_frame += flame_frame_increment;
		if (flame_frame >= 2)
			flame_frame_increment = -1;

		if (flame_frame <= 1)
			flame_frame_increment = 1;
	}

	if (do_flame_animation)
	{
		if (flame_length < flame_target_length)
		{
			flame_length += 10;
			if (flame_length >= flame_target_length)
			{
				flame_length = flame_target_length;
				do_flame_animation = false;
			}
		}

		if (flame_length > flame_target_length)
		{
			flame_length -= 10;
			if (flame_length <= flame_target_length)
			{
				flame_length = flame_target_length;
				do_flame_animation = false;
			}
		}

		if (flame_length == flame_target_length)
			do_flame_animation = false;
	}

	for (int i = 0; i < shot_particles.size(); ++i)
	{
		shot_particles[i].translate(shot_particles[i].velocity_x, shot_particles[i].velocity_y);
		shot_particles[i].alpha -= 0.20;
		if (i == shot_particles.size() - 1 && shot_particles[i].alpha <= 0.0)
			shot_particles.clear();
	}

	for (int i = 0; i < flame_particles.size(); ++i)
	{
		flame_particles[i].translate(flame_particles[i].velocity_x, flame_particles[i].velocity_y);
		flame_particles[i].alpha -= .20; //.20
	}

	flame_particles.erase(remove_if(flame_particles.begin(), flame_particles.end(), [](SpaceGameObject particle) {return particle.alpha <= 0.0;}), flame_particles.end());

	if (invincible && frames - hit_frame >= 50)
		invincible = false;

	if (do_glow_animation)
	{
		glow_alpha += glow_sign * .12;
		if (glow_alpha >= 1.0)
		{
			glow_alpha = 1.0;
			glow_sign = -1;
		}

		if (glow_alpha <= 0)
		{
			glow_alpha = 0;
			glow_sign = 1;
			do_glow_animation = false;
			green_glow = false;
		}
	}

	if (game_state != GAME_OVER)
	{
		player_world_x += velocity.first;
		player_world_y += velocity.second;
	}

	prev_objects.clear();
	for (int i = 0; i < game_objects.size(); ++i)
	{
		if (game_objects[i].type == 3)
		{
			if (game_state != GAME_OVER)
			{
				game_objects[i].translate(velocity.first, velocity.second);
				game_objects[i].translate(-velocity.first / 3, -velocity.second / 3);
			}

			if (game_objects[i].alpha == 1.0)
				game_objects[i].alpha -= 0.1;

			else if (game_objects[i].alpha <= 0.0)
				game_objects[i].alpha += 0.1;

			else game_objects[i].alpha += 0.1 * (rand() % 2 == 0 ? -1 : 1);
		}
	}

	if (velocity.first != 0 || velocity.second != 0)
	{
		fuel -= SHIP_VELOCITY;
		if (fuel <= 0)
		{
			fuel = 0;
			SHIP_VELOCITY = 3;
			if (velocity.first != 0)
				velocity.first = copysign(SHIP_VELOCITY, velocity.first);

			if (velocity.second != 0)
				velocity.second = copysign(SHIP_VELOCITY, velocity.second);

			flame_target_length = 40;
			do_flame_animation = true;
		}

		float player_size = 30.0 * ((float)glutGet(GLUT_WINDOW_WIDTH) / 1920.0); //40.0
		int px1 = -player_size;
		int py1 = -player_size;
		//int center_x = cos((angle + 90) * (PI / 180)) * 100 + player_world_x;
		//int center_y = sin((angle + 90) * (PI / 180)) * 100 + player_world_y;
		/*SpaceGameObject particle1 = SpaceGameObject(px1, py1, px1 + 7, py1 + 7, "alienbullet.png", 0);
		particle1.translate(rand() % 6 - 3, rand() % 30 * -1);
		particle1.velocity_x = rand() % 6 - 3;
		particle1.velocity_y = rand() % 20 * -1;
		flame_particles.push_back(particle1);*/

		int new_angle = angle - 180 + (rand() % 20 - 10);
		int center_x = cos((new_angle + 90) * (PI / 180)) * 100 + player_world_x;
		int center_y = sin((new_angle + 90) * (PI / 180)) * 100 + player_world_y;
		SpaceGameObject bullet(center_x, center_y, center_x + 7, center_y + 7, "alienbullet.png", 0);
		bullet.velocity_x = rand() % 6 - 3;
		bullet.velocity_x = rand() % 6 - 3;
		flame_particles.push_back(bullet); //TODO adding to game_objects only gives 1 frame input lag?

		/*px1 = player_size;
		SpaceGameObject particle2 = SpaceGameObject(px1, py1, px1 + 7, py1 + 7, "alienbullet.png", 0);
		particle2.translate(rand() % 6 - 3, rand() % 30 * -1);
		particle2.velocity_x = rand() % 6 - 3;
		particle2.velocity_y = rand() % 20 * -1;
		flame_particles.push_back(particle2);*/
	}

	for (int i = 0; i < game_objects.size(); ++i)
	{
		auto it = &(game_objects[i]);
		//MOVE ALIEN
		if (it->type == 1)
		{
			/*float dist = sqrt(pow(player_world_x - it->x1, 2) + pow(player_world_y - it->y2, 2));
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

			it->translate(it->velocity_x, it->velocity_y);*/

			float dist = sqrt(pow(player_world_x - it->x1, 2) + pow(player_world_y - it->y2, 2));
			if (dist > 800 && !it->slow_alien)
			{
				float slope = ((float)(it->y1 - player_world_y)) / ((float)(it->x1 - player_world_x)); //TODO Please don't divide by 0!
				float angle = atan(slope);
				if (it->x1 > player_world_x)
					angle += PI;

				it->move_angle = angle;
				it->velocity_x = 25;
				it->mulling = false;
			}

			if (dist < 600)
				it->mulling = true;

			if (it->mulling)
			{
				if (it->velocity_x == 0)
					it->velocity_x = 10;
				
				if (it->velocity_x > 10)
					it->velocity_x -= 1.5;

				if (rand() % 60 == 0)
					it->move_direction *= -1;

				it->move_angle += it->move_direction / 20;
			}

			it->translate(it->velocity_x * cos(it->move_angle), it->velocity_x * sin(it->move_angle));
		}

		if (it->type == 2)
			it->translate(it->velocity_x, it->velocity_y);

		if (it->type == 0)
		{
			it->translate(it->velocity_x, it->velocity_y);
			it->frame_counter++;
			if (it->frame_counter % 10 == 0)
			{
				if (it->name == "alienbullet1.png")
					it->name = "alienbullet2.png";

				else it->name = "alienbullet1.png";
			}
		}

		if (it->type == 6 || it->type == 7)
			it->animate();

		if (it->type == 5)
		{
			it->translate(it->velocity_x, it->velocity_y);
			it->frame_counter++;
			it->name = "debris" + to_string(it->frame_counter == 1 ? 1 : it->frame_counter / 2) + ".png";
			if (it->frame_counter > 20)
				it->name = "invisible.png";
		}

		if (it->type == 4)
		{
			it->death_counter++;
			//if (it->name == "explosion1.png")
			//	it->name = "explosion2.png";

			//else it->name = "explosion1.png";
			it->name = "fireframe" + to_string(it->death_counter == 1 ? 1 : it->death_counter / 2) + ".png";
			if (it->death_counter > 20)
				it->name = "invisible.png";
		}
	}

	float player_size = 40.0 * ((float)glutGet(GLUT_WINDOW_WIDTH) / 1920.0);
	ScreenElement player_element(player_world_x - player_size, player_world_y - player_size, player_world_x + player_size, player_world_y + player_size, "spaceship.png");
	vector<SpaceGameObject> to_remove;
	bool remove_stars = false;
	//if (frames % 60 == 0)
	//	remove_stars = true;

	for (int i = 0; i < game_objects.size(); ++i)
	{
		auto it = &(game_objects[i]);
		if (it->type == 1 && intersects(player_element, *it) && !invincible && game_state != GAME_OVER)
		{
			kill_alien(it);
			subtract_health(2);
			process_hit();
		}

		if (it->type == 0 && intersects(player_element, *it) && !invincible && game_state != GAME_OVER)
		{
			to_remove.push_back(*it);
			subtract_health(1);
			process_hit();
		}

		if (it->type == 6 && intersects(player_element, *it))
		{
			to_remove.push_back(*it);
			add_health(2);
		}

		if (it->type == 7 && intersects(player_element, *it))
		{
			if (fuel != 6000)
			{
				flame_target_length = 50;
				do_flame_animation = true;
			}

			fuel = 6000;
			SHIP_VELOCITY = 14;
			if (velocity.first != 0)
				velocity.first = copysign(SHIP_VELOCITY, velocity.first);

			if (velocity.second != 0)
				velocity.second = copysign(SHIP_VELOCITY, velocity.second);

			if (game_state != GAME_OVER)
				do_glow_animation = true;
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
					for (int k = 0; k < game_objects.size(); ++k)
					{
						auto target = &(game_objects[k]);
						if (*target == *it) //TODO got a crash on this line
						{
							target->health -= 1;
							if (target->health > 0)
							{
								target->name = "redalien.png";
								target->hit_on_frame = target->frame_counter;
							}

							else
							{
								score += score_to_give;
								if (2000 <= score && slow_aliens_killed < 3 && !spawn_slow_aliens)
								{
									spawn_slow_aliens = true;
									parent->start_playing_escape_sound("thesounds2.wav");
								}

								else if (spawn_slow_aliens && slow_aliens_killed >= 3)
									spawn_slow_aliens = false;

								score_to_give = 20;
								kill_alien(target);
								if (rand() % 100 == 0)
								{
									SpaceGameObject health_pickup(target->x1, target->y1, target->x1 + 100, target->y1 + 100, "energy.png", 6);
									to_add.push_back(health_pickup);
								}
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
			if (!(
				it->x1 < player_world_x + half_x && it->x2 > player_world_x - half_x &&
				it->y1 < player_world_y + half_y && it->y2 > player_world_y - half_y
				))
			{
				to_remove.push_back(*it);
				if (it->type == 2 && score_to_give > 0)
				{
					score_to_give--;
				}
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

	if (frames % spawn_interval == 0 && game_state != GAME_OVER)
	{
		bool any_aliens = false;
		for (int i = 0; i < game_objects.size(); ++i)
		{
			if (game_objects[i].type == 1)
			{
				any_aliens = true;
				break;
			}
		}

		if (spawn_slow_aliens)
		{
			if (!any_aliens)
				spawn_aliens(1);
		}

		else spawn_aliens(aliens_to_spawn);
	}

	if (frames % (30 * 60) && fire_interval > 10 && !spawn_slow_aliens)
		fire_interval -= 10;
	
	if (frames % (30 * 60) == 0 && spawn_interval > 30 && !spawn_slow_aliens && fire_interval < 20)
		spawn_interval -= 10;

	if (frames % (30 * 60) == 0 && spawn_interval <= 30 && !spawn_slow_aliens)
		aliens_to_spawn++;

	if (mouse_held && game_state != GAME_OVER)
	{
		if (mouse_held_frames % (rapid_fire ? 1 : 10) == 0) // 10
		{
			int new_angle = angle + (rapid_fire ? (rand() % 60 - 30) : 0);
			int center_x = cos((new_angle + 90) * (PI / 180)) * 100 + player_world_x;
			int center_y = sin((new_angle + 90) * (PI / 180)) * 100 + player_world_y;
			int bx1 = center_x - 10;
			int by1 = center_y - 10;
			int bx2 = center_x + 10;
			int by2 = center_y + 10;
			SpaceGameObject bullet(bx1 + 4, by1 - 10, bx2 - 4, by2 + 10, "bullet.png", 2);
			bullet.angle = new_angle;
			float slope = ((float)(center_y - player_world_y)) / ((float)(center_x - player_world_x)); //Please don't divide by 0!
			float tx = sqrt(pow(1, 2) / (1 + pow(slope, 2)));
			float ty = tx * slope;
			if (center_x < player_world_x)
			{
				tx *= -1.0;
				ty *= -1.0;
			}

			bullet.velocity_x = tx * 50;
			bullet.velocity_y = ty * 50;
			to_add.push_back(bullet); //TODO adding to game_objects only gives 1 frame input lag?
			for (int i = 0; i <= rand() % 4 + 1; ++i)
			{
				int px1 = cos((angle + 90) * (PI / 180)) * 50 + half_x;
				int py1 = sin((angle + 90) * (PI / 180)) * 50 + half_y;
				SpaceGameObject particle = SpaceGameObject(px1, py1, px1 + 7, py1 + 7, "bullet.png", 0);
				particle.translate(rand() % 20 - 10, rand() % 20 - 10);
				particle.velocity_x = rand() % 20 - 10;
				particle.velocity_y = rand() % 20 - 10;
				shot_particles.push_back(particle);
			}

			parent->sound_manager->play_sound("laser.wav");
		}

		mouse_held_frames++;
	}

	for (int i = 0; i < game_objects.size(); ++i)
	{
		auto it = &(game_objects[i]);
		if (it->type == 1)
		{
			it->frame_counter++;
			if (it->frame_counter - it->hit_on_frame > 3)
				it->name = "alien.png";

			if (it->frame_counter % fire_interval == 0 && !it->slow_alien) //TODO was 120
			{
				int bx1 = it->x1;
				int by1 = it->y1;
				int bx2 = it->x1 + 20;
				int by2 = it->y1 + 20;
				//SpaceGameObject bullet(bx1 + 4, by1 - 10, bx2 - 4, by2 + 10, "alienbullet.png", 0);
				SpaceGameObject bullet(bx1, by1, bx2, by2, "alienbullet1.png", 0);
				float slope = ((float)(it->y1 - player_world_y)) / ((float)(it->x1 - player_world_x)); //Please don't divide by 0!
				float tx = sqrt(pow(1, 2) / (1 + pow(slope, 2)));
				float ty = tx * slope;
				if (it->x1 > player_world_x)
				{
					tx *= -1.0;
					ty *= -1.0;
				}

				bullet.velocity_x = tx * 12;
				bullet.velocity_y = ty * 12;
				float arctan = atan2(bullet.velocity_y, bullet.velocity_x); //TODO don't divide by 0!
				bullet.angle = (180 / PI) * arctan + 90;
				to_add.push_back(bullet);
			}
		}
	}

	if (game_state != GAME_OVER)
		frames++;

	for (int i = 0; i < to_add.size(); ++i)
		game_objects.push_back(to_add[i]);

	to_add.clear();
}

void BubbleGame::mouse_moved(int x, int y)
{
	x = scalex(x);
	y = scaley(y);
	if (game_state == GAME_OVER || (game_state == TITLE_SCREEN && !extending))
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

	parent->set_cursor("invisible.png");
	parent->someone_set_cursor = true;
	return;
}

void BubbleGame::draw(TextureManager* texture_manager)
{
	ScreenElement::draw(texture_manager);
	glPushMatrix();
		glTranslatef(half_x - player_world_x, half_y - player_world_y, 0);
		for (auto it = game_objects.begin(); it != game_objects.end(); ++it)
		{
			if (!(score_mode && it->name == "fuel.png"))
				it->draw(texture_manager);
		}

		for (int i = 0; i < prev_objects.size(); ++i)
			prev_objects[i].draw(texture_manager);

		//for (int i = 0; i < flame_particles.size(); ++i)
		//	flame_particles[i].draw(texture_manager);
	glPopMatrix();
	float player_size = 30.0 * ((float)glutGet(GLUT_WINDOW_WIDTH) / 1920.0);
	ScreenElement player(-player_size, -player_size, player_size, player_size, "ship.png", BROWSER);
	ScreenElement glow(-player_size - 20, -player_size - 22, player_size + 20, player_size + 22, green_glow ? "greenblur.png" : "blueblur.png", BROWSER);
	player.no_scale = true;
	glPushMatrix();
	glTranslatef(half_x, half_y, 0);
	glRotatef(angle, 0.0, 0.0, 1.0);
	if (game_state == GAME_OVER && still_exploding)
		player.name = "invisible.png";

	if (still_exploding && !show_game_over)
	{
		glColor4f(1.0, 1.0, 1.0, (invincible && (frames % 4 == 2 || frames % 4 == 3)) ? 0.0 : glow_alpha);
		glow.draw(texture_manager);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		if (invincible)
		{
			if (frames % 4 == 0 || frames % 4 == 1)
				glColor4f(1.0, 1.0, 1.0, 0.7);

			else glColor4f(1.0, 1.0, 1.0, 0.0);
		}

		if (!score_mode)
			player.draw(texture_manager);

		if (game_state != GAME_OVER)
		{
			string flame_name = "flameframe";
			flame_name += to_string(flame_frame);
			flame_name += ".png";
			ScreenElement flame1(-player_size - (flame_length / 4.5), -player_size - flame_length, -player_size + (flame_length / 4.5), -player_size, flame_name); //5.0
			ScreenElement flame2(player_size - (flame_length / 4.5), -player_size - flame_length, player_size + (flame_length / 4.5), -player_size, flame_name);
			//flame1.alpha = 0.6;
			//flame2.alpha = 0.6;
			flame1.draw(texture_manager);
			flame2.draw(texture_manager);
		}
		
		//for (int i = 0; i < flame_particles.size(); ++i)
		//	flame_particles[i].draw(texture_manager);

		glColor4f(1.0, 1.0, 1.0, 1.0);
	}

	glPopMatrix();
	glPushMatrix();
		glTranslatef(half_x, half_y, 0);
		if (game_state == TITLE_SCREEN)
		{
			if (extending)
			{
				int number = extend_counter >= 68 ? 34 : extend_counter / 2;
				ScreenElement title(-1920 / 2, -1080 / 2, 1920 / 2, 1080 / 2, "extend" + to_string(number) + ".png", BROWSER);
				title.draw(texture_manager);
				//ScreenElement overlay(-half_x, -half_y, half_x, half_y, "gameborder.png");
				//overlay.draw(texture_manager);
			}
			
			else
			{
				if (score_mode)
				{
					glPushMatrix();
					glLoadIdentity();
					draw_score_table(texture_manager);
					glPopMatrix();
				}
				
				else
				{
					//ScreenElement title(-300, -150, 300, 150, "conquest.png", BROWSER);
					ScreenElement title(-1920 / 2, -1080 / 2, 1920 / 2, 1080 / 2, "titlescreen" + to_string((frames / 2) % 40) + ".png", BROWSER);
					title.draw(texture_manager);
					//ScreenElement overlay(-half_x, -half_y, half_x, half_y, "gameborder.png");
					//overlay.draw(texture_manager);
					/*if (show_message)
					{
						ScreenElement message(-300, -120 - 300, 300, 120 - 300, "pressanykey.png", BROWSER);
						message.draw(texture_manager);
					}*/
				}
			}
		}
	glPopMatrix();
	if (game_state == PLAYING)
	{
		//glColor4f(1.0, 0.0, 0.0, 1.0);
		//draw_string(texture_manager, 90, "Score: ", 50, 850, true, true);
		ScreenElement scoretext(50, 850, 50 + 146, 900, "score.png");
		scoretext.draw(texture_manager);
		change_font("Audiowide-Regular.ttf");
		draw_string(texture_manager, 90, to_string(score), 200, 860, true, true);
		change_font("Lato-Regular.ttf");
		//draw_string(texture_manager, 90, "Life: ", 50, 810, true, true);
		ScreenElement lifetext(50, 810, 50 + 80, 860, "life.png");
		lifetext.draw(texture_manager);
		//draw_string(texture_manager, 90, "Fuel: ", 50, 710, true, true);
		ScreenElement fueltext(50, 710, 50 + 101, 760, "fueltext.png");
		fueltext.draw(texture_manager);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		ScreenElement background1(190, 800, 360, 850, "barbackground.png");
		ScreenElement background2(190, 700, 360, 750, "barbackground.png");
		background1.draw(texture_manager);
		background2.draw(texture_manager);
		ProgressBar health_bar(200, 810, 350, 840, "shinyred.png", parent, NONE, []() {}, true);
		health_bar.progress = (float)health / max_health;
		health_bar.draw(texture_manager);
		ProgressBar fuel_bar(200, 710, 350, 740, "shinyred.png", parent, NONE, []() {}, true);
		fuel_bar.progress = (float)fuel / 6000.0;
		fuel_bar.draw(texture_manager);
	}

	if (game_state == GAME_OVER)
	{
		if (show_game_over)
		{
			ScreenElement game_over(half_x - 350, half_y - 200, half_x + 350, half_y + 200, "gameover.png");
			game_over.center();
			game_over.draw(texture_manager);
		}
		
		if (!still_exploding)
			draw_score_table(texture_manager);
	}

	for (int i = 0; i < shot_particles.size(); ++i)
		shot_particles[i].draw(texture_manager);
}

void BubbleGame::draw_score_table(TextureManager* texture_manager)
{
	int cur_y = 1080 - 215;
	change_font("Audiowide-Regular.ttf");
	right_justify(texture_manager, 90, "RANK", 490, cur_y, true, true);
	right_justify(texture_manager, 90, "NAME", 1020, cur_y, true, true);
	right_justify(texture_manager, 90, "SCORE", 1544, cur_y, true, true);
	cur_y -= 63;
	for (int i = parent->score_table.size() - 1; i >= 0; --i)
	{
		if (i != score_index)
		{
			right_justify(texture_manager, 90, rank_string(parent->score_table.size() - i), 490, cur_y, true, true);
			right_justify(texture_manager, 90, to_string(parent->score_table[i].second), 1020, cur_y, true, true);
			right_justify(texture_manager, 90, parent->score_table[i].first, 1544, cur_y, true, true);
		}

		else
		{
			string display_string = parent->score_table[i].first;
			if (!show_cur_letter)
				display_string[cur_index] = '_';

			right_justify(texture_manager, 90, rank_string(parent->score_table.size() - i), 490, cur_y, true, true);
			right_justify(texture_manager, 90, to_string(parent->score_table[i].second), 1020, cur_y, true, true);
			right_justify(texture_manager, 90, display_string, 1544, cur_y, true, true);
		}

		cur_y -= 63;
	}

	change_font("Lato-Regular.ttf");
}

BubbleGame::~BubbleGame()
{
	//parent->sound_manager->stop_sound("title.wav");
	//parent->sound_manager->stop_sound("takeoff.wav");
	//parent->sound_manager->stop_sound("bomb.wav");
	//parent->sound_manager->stop_sound("main.wav");
	//parent->sound_manager->stop_sound("gameover.wav");
	//parent->sound_manager->stop_sound("laser.wav"); //TODO quitting while game is up causes crash (seems to be resolved)
}