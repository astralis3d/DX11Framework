#include "PCH.h"
#include "BoundingFrustum.h"
#include "vec4.h"

//------------------------------------------------------------------------
// Creates a frustum from perspective matrix.
//------------------------------------------------------------------------
BoundingFrustum::BoundingFrustum( const Mat44& matProjection )
{
	// Corners of the projection frustum in homogenous space.
	static Vec4 HomogenousPoints[6] =
	{
		{ 1.0f,  0.0f, 1.0f, 1.0f },   // right (at far plane)
		{ -1.0f,  0.0f, 1.0f, 1.0f },   // left
		{ 0.0f,  1.0f, 1.0f, 1.0f },   // top
		{ 0.0f, -1.0f, 1.0f, 1.0f },   // bottom

		{ 0.0f, 0.0f, 0.0f, 1.0f },     // near
		{ 0.0f, 0.0f, 1.0f, 1.0f }      // far
	};

	Mat44 matInverse;
	MatrixInverse(matProjection, &matInverse);

	// Compute frustum corners in world space
	Vec4 points[6];
	for (int i=0; i < 6; ++i)
	{
		points[i] = matInverse.TransformPoint( HomogenousPoints[i] );
	}

	// Compute the slopes
	for (int i=0; i <= 3; ++i)
	{
		const float rcpOfZ = 1.f / points[i].z;
		Vec4Mult( points[i], Vec4( rcpOfZ, rcpOfZ, rcpOfZ, rcpOfZ ), &points[i] );
	}

	RightSlope = points[0].x;
	LeftSlope = points[1].x;
	TopSlope = points[2].y;
	BottomSlope = points[3].y;
	
	for (int i = 4; i <= 5; ++i)
	{
		const float rcpOfW = 1.f / points[i].w;
		Vec4Mult( points[i], Vec4( rcpOfW, rcpOfW, rcpOfW, rcpOfW ), &points[i] );
	}

	Near = points[4].z;
	Far = points[5].z;
}
