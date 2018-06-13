#include "PCH.h"
#include "Vec4.h"

Vec4::Vec4()
	: x(0.f)
	, y(0.f)
	, z(0.f)
	, w(0.f)
{

}


Vec4::Vec4(const Vec4& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
	, w(v.w)
{

}

Vec4::Vec4(float _x, float _y, float _z, float _w)
	: x(_x)
	, y(_y)
	, z(_z)
	, w(_w)
{

}

Vec4::Vec4( float xyzw )
	: x(xyzw)
	, y(xyzw)
	, z(xyzw)
	, w(xyzw)
{

}

//----------------------------------------------------------------------
bool Vec4::operator==(const Vec4& r)
{
	return (x == r.x && y == r.y && z == r.z);
}

//----------------------------------------------------------------------
bool Vec4::operator !=(const Vec4& r)
{
	return !(*this == r);
}

//----------------------------------------------------------------------
void Vec4::Set(const float _x, const float _y, const float _z, const float _w)
{
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}
