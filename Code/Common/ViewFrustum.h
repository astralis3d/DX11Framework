#pragma once

#include "vec3.h"
#include "Vec4.h"

class Mat44;

__declspec(align(16))
class CViewFrustum
{
public:
	CViewFrustum() {}
	~CViewFrustum() {}

	void CalculateViewFrustum(const Mat44& viewProj);

	// Testing functions
	// true - visible
	// false - culled
	bool TestAgainstBoundingBox(const Vec3& min, const Vec3& max);

private:
	// Frustum planes 
	static const int PLANE_NEAR = 0;
	static const int PLANE_FAR = 1;
	static const int PLANE_LEFT = 2;
	static const int PLANE_RIGHT = 3;
	static const int PLANE_TOP = 4;
	static const int PLANE_BOTTOM = 5;

	// Frustum planes. Refer to them via constants
	_declspec(align(16)) Vec4	m_frustum[6];
	_declspec(align(16)) Vec3	m_planeNormal[6];
};