#ifndef __Vec2_h__
#define __Vec2_h__

#pragma once

class Vec2
{
public:
	Vec2();
	Vec2(const Vec2& r);
	Vec2(float _x, float _y);

	void Set(const float _x, const float _y);
	void Zero();

	// Operators
	Vec2 operator+(const Vec2& r)		const;
	Vec2 operator-(const Vec2& r)		const;
	Vec2 operator*(const Vec2& r)		const;
	Vec2 operator/(const Vec2& r)		const;
	Vec2 operator*(float r)				const;
	Vec2 operator/(float r)				const;
	friend Vec2 operator*(float l, const Vec2& r);

	Vec2 operator-()					const;

	// Useful functions
	float GetLength()					const;

	float GetDistance(const Vec2& r)	const;
	float GetDistanceSquared(const Vec2& r) const;

	void Normalize();

public:
	float x;
	float y;
};

#endif