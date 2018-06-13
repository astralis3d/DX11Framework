#ifndef __Vec3_h__
#define __Vec3_h__

#pragma once

class Vec3
{
public:
	Vec3();
	Vec3(const Vec3& v);
	Vec3(float _x, float _y, float _z);

	void Set(const float _x, const float _y, const float _z);
	void Zero();

	Vec3 operator+(const Vec3& r)	const;
	Vec3 operator-(const Vec3& r)	const;
	Vec3 operator*(const Vec3& r)	const;
	Vec3 operator/(const Vec3& r)	const;
	Vec3 operator*(float r)			const;	
	Vec3 operator/(float r)			const;
	friend Vec3 operator*(float l, const Vec3& r);

	Vec3& operator+=(const Vec3& r);
	Vec3& operator-=(const Vec3& r);
	Vec3& operator*=(const Vec3& r);
	Vec3& operator/=(const Vec3& r);
	Vec3& operator*=(float r);
	Vec3& operator/=(float r);

	float& operator[](int i);
	float operator[](int i) const;

	bool operator==(const Vec3& r);
	bool operator!=(const Vec3& r);

	Vec3 operator-() const;


	// Useful functions
	void Normalize();
	float GetLength() const;

	inline float GetDotProduct(const Vec3& v)	const;
	Vec3 GetCrossProduct(const Vec3& v)			const;

	float GetDistance(const Vec3& v)			const;
	float GetDistanceSquared(const Vec3& v)		const;
	float GetDistanceXZ(const Vec3& v)			const;
	float GetDistanceXZSquared(const Vec3& v)	const;
	float GetDistanceXY(const Vec3& v)			const;	
	float GetDistanceXYSquared(const Vec3& v)	const;

public:	
	float x;
	float y;
	float z;		
};

float Vec3::GetDotProduct(const Vec3& v) const
{
	return x*v.x + y*v.y + z*v.z;
}

#endif