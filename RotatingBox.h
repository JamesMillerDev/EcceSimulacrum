#ifndef ROTATING_BOX
#define ROTATING_BOX

#include "Computer.h"
#include "Vector3.h"
#include "FallingLetter.h"
#include "fonts.h"

struct RotatingBox : public ScreenElement
{
	float distance;
	float speed;
	float to_traverse;
	float traversed = 0;
	int speed_sign;
	int fall_speed = 0;
	bool animating = false;
	vector<FallingLetter> falling_letters;
	vector<pair<ScreenElement, float>> shards;
	Computer* parent;
	bool done_shards = false;
	RotatingBox(float _x1, float _y1, float _x2, float _y2, string _name) : ScreenElement(_x1, _y1, _x2, _y2, _name, BROWSER)
	{
		distance = 300;
		speed = 2;
		to_traverse = 100;
		speed_sign = -1;
	}

	void mouse_clicked(int button, int state, int x, int y)
	{
		animating = true;
		if (rotated)
			fall_speed = -20;
	}

	void animate_falling_letters()
	{
		for (int i = 0; i < falling_letters.size(); ++i)
		{
			FallingLetter& letter = falling_letters[i];
			letter.translate(5, 0);
			if (letter.x1 > distance)
			{
				if (letter.sign == 2)
				{
					if (angle < -90)
						letter.sign = 1;

					else if (angle > -90)
						letter.sign = -1;

					else letter.sign = 0;
					letter.rotation_angle = angle;
				}

				letter.translate(0, letter.sign * 5);
			}
		}
	}

	void animate()
	{
		if (!animating)
			return;
		
		angle += speed * speed_sign;
		traversed += speed;
		if (traversed >= to_traverse)
		{
			traversed = 0;
			speed_sign *= -1;
			to_traverse /= 2.0;
			speed -= 0.3;
			if (speed <= 0)
			{
				speed = 0;
				rotated = true;
			}

			if (!done_shards)
			{
				done_shards = true;
				name = "brokentextfield.png";
				shards.push_back(pair<ScreenElement, float>(ScreenElement(300, 0, 391, 30, "shard3.png"), angle));
				shards.push_back(pair<ScreenElement, float>(ScreenElement(400, 0, 491, 30, "shard3.png"), angle));
				shards.push_back(pair<ScreenElement, float>(ScreenElement(500, 0, 591, 30, "shard3.png"), angle));
				shards.push_back(pair<ScreenElement, float>(ScreenElement(300, 0, 356, 31, "shard2.png"), angle));
				shards.push_back(pair<ScreenElement, float>(ScreenElement(400, 0, 456, 31, "shard2.png"), angle));
				shards.push_back(pair<ScreenElement, float>(ScreenElement(500, 0, 556, 31, "shard2.png"), angle));
				shards.push_back(pair<ScreenElement, float>(ScreenElement(300, 0, 361, 30, "shard1.png"), angle));
				shards.push_back(pair<ScreenElement, float>(ScreenElement(400, 0, 461, 30, "shard1.png"), angle));
				shards.push_back(pair<ScreenElement, float>(ScreenElement(500, 0, 561, 30, "shard1.png"), angle));
			}
		}

		animate_falling_letters();
		for (int i = 0; i < shards.size(); ++i)
			shards[i].first.translate(10, (i % 2 == 0 ? 1 : -1) * 5);

		translate(0, fall_speed);
		if (y2 < -20)
		{
			parent->increment_breaking_stage();
			animating = false;
		}
	}

	void draw(TextureManager* texture_manager)
	{
		ScreenElement::draw(texture_manager);
		glPushMatrix();
		glTranslatef(x1, y1, 0.0);
		for (int i = 0; i < falling_letters.size(); ++i)
		{
			glPushMatrix();
			if (falling_letters[i].sign == 2)
				glRotatef(angle, 0.0, 0.0, 1.0);

			else 
				glRotatef(falling_letters[i].rotation_angle, 0.0, 0.0, 1.0);

			falling_letters[i].draw(texture_manager);
			glPopMatrix();
		}

		for (int i = 0; i < shards.size(); ++i)
		{
			glPushMatrix();
			glRotatef(shards[i].second, 0.0, 0.0, 1.0);
			shards[i].first.draw(texture_manager);
			glPopMatrix();
		}

		glPopMatrix();
	}

	void prepare_string(string str)
	{
		vector<float> string_lengths = draw_string(parent->texture_manager, 32, str, 0, 0, false);
		for (int i = 0; i < str.length(); ++i)
		{
			string arg = "32"; //"32" + str[i] doesn't work
			arg += str[i];
			falling_letters.push_back(FallingLetter(string_lengths[i + 1] - parent->texture_manager->get_width(arg) + 10, 10, 0, 0, string(1, str[i])));
		}
	}
};

#endif