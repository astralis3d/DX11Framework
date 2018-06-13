#include "PCH.h"
#include <cmath>

#include "mathlib.h"

//----------------------------------------------------------------------
Mat44* MatrixLookAtLH(const Vec3& eyePos, const Vec3& focusPos, const Vec3& upDir, Mat44* pOut)
{
	Vec3 zAxis = focusPos - eyePos;
	zAxis.Normalize();

	Vec3 xAxis;
	Vec3Cross(&xAxis, upDir, zAxis);
	xAxis.Normalize();

	Vec3 yAxis;
	Vec3Cross(&yAxis, zAxis, xAxis);

	pOut->Set(	xAxis.x,					yAxis.x,					zAxis.x,					0.f,
				xAxis.y,					yAxis.y,					zAxis.y,					0.f,
				xAxis.z,					yAxis.z,					zAxis.z,					0.f,
				-Vec3Dot(xAxis, eyePos),	-Vec3Dot(yAxis, eyePos),	-Vec3Dot(zAxis, eyePos),	1.f	
		);

	return pOut;
}

//------------------------------------------------------------------------
Mat44* MatrixLookAtRH( const Vec3& eyePos, const Vec3& focusPos, const Vec3& upDir, Mat44* pOut )
{
	Vec3 zAxis = eyePos - focusPos;
	zAxis.Normalize();

	Vec3 xAxis;
	Vec3Cross(&xAxis, upDir, zAxis);
	xAxis.Normalize();

	Vec3 yAxis;
	Vec3Cross(&yAxis, zAxis, xAxis);

	pOut->Set(	xAxis.x,					yAxis.x,					zAxis.x,					0.f,
				xAxis.y,					yAxis.y,					zAxis.y,					0.f,
				xAxis.z,					yAxis.z,					zAxis.z,					0.f,
				-Vec3Dot(xAxis, eyePos),	-Vec3Dot(yAxis, eyePos),	-Vec3Dot(zAxis, eyePos),	1.f	
		);

	return pOut;
}

//----------------------------------------------------------------------
void MatrixMult(const Mat44& a, const Mat44& b, Mat44* pOut)
{
	// Algorithm:
	//
	// P11 = dot (A.row1, B.col1)	P12 = dot (A.row1, B.col2)
	// P21 = dot (A.row2, B.col1)	P22 = dot (A.row2, B.col2)
	//
	//... and so on


	pOut->_11 = a._11*b._11 + a._12*b._21 + a._13*b._31 + a._14*b._41;
	pOut->_12 = a._11*b._12 + a._12*b._22 + a._13*b._32 + a._14*b._42;
	pOut->_13 = a._11*b._13 + a._12*b._23 + a._13*b._33 + a._14*b._43;
	pOut->_14 = a._11*b._14 + a._12*b._24 + a._13*b._34 + a._14*b._44;

	pOut->_21 = a._21*b._11 + a._22*b._21 + a._23*b._31 + a._24*b._41;
	pOut->_22 = a._21*b._12 + a._22*b._22 + a._23*b._32 + a._24*b._42;
	pOut->_23 = a._21*b._13 + a._22*b._23 + a._23*b._33 + a._24*b._43;
	pOut->_24 = a._21*b._14 + a._22*b._24 + a._23*b._34 + a._24*b._44;

	pOut->_31 = a._31*b._11 + a._32*b._21 + a._33*b._31 + a._34*b._41;
	pOut->_32 = a._31*b._12 + a._32*b._22 + a._33*b._32 + a._34*b._42;
	pOut->_33 = a._31*b._13 + a._32*b._23 + a._33*b._33 + a._34*b._43;
	pOut->_34 = a._31*b._14 + a._32*b._24 + a._33*b._34 + a._34*b._44;

	pOut->_41 = a._41*b._11 + a._42*b._21 + a._43*b._31 + a._44*b._41;
	pOut->_42 = a._41*b._12 + a._42*b._22 + a._43*b._32 + a._44*b._42;
	pOut->_43 = a._41*b._13 + a._42*b._23 + a._43*b._33 + a._44*b._43;
	pOut->_44 = a._41*b._14 + a._42*b._24 + a._43*b._34 + a._44*b._44;
}

