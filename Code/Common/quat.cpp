#include "PCH.h"
#include "quat.h"

Quaternion::Quaternion(float EulerX, float EulerY, float EulerZ)
{
	set(EulerX, EulerY, EulerZ);
}

Quaternion::Quaternion(const Vec3& EulerXYZ)
{
	set(EulerXYZ.x, EulerXYZ.y, EulerXYZ.z);
}

//------------------------------------------------------------------------
void Quaternion::getMatrix(Mat44& outMatrix)
{
	outMatrix._11 = 1.0f - 2.0f*Y*Y - 2.0f*Z*Z;
	outMatrix._12 = 2.0f*X*Y + 2.0f * Z * W;
	outMatrix._13 = 2.0f*X*Z - 2.0f*Y*W;
	outMatrix._14 = 0.0f;

	outMatrix._21 = 2.0f*X*Y - 2.0f*Z*W;
	outMatrix._22 = 1.0f - 2.0f*X*X - 2.0f*Z*Z;
	outMatrix._23 = 2.0f*Z*Y + 2.0f*X*W;
	outMatrix._24 = 0.0f;

	outMatrix._31 = 2.0f*X*Z + 2.0f*Y*W;
	outMatrix._32 = 2.0f*Z*Y - 2.0f*X*W;
	outMatrix._33 = 1.0f - 2.0f*X*X* - 2.0f*Y*Y;
	outMatrix._34 = 0.0f;

	outMatrix._41 = outMatrix._42 = outMatrix._43 = 0.f;
	outMatrix._44 = 1.0f;
}

Quaternion Quaternion::operator+(const Quaternion& b) const
{
	return Quaternion(X + b.X, Y + b.Y, Z + b.Z, W + b.W);
}

Quaternion Quaternion::operator*(float s) const
{
	return Quaternion(s*X, s*Y, s*Z, s*W);
}

Quaternion& Quaternion::operator*=(float s)
{
	X *= s;
	Y *= s;
	Z *= s;
	W *= s;

	return *this;
}

//------------------------------------------------------------------------
Quaternion Quaternion::operator*(const Quaternion& other) const
{
	Quaternion tmp;

	tmp.W = (other.W * W) - (other.X * X) - (other.Y * Y) - (other.Z * Z);
	tmp.X = (other.W * X) + (other.X * W) + (other.Y * Z) - (other.Z * Y);
	tmp.Y = (other.W * Y) + (other.Y * W) + (other.Z * X) - (other.X * Z);
	tmp.Z = (other.W * Z) + (other.Z * W) + (other.X * Y) - (other.Y * X);

	return tmp;
}

float Quaternion::dotProduct(const Quaternion& other) const
{
	return (X * other.X) + (Y * other.Y) + (Z * other.Z) + (W * other.W);
}

Quaternion& Quaternion::set(float x, float y, float z, float w)
{
	X = x;
	Y = y;
	Z = z;
	W = w;

	return *this;
}

Quaternion& Quaternion::set(float EulerX, float EulerY, float EulerZ)
{
	float angle;

	angle = EulerX * 0.5f;
	const float sr = sinf(angle);
	const float cr = cosf(angle);

	angle = EulerY * 0.5f;
	const float sp = sinf(angle);
	const float cp = cosf(angle);

	angle = EulerZ * 0.5f;
	const float sy = sinf(angle);
	const float cy = cosf(angle);

	const float cpcy = cp * cy;
	const float spcy = sp * cy;
	const float cpsy = cp * sy;
	const float spsy = sp * sy;

	X = sr * cpcy - cr * spsy;
	Y = cr * spcy + sr * cpsy;
	Z = cr * cpsy - sr * spcy;
	W = cr * cpcy + sr * spsy;

	return normalize();
}

Quaternion& Quaternion::set(const Vec3& EulerXYZ)
{
	return set(EulerXYZ.x, EulerXYZ.y, EulerXYZ.z);
}

Quaternion& Quaternion::set(const Quaternion& other)
{

}

Quaternion& Quaternion::normalize()
{
	const float n = X*X + Y*Y + Z*Z + W*W;

	if (n == 1.0f)
		return (*this);

	// n = 1.f / sqrtf(n)

	float rcp_sqrt = 1.0f / sqrtf(n);

	return (*this *= rcp_sqrt);

}

//------------------------------------------------------------------------
Quaternion& Quaternion::slerp(Quaternion q1, Quaternion q2, float time, float threshold /*= 0.05f*/)
{
	float angle = q1.dotProduct(q2);

	// Make sure we use the short rotation
	if (angle < 0.0f)
	{
		q1 *= -1.0f;
		angle *= -1.0f;
	}

	if (angle <= (1.f - threshold))
	{
		const float theta = acosf(angle);
		const float invsintheta = 1.f / sinf(theta);
		const float scale = sinf(theta * (1.0f - time)) * invsintheta;
		const float invscale = sinf(theta * time) * invsintheta;

		return (*this = (q1*scale) + (q2*invscale));
	}
	else
	{
		return lerp(q1, q2, time);	
	}

}

//------------------------------------------------------------------------
Quaternion& Quaternion::lerp(Quaternion& q1, Quaternion& q2, float time)
{
	const float scale = 1.0f - time;

	return (*this = (q1*scale) + (q2*time));
}

//------------------------------------------------------------------------
void Quaternion::toEuler(Vec3& eulerOut) const
{

}

Quaternion& Quaternion::makeIdentity()
{
	W = 1.0f;
	X = Y = Z = 0.0f;

	return *this;
}

Quaternion& Quaternion::rotationFromTo(const Vec3& from, const Vec3& to)
{
	return *this;
}

Quaternion& Quaternion::operator=(const Quaternion& rhs)
{
	X = rhs.X;
	Y = rhs.Y;
	Z = rhs.Z;
	W = rhs.W;

	return *this;
}

bool Quaternion::operator!=(const Quaternion& rhs) const
{
	return !(*this == rhs);
}

bool Quaternion::operator==(const Quaternion& rhs) const
{
	return ( (X == rhs.X ) && (Y == rhs.Y) && (Z == rhs.Z) && (W == rhs.W) );
}
