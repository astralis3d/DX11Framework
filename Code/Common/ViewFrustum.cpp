#include "PCH.h"
#include "ViewFrustum.h"

void CViewFrustum::CalculateViewFrustum(const Mat44& viewProj)
{
	// Calculate view frustum
	m_frustum[PLANE_LEFT].x = viewProj._14 + viewProj._11;
	m_frustum[PLANE_LEFT].y = viewProj._24 + viewProj._21;
	m_frustum[PLANE_LEFT].z = viewProj._34 + viewProj._31;
	m_frustum[PLANE_LEFT].w = viewProj._44 + viewProj._41;

	m_frustum[PLANE_RIGHT].x = viewProj._14 - viewProj._11;
	m_frustum[PLANE_RIGHT].y = viewProj._24 - viewProj._21;
	m_frustum[PLANE_RIGHT].z = viewProj._34 - viewProj._31;
	m_frustum[PLANE_RIGHT].w = viewProj._44 - viewProj._41;

	m_frustum[PLANE_TOP].x = viewProj._14 - viewProj._12;
	m_frustum[PLANE_TOP].y = viewProj._24 - viewProj._22;
	m_frustum[PLANE_TOP].z = viewProj._34 - viewProj._32;
	m_frustum[PLANE_TOP].w = viewProj._44 - viewProj._42;

	m_frustum[PLANE_BOTTOM].x = viewProj._14 + viewProj._12;
	m_frustum[PLANE_BOTTOM].y = viewProj._24 + viewProj._22;
	m_frustum[PLANE_BOTTOM].z = viewProj._34 + viewProj._32;
	m_frustum[PLANE_BOTTOM].w = viewProj._44 + viewProj._42;

	m_frustum[PLANE_NEAR].x = viewProj._14 + viewProj._13;
	m_frustum[PLANE_NEAR].y = viewProj._24 + viewProj._23;
	m_frustum[PLANE_NEAR].z = viewProj._34 + viewProj._33;
	m_frustum[PLANE_NEAR].w = viewProj._44 + viewProj._43;

	m_frustum[PLANE_FAR].x = viewProj._14 - viewProj._13;
	m_frustum[PLANE_FAR].y = viewProj._24 - viewProj._23;
	m_frustum[PLANE_FAR].z = viewProj._34 - viewProj._33;
	m_frustum[PLANE_FAR].w = viewProj._44 - viewProj._43;


	// Normalize each plane
	for (int i = 0; i < 6; ++i)
	{
		Vec4& v = m_frustum[i];
		Vec3& p = m_planeNormal[i];

		const float Length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
		const float rcpLength = 1.f / Length;

		v.x *= rcpLength;
		v.y *= rcpLength;
		v.z *= rcpLength;
		v.w *= rcpLength;

		p.x = v.x;
		p.y = v.y;
		p.z = v.z;
	}
}

//----------------------------------------------------------------------
bool CViewFrustum::TestAgainstBoundingBox(const Vec3& min, const Vec3& max)
{
	// Test for each plane

	const Vec3 center = (max + min) * 0.5f; // c
	const Vec3 extent = (max - min) * 0.5f; // h

	for (int i = 0; i < 6; ++i)
	{
		const Vec3& planeNormal = m_planeNormal[i];

		const float dpr = Vec3Dot(center, planeNormal) + extent.x * abs(planeNormal.x) + extent.y * abs(planeNormal.y) + extent.z * abs(planeNormal.z);

		if (dpr + m_frustum[i].w < 0.f)
			return false;
	}

	return true;
}