#ifndef __QUAT_H__
#define __QUAT_H__

#pragma once

#include "vec3.h"

class Quaternion
{
public:
	//! Default constructor
	Quaternion() : X(0.0f), Y(0.0f), Z(0.0f), W(1.0f) {}

	//! Constructor
	Quaternion(float x, float y, float z, float w) : X(x), Y(y), Z(z), W(w) {}

	//! Constructor which converts Euler angles (radians) to a quaternion
	Quaternion(float EulerX, float EulerY, float EulerZ);

	//! Constructor which converts Euler angles (radians) to a quaternion
	Quaternion(const Vec3& EulerXYZ);


	void getMatrix(Mat44& outMatrix);


	//! Add operator
	Quaternion operator+(const Quaternion& rhs) const;

	//! Equality operator
	bool operator==(const Quaternion& rhs) const;

	//! Inequality operator
	bool operator!=(const Quaternion& rhs) const;

	//! Assignment operator
	inline Quaternion& operator=(const Quaternion& rhs);

	//! Multiplication operator
	Quaternion operator*(const Quaternion& other) const;

	//! Multiplication operator with scalar
	Quaternion operator*(float s) const;

	//! Multiplication operator with scalar
	Quaternion& operator*=(float s);

	//! Multiplication operator
	Vec3 operator*(const Vec3& rhs) const;


	
	//! Calculates the dot product
	inline float dotProduct(const Quaternion& other) const;

	//! Sets new quaternion
	inline Quaternion& set(float x, float y, float z, float w);

	//! Sets new quaternion (based on Euler angles - radians)
	inline Quaternion& set(float EulerX, float EulerY, float EulerZ);


	//! Sets new quaternion (based on Euler angles - radians)
	inline Quaternion& set(const Vec3& EulerXYZ);

	//! Sets new quaternion from other one
	inline Quaternion& set(const Quaternion& other);



	//! Normalizes the quaternion
	inline Quaternion& normalize();


	
	//! Set this quaternion to the result of the slerp between two quaternions.
	Quaternion& slerp(Quaternion q1, Quaternion q2, float time, float threshold = 0.05f);

	//! Set this quaternion to the linear interpolation between two quaternions
	Quaternion& lerp(Quaternion& q1, Quaternion& q2, float time);

	//! Output this quaternion to an Euler angle (radians)
	void toEuler(Vec3& eulerOut) const;

	//! Set this quaternion to indentity
	Quaternion& makeIdentity();

	//! Set this quaternion to represent a rotation from one vector to another
	Quaternion& rotationFromTo(const Vec3& from, const Vec3& to);
	


	float X;	// vectorial (imaginary) part
	float Y;
	float Z;
	float W;	// real part

};

#endif