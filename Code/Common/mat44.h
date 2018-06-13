#ifndef __mat44_h__
#define __mat44_h__

#pragma once

#pragma warning (disable : 4201)	// temp

class /*_declspec(align(16))*/ Mat44
{
	typedef const float cfloat;

public:
	Mat44();
	Mat44(const Mat44& r);
	Mat44(cfloat* pArr);
	Mat44(cfloat m11, cfloat m12, cfloat m13, cfloat m14,
		cfloat m21, cfloat m22, cfloat m23, cfloat m24,
		cfloat m31, cfloat m32, cfloat m33, cfloat m34,
		cfloat m41, cfloat m42, cfloat m43, cfloat m44);	


	void Set(cfloat m11, cfloat m12, cfloat m13, cfloat m14,
		cfloat m21, cfloat m22, cfloat m23, cfloat m24,
		cfloat m31, cfloat m32, cfloat m33, cfloat m34,
		cfloat m41, cfloat m42, cfloat m43, cfloat m44);

	// @TODO: Remove
	float GetDeterminant() const;

	Vec3 TransformPoint(const Vec3& v) const;
	Vec4 TransformPoint(const Vec4& v) const;
	Vec3 TransformVector(const Vec3& v) const;

	void SetRotation(float yaw, float pitch, float roll);

	Mat44 operator*(const Mat44& r) const;

	float operator()(unsigned int row, unsigned int col) const;
	float& operator()(unsigned int row, unsigned int col);
	
public:
	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;
};

#endif