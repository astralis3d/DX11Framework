#pragma once

#include "mat44.h"
#include "vec3.h"

struct BoundingFrustum
{
	BoundingFrustum( const Mat44& matProjection );

	float RightSlope;           // Positive X slope (X/Z).
	float LeftSlope;            // Negative X slope.
	float TopSlope;             // Positive Y slope (Y/Z).
	float BottomSlope;          // Negative Y slope.
	float Near, Far;            // Z of the near plane and far plane.
};