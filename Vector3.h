#ifndef VECTOR3
#define VECTOR3

struct Vector3
{
	float x;
	float y;
	float z;
	Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
	Vector3() : x(0.0), y(0.0), z(0.0) {}
	Vector3 operator*(const float& rhs)
	{
		return Vector3(x * rhs, y * rhs, z * rhs);
	}

	Vector3 operator+(const Vector3& rhs)
	{
		return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	Vector3 operator-(const Vector3& rhs)
	{
		return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	float dot(Vector3 rhs)
	{
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	float length()
	{
		return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
	}

	Vector3 change_length(float new_length)
	{
		return (*this * new_length) * (1.0 / length());
	}

	bool operator==(const Vector3& rhs)
	{
		return x == rhs.x && y == rhs.y && z == rhs.z;
	}
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
	bool operator==(const Vector4& rhs) const
	{
		return x == rhs.x && y == rhs.y && z == rhs.z && q == rhs.q;
	}
};

#endif