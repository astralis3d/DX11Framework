#include "PCH.h"
#include "mathlib.h"

Mat44::Mat44()
{
	// Set identity matrix

	Set(	1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
		);
}

Mat44::Mat44(const Mat44& r)
{
	if (this != &r)
	{
		Set(	r._11, r._12, r._13, r._14,
			r._21, r._22, r._23, r._24,
			r._31, r._32, r._33, r._34,
			r._41, r._42, r._43, r._44
			);
	}
}

Mat44::Mat44(cfloat* pArr)
{
	Set(	*(pArr + 0),	*(pArr + 1),	*(pArr + 2),	*(pArr + 3),
		*(pArr + 4),	*(pArr + 5),	*(pArr + 6),	*(pArr + 7),
		*(pArr + 8),	*(pArr + 9),	*(pArr + 10),	*(pArr + 11),
		*(pArr + 12),	*(pArr + 13),	*(pArr + 14),	*(pArr + 15)
		);
}

Mat44::Mat44(cfloat m11, cfloat m12, cfloat m13, cfloat m14, cfloat m21, cfloat m22, cfloat m23, cfloat m24, cfloat m31, cfloat m32, cfloat m33, cfloat m34, cfloat m41, cfloat m42, cfloat m43, cfloat m44)
{
	Set(	m11, m12, m13, m14,
		m21, m22, m23, m24,
		m31, m32, m33, m34,
		m41, m42, m43, m44
		);
}

void Mat44::Set(cfloat m11, cfloat m12, cfloat m13, cfloat m14, cfloat m21, cfloat m22, cfloat m23, cfloat m24, cfloat m31, cfloat m32, cfloat m33, cfloat m34, cfloat m41, cfloat m42, cfloat m43, cfloat m44)
{
	_11 = m11;
	_12 = m12;	
	_13 = m13;	
	_14 = m14;

	_21 = m21;
	_22 = m22;
	_23 = m23;
	_24 = m24;

	_31 = m31;
	_32 = m32;
	_33 = m33;
	_34 = m34;

	_41 = m41;
	_42 = m42;
	_43 = m43;
	_44 = m44;
}

float Mat44::operator()(unsigned int row, unsigned int col) const
{
	return (&_11)[row*4 + col];
}

float& Mat44::operator()(unsigned int row, unsigned int col)
{
	return (&_11)[row*4 + col];
}

//----------------------------------------------------------------------
void Mat44::SetRotation(float yaw, float pitch, float roll)
{
	const float sinY = sinf(yaw);
	const float sinP = sinf(pitch);
	const float sinR = sinf(roll);
	const float cosY = cosf(yaw);
	const float cosP = cosf(pitch);
	const float cosR = cosf(roll);


	_11 = cosR * cosY - sinR * sinP * sinY;
	_12 = sinR * cosY + cosR * sinP * sinY;
	_13 = -cosP * sinY;
	_14 = 0.f;

	_21 = -sinR * cosP;
	_22 = cosR * cosP;
	_23 = sinP;
	_24 = 0.f;

	_31 = cosR * sinY + sinR * sinP * cosY;
	_32 = sinR * sinY - cosR * sinP * cosY;
	_33 = cosP * cosY;
	_34 = 0.f;

	_41 = 0.f;
	_42 = 0.f;
	_43 = 0.f;
	_44 = 1.f;
}

float Mat44::GetDeterminant() const
{
	// return (m00*m11*m22) + (m01*m12*m20) + (m02*m10*m21) - (m02*m11*m20) - (m00*m12*m21) - (m01*m10*m22);

	//return (_11*_22*_33) + (_12*_23*_31) + (_13*_21*_32) - (_13*_22*_31) - (_11*_23*_32) - (_12*_21*_33);

	float r;
	r =	_14*_23*_32*_41 - _13*_24*_32*_41 - _14*_22*_33*_41 + _12*_24*_33*_41+
		_13*_22*_34*_41 - _12*_23*_34*_41 - _14*_23*_31*_42 + _13*_24*_31*_42+
		_14*_21*_33*_42 - _11*_24*_33*_42 - _13*_21*_34*_42 + _11*_23*_34*_42+
		_14*_22*_31*_43 - _12*_24*_31*_43 - _14*_21*_32*_43 + _11*_24*_32*_43+
		_12*_21*_34*_43 - _11*_22*_34*_43 - _13*_22*_31*_44 + _12*_23*_31*_44+
		_13*_21*_32*_44 - _11*_23*_32*_44 - _12*_21*_33*_44 + _11*_22*_33*_44;


	return r;
}

Mat44 Mat44::operator*(const Mat44& r) const
{
	Mat44 m;
	MatrixMult(*this, r, &m);

	return m;
}

Vec3 Mat44::TransformPoint(const Vec3& b) const
{
	Vec3 v;

	//v.x = m00*b.x + m01*b.y + m02* b.z + m03;
	//v.y = m10*b.x + m11*b.y + m12* b.z + m13;
	//v.z = m20*b.x + m21*b.y + m22* b.z + m23;

	v.x = b.x*_11 + b.y*_21 + b.z*_31 + _41;
	v.y = b.x*_12 + b.y*_22 + b.z*_32 + _42;
	v.z = b.x*_13 + b.y*_23 + b.z*_33 + _43;

	float w =  b.x*_14 + b.y*_24 + b.z*_34 + _44;

	v.x /= w;
	v.y /= w;
	v.z /= w;

	return v;
}

Vec4 Mat44::TransformPoint(const Vec4& in) const
{
	Vec4 out;

	out.x = _11*in.x + _21*in.y + _31*in.z + _41*in.w;
	out.y = _12*in.x + _22*in.y + _32*in.z + _42*in.w;
	out.z = _13*in.x + _23*in.y + _33*in.z + _43*in.w;
	out.w = _14*in.x + _24*in.y + _34*in.z + _44*in.w;


	return out;
}

Vec3 Mat44::TransformVector(const Vec3& b) const
{
	Vec3 out;

	float x = b.x;
	float y = b.y;
	float z = b.z;

	out.x = x*_11 + y*_21 + z*_31;
	out.y = x*_12 + y*_22 + z*_32;
	out.z = x*_13 + y*_23 + z*_33;

	return out;
}

