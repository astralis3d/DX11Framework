#ifndef __Vec4_h__
#define __Vec4_h__

#pragma once

class Vec4
{
public:
	Vec4();
	Vec4(const Vec4& v);
	Vec4(float _x, float _y, float _z, float _w = 1.f);
	Vec4(float xyzw);

	void Set(const float _x, const float _y, const float _z, const float _w = 1.f);

	bool operator==(const Vec4& r);
	bool operator!=(const Vec4& r);

	float operator[](int i) const { return (&x)[i]; } 
	float& operator[](int i) { return (&x)[i]; }

	float x;
	float y;
	float z;
	float w;
};

#endif