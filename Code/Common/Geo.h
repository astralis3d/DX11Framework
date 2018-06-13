#pragma once

#include "vec3.h"
#include "mathlib.h"
#include "mat44.h"

struct OBB
{
	// default constructor
	OBB() {}

	Vec3 c;		// center
	Vec3 h;		// half-length vector
	Mat44 m;	// orientation vectors (3x3 matrix, 9 floats, 3 vectors)

	static OBB CreateOBB(const Mat44& m, const Vec3& hlv, const Vec3& center)
	{
		OBB obb;

		obb.m = m;
		obb.h = hlv;
		obb.c = center;

		return obb;
	}
};

/*
struct AABB
{
	Vec3 min;
	Vec3 max;

	// Convert an OBB into a tight fitting AABB
	void SetAABBFromOBB(const Vec3& wpos, const OBB& obb, float scaling = 1.0f)
	{
		Vec3 pos;
		pos = obb.m.TransformPoint
	}

	static AABB CreateAABBfromOBB(const Vec3& wpos,
};
*/