#ifndef __ceMathlib_h__
#define __ceMathlib_h__

#pragma once

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// citizengine Math Library
//----------------------------------------------------------------------
//----------------------------------------------------------------------

#define PI		(3.141592f)
#define PI2		(2.f*PI)
#define PIHALF	(PI/2.f)

#include <cmath>


#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Mat44.h"
#include "quat.h"

static const float RADIAN = PI / 180.f;
static const float DEGREE = 180.0f / PI;

inline float DegToRad(float degrees)
{
	return degrees * RADIAN;
}

inline float RadToDeg(float radian)
{
	return radian * DEGREE;
}

inline float RandFloat(float a, float b)
{
	return a + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (b - a)));
}

inline float lerp(float x, float y, float s)
{
	return x + s*(y - x);
}

//---------------------------------------------------------------------
// Vectors
//----------------------------------------------------------------------

Vec3* Vec3Cross(Vec3* pOut, const Vec3& a, const Vec3& b);
float Vec3Dot(const Vec3& a, const Vec3& b);
Vec3* Vec3Min(const Vec3& a, const Vec3& b, Vec3* pOut);
Vec3* Vec3Max(const Vec3& a, const Vec3& b, Vec3* pOut);
Vec3* Vec3Mult(const Vec3& a, const Vec3& b, Vec3* pOut);
Vec4* Vec4Mult(const Vec4& a, const Vec4& b, Vec4* pOut);

void FindAABBFromPoints(Vec3* points, unsigned int n, Vec3* pOutMin, Vec3* pOutMax);


//----------------------------------------------------------------------
// Matrices
//----------------------------------------------------------------------

// Identity matrix
void MatrixIdentity(Mat44* pOut);

// Matrix - Matrix multiplication
void MatrixMult(const Mat44& a, const Mat44& b, Mat44* pOut);

// Transposition (replace row with columns)
Mat44* MatrixTranspose(const Mat44& m, Mat44* pOut);
Mat44* MatrixInverse(const Mat44& m, Mat44* pOut);

void MatrixInvertedTranspose(const Mat44& m, Mat44* pOut);

// Various
Mat44* MatrixRotationYawPitchRoll(const float yaw, const float pitch, const float roll, Mat44* pOut);
Mat44* MatrixScaling(Mat44* pOut, float sx, float sy, float sz);
Mat44* MatrixTranslation(float x, float y, float z, Mat44* pOut);
Mat44* MatrixTranslation(const Vec3& v, Mat44* pOut);
Mat44* MatrixRotationX(float ang, Mat44* pOut);
Mat44* MatrixRotationY(float ang, Mat44* pOut);
Mat44* MatrixRotationZ(float ang, Mat44* pOut);

Mat44* MatrixLookAtLH(const Vec3& eyePos, const Vec3& focusPos, const Vec3& upDir, Mat44* pOut);
Mat44* MatrixLookAtRH( const Vec3& eyePos, const Vec3& focusPos, const Vec3& upDir, Mat44* pOut );
Mat44* MatrixPerspectiveFovLH(float fov, float aspectRatio, float zNear, float zFar, Mat44* pOut);
Mat44* MatrixPerspectiveFovRH(float fov, float aspectRatio, float zNear, float zFar, Mat44* pOut);
Mat44* MatrixOrthographicOffCenterLH(float l, float r, float b, float t, float n, float f, Mat44* pOut);
Mat44* MatrixOrtographicLH(float w, float h, float zn, float zf, Mat44* pOut);



template <typename T>
int signum(T val)
{
	if (val == (T) 0)
		return 0;
	else
		return ((val > (T) 0)) ? 1 : -1;
}


template <typename T>
T ceMin(const T&a, const T& b)
{
	return (a < b) ? a : b;
}

template <typename T>
T ceMax(const T&a, const T& b)
{
	return (a > b) ? a : b;
}


struct Sphere
{
	Vec3 center;
	float radius;
};



//----------------------------------------------------------------------
// Various intersections
//----------------------------------------------------------------------

#endif