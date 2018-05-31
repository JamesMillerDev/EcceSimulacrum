#ifndef PHYSICS_PIECE
#define PHYSICS_PIECE

#include "Computer.h"

struct PhysicsPiece : public ScreenElement
{
	float velocity_x;
	float velocity_y;
	float rotation;
	float accel_x;
	float accel_y;
	PhysicsPiece() : ScreenElement(0, 0, 0, 0, "") {}
	PhysicsPiece(float _x1, float _y1, float _x2, float _y2, string _name) : ScreenElement(_x1, _y1, _x2, _y2, _name)
	{
		velocity_x = velocity_y = rotation = accel_x = accel_y = 0.0;
	}

	void animate()
	{
		translate(velocity_x, velocity_y);
		velocity_x += accel_x;
		velocity_y += accel_y;
		angle += rotation;
	}

	void fall()
	{
		accel_y = (5 * -9.8) / 60.0;
		rotation = 1.0;
	}
};

#endif