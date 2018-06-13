#include "PCH.h"
#include <assert.h>
#include <cmath>
#include "Vec3.h"


Vec3::Vec3()
	: x(0.f)
	, y(0.f)
	, z(0.f)
{

}

//----------------------------------------------------------------------
Vec3::Vec3(const Vec3& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
{

}

//----------------------------------------------------------------------
Vec3::Vec3(float _x, float _y, float _z)
	: x(_x)
	, y(_y)
	, z(_z)
{

}

//----------------------------------------------------------------------
void Vec3::Set(const float _x, const float _y, const float _z)
{
	x = _x;
	y = _y;
	z = _z;
}



//----------------------------------------------------------------------
Vec3 Vec3::operator+(const Vec3& r) const
{
	return Vec3(x + r.x, y + r.y, z + r.z);
}

//----------------------------------------------------------------------
Vec3 Vec3::operator-(const Vec3& r) const
{
	return Vec3(x - r.x, y - r.y, z - r.z);
}

//----------------------------------------------------------------------
Vec3 Vec3::operator*(const Vec3& r) const
{
	return Vec3(x * r.x, y * r.y, z * r.z);
}

//----------------------------------------------------------------------
Vec3 Vec3::operator*(float r) const
{
	return Vec3(x * r, y * r, z * r);
}

//----------------------------------------------------------------------
Vec3 Vec3::operator/(const Vec3& r) const
{
	return Vec3(x / r.x, y / r.y, z / r.z);
}

//----------------------------------------------------------------------
Vec3 Vec3::operator/(float r) const
{
	return Vec3(x / r, y / r, z / r);
}

//----------------------------------------------------------------------
Vec3 operator*(float l, const Vec3& r)
{
	return r * l;
}

//----------------------------------------------------------------------
bool Vec3::operator==(const Vec3& r)
{
	return (x == r.x && y == r.y && z == r.z);
}

//----------------------------------------------------------------------
bool Vec3::operator !=(const Vec3& r)
{
	return (x != r.x || y != r.y || z != r.z);
}

//----------------------------------------------------------------------
float& Vec3::operator[](int i)
{
	assert(i >= 0 && i <= 2);

	if (i == 0)
		return x;

	else if (i == 1)
		return y;

	else
		return z;
}

//----------------------------------------------------------------------
float Vec3::operator[](int i) const
{
	assert(i >= 0 && i <= 2);

	if (i == 0)
		return x;

	else if (i == 1)
		return y;

	else
		return z;
}

//----------------------------------------------------------------------
Vec3& Vec3::operator+=(const Vec3& r)
{
	x += r.x;
	y += r.y;
	z += r.z;

	return *this;
}

//----------------------------------------------------------------------
Vec3& Vec3::operator-=(const Vec3& r)
{
	x -= r.x;
	y -= r.y;
	z -= r.z;

	return *this;
}

//----------------------------------------------------------------------
Vec3& Vec3::operator*=(const Vec3& r)
{
	x *= r.x;
	y *= r.y;
	z *= r.z;

	return *this;
}

//----------------------------------------------------------------------
Vec3& Vec3::operator*=(float r)
{
	x *= r;
	y *= r;
	z *= r;

	return *this;
}

//----------------------------------------------------------------------
Vec3& Vec3::operator/=(const Vec3& r)
{
	x /= r.x;
	y /= r.y;
	z /= r.z;

	return *this;
}

//----------------------------------------------------------------------
Vec3& Vec3::operator/=(float r)
{
	x /= r;
	y /= r;
	z /= r;

	return *this;
}

//----------------------------------------------------------------------
Vec3 Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
void Vec3::Normalize()
{
	const float length = GetLength();

	x /= length;
	y /= length;
	z /= length;

	//*this /= GetLength();
}

//----------------------------------------------------------------------
float Vec3::GetLength() const
{
	return sqrt(x*x + y*y + z*z);
}

//----------------------------------------------------------------------
float Vec3::GetDistance(const Vec3& v) const
{
	const float dx = x - v.x;
	const float dy = y - v.y;
	const float dz = z - v.z;

	return sqrt(dx*dx + dy*dy + dz*dz);
}

//----------------------------------------------------------------------
float Vec3::GetDistanceXZ(const Vec3& v) const
{
	const float dx = x - v.x;
	const float dz = z - v.z;

	return sqrtf(dx*dx + dz*dz);
}

//----------------------------------------------------------------------
float Vec3::GetDistanceXY(const Vec3& v) const
{
	const float dx = x - v.x;
	const float dy = y - v.y;

	return sqrtf(dx*dx + dy*dy);
}

//----------------------------------------------------------------------
float Vec3::GetDistanceSquared(const Vec3& v) const
{
	const float dx = x - v.x;
	const float dy = y - v.y;
	const float dz = z - v.z;

	return (dx*dx + dy*dy + dz*dz);
}

//----------------------------------------------------------------------
float Vec3::GetDistanceXZSquared(const Vec3& v) const
{
	const float dx = x - v.x;
	const float dz = z - v.z;

	return (dx*dx + dz*dz);
}

//----------------------------------------------------------------------
float Vec3::GetDistanceXYSquared(const Vec3& v) const
{
	const float dx = x - v.x;
	const float dy = y - v.y;

	return (dx*dx + dy*dy);
}

//----------------------------------------------------------------------
Vec3 Vec3::GetCrossProduct(const Vec3& v) const
{
	const float cx = y * v.z - z * v.y;
	const float cy = z * v.x - x * v.z;
	const float cz = x * v.y - y * v.x;

	return Vec3(cx, cy, cz);
}

//----------------------------------------------------------------------
void Vec3::Zero()
{
	Set(0.f, 0.f, 0.f);
}




