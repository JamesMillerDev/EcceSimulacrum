#ifndef PLANE
#define PLANE

#include <string>
#include "Vector3.h"

using namespace std;

struct Plane
{
	Vector3 p1, p2, p3, p4;
	Vector3 normal;
	Plane(Vector3 origin, Vector3 norm, float delta1, float delta2, float delta3, string _name = "blue.png");
	Plane();
	Plane(Vector3 _p1, Vector3 _p2, Vector3 _p3, Vector3 _p4, Vector3 _normal, string _name = "blue.png", bool _strict = true, int _expansion = -1);
	float dot(Vector3 a, Vector3 b);
	Vector3 cross(Vector3 a, Vector3 b);
	bool point_in_bounds(Vector3 point);
	float minx, maxx, miny, maxy, minz, maxz;
	void calculate_bounds();
	bool same_side(Vector3 a, Vector3 b);
	bool same_side2(Vector3 a, Vector3 b, Vector3 p, Vector3 n);
	float distance_to_point(Vector3 a, Vector3 b, Vector3 p);
	string name;
	bool strict = false;
	int expansion;
};

#endif