#include <algorithm>
#include "Plane.h"

using namespace std;

Plane::Plane(Vector3 origin, Vector3 norm, float delta1, float delta2, float delta3, string _name)
{
	p1 = origin;
	normal = norm;
	Vector3 basis1, basis2;
	if (normal.y != 0)
		basis1 = Vector3(delta1, (-delta1 * normal.x - delta2 * normal.z) / normal.y, delta2);

	else if (normal.x != 0)
		basis1 = Vector3((-delta1 * normal.y - delta2 * normal.z) / normal.x, delta1, delta2);

	else if (normal.z != 0)
		basis1 = Vector3(delta1, delta2, 0);

	basis2 = cross(basis1, normal);
	p2 = p1 + basis1;
	p3 = p1 + (basis2 * delta3);
	p4 = p3 + basis1;
	name = _name;
}

Plane::Plane(Vector3 _p1, Vector3 _p2, Vector3 _p3, Vector3 _p4, Vector3 _normal, string _name, bool _strict, int _expansion)
{
	p1 = _p1;
	p2 = _p2;
	p3 = _p3;
	p4 = _p4;
	normal = _normal;
	name = _name;
	strict = _strict;
	expansion = _expansion;
}

void Plane::calculate_bounds()
{
	minx = min(p1.x, min(p2.x, min(p3.x, p4.x)));
	maxx = max(p1.x, max(p2.x, max(p3.x, p4.x)));
	miny = min(p1.y, min(p2.y, min(p3.y, p4.y)));
	maxy = max(p1.y, max(p2.y, max(p3.y, p4.y)));
	minz = min(p1.z, min(p2.z, min(p3.z, p4.z)));
	maxz = max(p1.z, max(p2.z, max(p3.z, p4.z)));
}

Plane::Plane()
{

}

float Plane::dot(Vector3 a, Vector3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 Plane::cross(Vector3 a, Vector3 b)
{
	return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

bool Plane::point_in_bounds(Vector3 point)
{
	return same_side2(p3, point, p1, p3 - p1) &&
		same_side2(p2, point, p1, p2 - p1) &&
		same_side2(p1, point, p2, p1 - p2) &&
		same_side2(p1, point, p3, p1 - p3);
}

bool Plane::same_side(Vector3 a, Vector3 b)
{
	return copysign(1, (a - p1).dot(normal)) == copysign(1, (b - p1).dot(normal));
}

bool Plane::same_side2(Vector3 a, Vector3 b, Vector3 p, Vector3 n)
{
	if (!strict)
		return copysign(1, (a - p).dot(n)) == copysign(1, (b - p).dot(n));
	
	float x = (a - p).dot(n);
	float y = (b - p).dot(n);
	return x != 0 && y != 0 && copysign(1, x) == copysign(1, y);
}

float Plane::distance_to_point(Vector3 a, Vector3 b, Vector3 p)
{
	return cross(p - a, p - b).length() / (b - a).length();
}