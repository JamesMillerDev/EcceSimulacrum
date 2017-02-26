#ifndef VECTOR3
#define VECTOR3

struct Vector3
{
	float x;
	float y;
	float z;
	Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
	Vector3() : x(0.0), y(0.0), z(0.0) {}
};

struct Vector3int
{
	int x;
	int y;
	int z;
	Vector3int(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
	Vector3int() : x(0.0), y(0.0), z(0.0) {}
};

struct Vector4
{
	float x;
	float y;
	float z;
	float q;
	Vector4(float x_, float y_, float z_, float q_) : x(x_), y(y_), z(z_), q(q_) {}
	Vector4() : x(0.0), y(0.0), z(0.0), q(0.0) {}
};

#endif