//----------------------------------------------------------------------
Mat44* MatrixScaling(Mat44* pOut, float sx = 1.f, float sy = 1.f, float sz = 1.f)
{
	pOut->Set(	sx,		0.f,	0.f,	0.f,
				0.f,	sy,		0.f,	0.f,
				0.f,	0.f,	sz,		0.f,
				0.f,	0.f,	0.f,	1.f);

	return pOut;
}

//----------------------------------------------------------------------
Vec3* Vec3Cross(Vec3* pOut, const Vec3& a, const Vec3& b)
{
	*pOut = a.GetCrossProduct(b);

	return pOut;
}

//----------------------------------------------------------------------
float Vec3Dot(const Vec3& a, const Vec3& b)
{
	return a.GetDotProduct(b);
}

//----------------------------------------------------------------------
Mat44* MatrixPerspectiveFovLH(float fov, float aspectRatio, float zn, float zf, Mat44* pOut)
{
	// yScale = cot(fov / 2) = tan(PI/2 - fov/2)
	const float yScale = 1.f / tan((fov/2.f));
	const float xScale = yScale / aspectRatio;

	pOut->Set(
		xScale,		0.f,		0.f,			0.f,
		0.f,		yScale,		0.f,			0.f,
		0.f,		0.f,		zf/(zf-zn),		1.f,
		0.f,		0.f,		-zn*zf/(zf-zn),	0.f);

	return pOut;
}

//------------------------------------------------------------------------
Mat44* MatrixPerspectiveFovRH( float fov, float aspectRatio, float zn, float zf, Mat44* pOut )
{
	// yScale = cot(fov / 2) = tan(PI/2 - fov/2)
	const float yScale = 1.f / tan((fov/2.f));
	const float xScale = yScale / aspectRatio;

	pOut->Set(
		xScale,		0.f,		0.f,			0.f,
		0.f,		yScale,		0.f,			0.f,
		0.f,		0.f,		zf/(zn-zf),		-1.f,
		0.f,		0.f,		zn*zf/(zn-zf),	0.f);

	return pOut;
}

//----------------------------------------------------------------------
Mat44* MatrixRotationYawPitchRoll(const float yaw, const float pitch, const float roll, Mat44* pOut)
{
	const float sinY = sinf(yaw);
	const float sinP = sinf(pitch);
	const float sinR = sinf(roll);
	const float cosY = cosf(yaw);
	const float cosP = cosf(pitch);
	const float cosR = cosf(roll);


	pOut->_11 = cosR * cosY - sinR * sinP * sinY;
	pOut->_12 = sinR * cosY + cosR * sinP * sinY;
	pOut->_13 = -cosP * sinY;
	pOut->_14 = 0.f;

	pOut->_21 = -sinR * cosP;
	pOut->_22 = cosR * cosP;
	pOut->_23 = sinP;
	pOut->_24 = 0.f;

	pOut->_31 = cosR * sinY + sinR * sinP * cosY;
	pOut->_32 = sinR * sinY - cosR * sinP * cosY;
	pOut->_33 = cosP * cosY;
	pOut->_34 = 0.f;

	pOut->_41 = 0.f;
	pOut->_42 = 0.f;
	pOut->_43 = 0.f;
	pOut->_44 = 1.f;


	return pOut;
}

