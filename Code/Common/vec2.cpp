#include "PCH.h"
#include <cmath>
#include "Vec2.h"


Vec2::Vec2()
	: x(0.f)
	, y(0.f)
{

}

//----------------------------------------------------------------------
Vec2::Vec2(float _x, float _y)
	: x(_x)
	, y(_y)
{

}

//----------------------------------------------------------------------
Vec2::Vec2(const Vec2& r)
	: x(r.x)
	, y(r.y)
{

}

//----------------------------------------------------------------------
void Vec2::Set(const float _x, const float _y)
{
	x = _x;
	y = _y;
}

//----------------------------------------------------------------------
void Vec2::Zero()
{
	x = 0.f;
	y = 0.f;
}

//----------------------------------------------------------------------
Vec2 Vec2::operator+(const Vec2& r) const
{
	return Vec2(x + r.x, y + r.y);
}

//----------------------------------------------------------------------
Vec2 Vec2::operator-(const Vec2& r) const
{
	return Vec2(x - r.x, y - r.y);
}

//----------------------------------------------------------------------
Vec2 Vec2::operator*(const Vec2& r) const
{
	return Vec2(x * r.x, y * r.y);
}

//----------------------------------------------------------------------
Vec2 Vec2::operator*(float r) const
{
	return Vec2(x * r, y * r);
}

//----------------------------------------------------------------------
Vec2 operator*(float l, const Vec2& r)
{
	return r * l;
}

//----------------------------------------------------------------------
Vec2 Vec2::operator/(const Vec2& r) const
{
	return Vec2(x / r.x, y / r.y);
}

//----------------------------------------------------------------------
Vec2 Vec2::operator/(float r) const
{
	return Vec2(x / r, y / r);
}

//----------------------------------------------------------------------
Vec2 Vec2::operator-() const
{
	return Vec2(-x, -y);
}

//----------------------------------------------------------------------
float Vec2::GetLength() const
{
	return sqrtf(x * x + y * y);
}

//----------------------------------------------------------------------
float Vec2::GetDistance(const Vec2& r) const
{
	const float dx = x - r.x;
	const float dy = y - r.y;

	return sqrtf(dx * dx + dy * dy);
}

//----------------------------------------------------------------------
float Vec2::GetDistanceSquared(const Vec2& r) const
{
	const float dx = x - r.x;
	const float dy = y - r.y;

	return (dx * dx + dy * dy);
}

//----------------------------------------------------------------------
void Vec2::Normalize()
{
	*this / GetLength();
}