//----------------------------------------------------------------------
void MatrixIdentity(Mat44* pOut)
{
	pOut->Set(	1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
}

//----------------------------------------------------------------------
Mat44* MatrixTranspose(const Mat44& m, Mat44* pOut)
{
	// Transpose - replace rows with columns

	Mat44 tmp;

	tmp._11 = m._11;		tmp._12 = m._21;		tmp._13 = m._31;		tmp._14 = m._41;
	tmp._21 = m._12;		tmp._22 = m._22;		tmp._23 = m._32;		tmp._24 = m._42;
	tmp._31 = m._13;		tmp._32 = m._23;		tmp._33 = m._33;		tmp._34 = m._43;
	tmp._41 = m._14;		tmp._42 = m._24;		tmp._43 = m._34;		tmp._44 = m._44;


	*pOut = tmp;

	return pOut;
}

Mat44* MatrixInverse(const Mat44& m, Mat44* pOut)
{
	// Taken from CryEngine / CryCommon (Cry_Matrix.h)

	Mat44 mtmp;
	float tmp[12];

	// Calculate pairs for first 8 elements (cofactors)
	tmp[0] = m._33 * m._44;
	tmp[1] = m._43 * m._34;
	tmp[2] = m._23 * m._44;
	tmp[3] = m._43 * m._24;
	tmp[4] = m._23 * m._34;
	tmp[5] = m._33 * m._24;
	tmp[6] = m._13 * m._44;
	tmp[7] = m._43 * m._14;
	tmp[8] = m._13 * m._34;
	tmp[9] = m._33 * m._14;
	tmp[10] = m._13 * m._24;
	tmp[11] = m._23 * m._14;

	// Calculate first 8 elements (cofactors)
	mtmp._11 = tmp[0]*m._22 + tmp[3]*m._32 + tmp[4]*m._42;
	mtmp._11-= tmp[1]*m._22 + tmp[2]*m._32 + tmp[5]*m._42;
	mtmp._12 = tmp[1]*m._12 + tmp[6]*m._32 + tmp[9]*m._42;
	mtmp._12-= tmp[0]*m._12 + tmp[7]*m._32 + tmp[8]*m._42;
	mtmp._13 = tmp[2]*m._12 + tmp[7]*m._22 + tmp[10]*m._42;
	mtmp._13-= tmp[3]*m._12 + tmp[6]*m._22 + tmp[11]*m._42;
	mtmp._14 = tmp[5]*m._12 + tmp[8]*m._22 + tmp[11]*m._32;
	mtmp._14-= tmp[4]*m._12 + tmp[9]*m._22 + tmp[10]*m._32;
	mtmp._21 = tmp[1]*m._21 + tmp[2]*m._31 + tmp[5]*m._41;
	mtmp._21-= tmp[0]*m._21 + tmp[3]*m._31 + tmp[4]*m._41;
	mtmp._22 = tmp[0]*m._11 + tmp[7]*m._31 + tmp[8]*m._41;
	mtmp._22-= tmp[1]*m._11 + tmp[6]*m._31 + tmp[9]*m._41;
	mtmp._23 = tmp[3]*m._11 + tmp[6]*m._21 + tmp[11]*m._41;
	mtmp._23-= tmp[2]*m._11 + tmp[7]*m._21 + tmp[10]*m._41;
	mtmp._24 = tmp[4]*m._11 + tmp[9]*m._21 + tmp[10]*m._31;
	mtmp._24-= tmp[5]*m._11 + tmp[8]*m._21 + tmp[11]*m._31;

	// Calculate pairs for second 8 elements (cofactors)

	tmp[ 0] = m._31*m._42;
	tmp[ 1] = m._41*m._32;
	tmp[ 2] = m._21*m._42;
	tmp[ 3] = m._41*m._22;
	tmp[ 4] = m._21*m._32;
	tmp[ 5] = m._31*m._22;
	tmp[ 6] = m._11*m._42;
	tmp[ 7] = m._41*m._12;
	tmp[ 8] = m._11*m._32;
	tmp[ 9] = m._31*m._12;
	tmp[10] = m._11*m._22;
	tmp[11] = m._21*m._12;

	// Calculate second 8 elements

	mtmp._31 = tmp[ 0]*m._24 + tmp[ 3]*m._34 + tmp[ 4]*m._44;
	mtmp._31-= tmp[ 1]*m._24 + tmp[ 2]*m._34 + tmp[ 5]*m._44;
	mtmp._32 = tmp[ 1]*m._14 + tmp[ 6]*m._34 + tmp[ 9]*m._44;
	mtmp._32-= tmp[ 0]*m._14 + tmp[ 7]*m._34 + tmp[ 8]*m._44;
	mtmp._33 = tmp[ 2]*m._14 + tmp[ 7]*m._24 + tmp[10]*m._44;
	mtmp._33-= tmp[ 3]*m._14 + tmp[ 6]*m._24 + tmp[11]*m._44;
	mtmp._34 = tmp[ 5]*m._14 + tmp[ 8]*m._24 + tmp[11]*m._34;
	mtmp._34-= tmp[ 4]*m._14 + tmp[ 9]*m._24 + tmp[10]*m._34;
	mtmp._41 = tmp[ 2]*m._33 + tmp[ 5]*m._43 + tmp[ 1]*m._23;
	mtmp._41-= tmp[ 4]*m._43 + tmp[ 0]*m._23 + tmp[ 3]*m._33;
	mtmp._42 = tmp[ 8]*m._43 + tmp[ 0]*m._13 + tmp[ 7]*m._33;
	mtmp._42-= tmp[ 6]*m._33 + tmp[ 9]*m._43 + tmp[ 1]*m._13;
	mtmp._43 = tmp[ 6]*m._23 + tmp[11]*m._43 + tmp[ 3]*m._13;
	mtmp._43-= tmp[10]*m._43 + tmp[ 2]*m._13 + tmp[ 7]*m._23;
	mtmp._44 = tmp[10]*m._33 + tmp[ 4]*m._13 + tmp[ 9]*m._23;
	mtmp._44-= tmp[ 8]*m._23 + tmp[11]*m._33 + tmp[ 5]*m._13;

	// Calculate determinant
	const float det = m._11*mtmp._11 + m._21*mtmp._12 + m._31*mtmp._13 + m._41*mtmp._14;
	const float oodet = 1.f / det;


	pOut->_11 = mtmp._11*oodet;		pOut->_12 = mtmp._12*oodet;		pOut->_13 = mtmp._13*oodet;		pOut->_14 = mtmp._14*oodet;
	pOut->_21 = mtmp._21*oodet;		pOut->_22 = mtmp._22*oodet;		pOut->_23 = mtmp._23*oodet;		pOut->_24 = mtmp._24*oodet;
	pOut->_31 = mtmp._31*oodet;		pOut->_32 = mtmp._32*oodet;		pOut->_33 = mtmp._33*oodet;		pOut->_34 = mtmp._34*oodet;
	pOut->_41 = mtmp._41*oodet;		pOut->_42 = mtmp._42*oodet;		pOut->_43 = mtmp._43*oodet;		pOut->_44 = mtmp._44*oodet;



	return pOut;
}

Mat44* MatrixTranslation(float x, float y, float z, Mat44* pOut)
{
	pOut->Set(	1.f,	0.f,	0.f,	0.f,
				0.f,	1.f,	0.f,	0.f,
				0.f,	0.f,	1.f,	0.f,
				x,		y,		z,		1.f);

	return pOut;
}

Mat44* MatrixTranslation(const Vec3& v, Mat44* pOut)
{
	return MatrixTranslation(v.x, v.y, v.z, pOut);
}

Mat44* MatrixRotationX(float ang, Mat44* pOut)
{
	const float fSinAngle = sinf(ang);
	const float fCosAngle = cosf(ang);

	pOut->_11 = 1.0f;
	pOut->_12 = 0.0f;
	pOut->_13 = 0.0f;
	pOut->_14 = 0.0f;
		  
	pOut->_21 = 0.0f;
	pOut->_22 = fCosAngle;
	pOut->_23 = fSinAngle;
	pOut->_24 = 0.0f;
		  
	pOut->_31 = 0.0f;
	pOut->_32 = -fSinAngle;
	pOut->_33 = fCosAngle;
	pOut->_34 = 0.0f;
		  
	pOut->_41 = 0.0f;
	pOut->_42 = 0.0f;
	pOut->_43 = 0.0f;
	pOut->_44 = 1.0f;


	return pOut;
}

Mat44* MatrixRotationY(float ang, Mat44* pOut)
{
	const float fSinAngle = sinf(ang);
	const float fCosAngle = cosf(ang);

	pOut->_11 = fCosAngle;
	pOut->_12 = 0.0f;
	pOut->_13 = -fSinAngle;
	pOut->_14 = 0.0f;

	pOut->_21 = 0.0f;
	pOut->_22 = 1.0f;
	pOut->_23 = 0.0f;
	pOut->_24 = 0.0f;

	pOut->_31 = fSinAngle;
	pOut->_32 = 0.0f;
	pOut->_33 = fCosAngle;
	pOut->_34 = 0.0f;

	pOut->_41 = 0.0f;
	pOut->_42 = 0.0f;
	pOut->_43 = 0.0f;
	pOut->_44 = 1.0f;


	return pOut;
}

Mat44* MatrixRotationZ(float ang, Mat44* pOut)
{
	const float fSinAngle = sinf(ang);
	const float fCosAngle = cosf(ang);

	pOut->_11 = fCosAngle;
	pOut->_12 = fSinAngle;
	pOut->_13 = 0.0f;
	pOut->_14 = 0.0f;

	pOut->_21 = -fSinAngle;
	pOut->_22 = fCosAngle;
	pOut->_23 = 0.0f;
	pOut->_24 = 0.0f;

	pOut->_31 = 0.0f;
	pOut->_32 = 0.0f;
	pOut->_33 = 1.0f;
	pOut->_34 = 0.0f;

	pOut->_41 = 0.0f;
	pOut->_42 = 0.0f;
	pOut->_43 = 0.0f;
	pOut->_44 = 1.0f;


	return pOut;
}

//----------------------------------------------------------------------
void MatrixInvertedTranspose(const Mat44& m, Mat44* pOut)
{
	Mat44 mTmp = m;
	MatrixInverse(m, &mTmp);

	MatrixTranspose(mTmp, pOut);
}

//----------------------------------------------------------------------
Mat44* MatrixOrtographicLH(float w, float h, float zn, float zf, Mat44* pOut)
{
	pOut->Set(	
		2.f/w,	0.f,	0.f,			0.f,
		0.f,	2.f/h,	0.f,			0.f,
		0.f,	0.f,	1.f/(zf-zn),	0.f,
		0.f,	0.f,	zn/(zn-zf)	,	1.f);

	return pOut;
}

//----------------------------------------------------------------------
Vec3* Vec3Min(const Vec3& a, const Vec3& b, Vec3* pOut)
{
	Vec3 tmp;
	tmp.x = ceMin<float>(a.x, b.x);
	tmp.y = ceMin<float>(a.y, b.y);
	tmp.z = ceMin<float>(a.z, b.z);

	pOut->x = tmp.x;
	pOut->y = tmp.y;
	pOut->z = tmp.z;

	return pOut;
}

//----------------------------------------------------------------------
Vec3* Vec3Max(const Vec3& a, const Vec3& b, Vec3* pOut)
{
	Vec3 tmp;
	tmp.x = ceMax<float>(a.x, b.x);
	tmp.y = ceMax<float>(a.y, b.y);
	tmp.z = ceMax<float>(a.z, b.z);

	pOut->x = tmp.x;
	pOut->y = tmp.y;
	pOut->z = tmp.z;

	return pOut;
}

//------------------------------------------------------------------------
Mat44* MatrixOrthographicOffCenterLH(float l, float r, float b, float t, float n, float f, Mat44* pOut)
{
	pOut->Set(	2.f / (r - l),	0.f,		0.f,		0.f,
				0.f,			2.f/(t-b),	0.f,		0.f,
				0.f,			0.f,		1.f/(f-n),	0.f,
				(l+r)/(l-r),	(t+b)/(b-t),n/(n-f),	1.f
				);

	return pOut;
}

//------------------------------------------------------------------------
void FindAABBFromPoints(Vec3* points, unsigned int n, Vec3* pOutMin, Vec3* pOutMax)
{
	*pOutMin = *pOutMax = points[0];

	for (unsigned int i = 1; i < n; ++i)
	{
		// min
		if (points[i].x < pOutMin->x)
			pOutMin->x = points[i].x;
		if (points[i].y < pOutMin->y)
			pOutMin->y = points[i].y;
		if (points[i].z < pOutMin->z)
			pOutMin->z = points[i].z;

		// max
		if (points[i].x > pOutMax->x)
			pOutMax->x = points[i].x;
		if (points[i].y > pOutMax->y)
			pOutMax->y = points[i].y;
		if (points[i].z > pOutMax->z)
			pOutMax->z = points[i].z;
	}
}

Vec3* Vec3Mult( const Vec3& a, const Vec3& b, Vec3* pOut )
{
	pOut->x = a.x * b.x;
	pOut->y = a.y * b.y;
	pOut->z = a.z * b.z;

	return pOut;
}

Vec4* Vec4Mult( const Vec4& a, const Vec4& b, Vec4* pOut )
{
	pOut->x = a.x * b.x;
	pOut->y = a.y * b.y;
	pOut->z = a.z * b.z;
	pOut->w = a.w * b.w;

	return pOut;
}
