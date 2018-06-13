#ifndef __ICamera_h__
#define __ICamera_h__

#pragma once

// Don't include whole math lib. In this case just Mat44 & Vec3 are fine.
#include "Mat44.h"
#include "Vec3.h"


class CInput;

class ICamera
{
public:
	ICamera() {}
	ICamera(float posX, float posY, float posZ, float movSpeed, float rotSpeed, float yaw, float pitch);
	virtual ~ICamera() {}

	//----------------------------------------------------------------------
	// camera-specific implementations
	//----------------------------------------------------------------------
	virtual void HandleMousemove() = 0;
	virtual void HandleInput(CInput* input, float dt) = 0;
	virtual void UpdateMatrixView(float dt) = 0;

	virtual void OnRMBDown() = 0;
	virtual void OnRMBUp() = 0;
	
	void UpdateMatrixProj();

	void ResetCamera(float x, float y, float z, float yaw, float pitch);

	const Mat44& GetMatrixView() const { return m_MatView; }
	const Mat44& GetMatrixProj() const { return m_MatProj; }
	const Mat44& GetMatrixViewProj() const { return m_MatViewProj; }

	// trivial getters and setters for particular camera's position
	void SetX(float x)			{ m_pos.x = x; }
	void SetY(float y)			{ m_pos.y = y; }
	void SetZ(float z)			{ m_pos.z = z; }
	void SetPos(const Vec3& p)	{ m_pos = p; }

	void SetClippingDistances(float fNear, float fFar) { m_fClipNear = fNear; m_fClipFar = fFar; UpdateMatrixProj(); }
	void GetClippingDistances(float& fNear, float& fFar) { fNear = m_fClipNear; fFar = m_fClipFar; }
	void SetFOV(float degrees)	{ m_fFOV = degrees; UpdateMatrixProj(); }

	void SetOrtho(bool b)		{ m_bIsOrtho = b; UpdateMatrixProj(); }
	bool IsOrtho() const		{ return m_bIsOrtho; }

	void SetDimensions(float width, float height)
	{
		m_fWidth = width;
		m_fHeight = height;
		m_fAspectRatio = (width/height);

		UpdateMatrixProj();
	}

	void SetSpeedMove(float f)	{ m_camSpeedMove = f; }
	void SetSpeedRot(float f)	{ m_camSpeedRot = f; }

	void SetYaw(float f)		{ m_fCamYaw = f; }
	void SetPitch(float f)		{ m_fCamPitch = f; }

	void SetLMB(bool flag)		{ m_bLMB = flag; }
	void SetRMB(bool flag)		{ m_bRMB = flag; }
	void SetDragging(bool flag)	{ m_bDragging = flag; }

	void SetHWND(void* hwnd)	{ m_hWnd = hwnd; }

	/////////////////////////////////////////////

	const Vec3& GetPosition() const	{ return m_pos; }
	Vec3& GetPosition()	{ return m_pos; }

	const Vec3& GetFocus() const { return m_focus; }

	Vec3 GetDirection() const  { return Vec3(m_MatView._13, m_MatView._23, m_MatView._33); }
	

	float GetSpeedMove() const	{ return m_camSpeedMove; }
	float GetSpeedRot() const	{ return m_camSpeedRot; }

	float GetYaw() const		{ return m_fCamYaw; }
	float GetPitch() const		{ return m_fCamPitch; }
	float GetFOV() const		{ return m_fFOV; }
	float GetAspect() const		{ return m_fAspectRatio; }

	bool IsLMB() const			{ return m_bLMB; }
	bool IsRMB() const			{ return m_bRMB; }
	bool IsDragging() const		{ return m_bDragging; }

	bool IsChanged() const		{ return m_bChange; }

protected:
	Mat44	m_MatView;
	Mat44	m_MatProj;
	Mat44	m_MatViewProj;

	Vec3	m_pos;
	Vec3	m_focus;

	bool	m_bChange;

	// Rotation
	float	m_fCamYaw;
	float	m_fCamPitch;

	float	m_fFOV;
	float	m_fAspectRatio;
	float	m_fClipNear;
	float	m_fClipFar;

	bool	m_bIsOrtho;

	float	m_fWidth;
	float	m_fHeight;

	void*	m_hWnd;

private:
	float	m_camSpeedMove;
	float	m_camSpeedRot;

	bool	m_bLMB;
	bool	m_bRMB;
	bool	m_bDragging;
};

inline ICamera::ICamera(float posX, float posY, float posZ, float movSpeed, float rotSpeed, float yaw, float pitch)
	: m_pos(posX, posY, posZ)
	, m_camSpeedMove(movSpeed)
	, m_camSpeedRot(rotSpeed)
	, m_fCamYaw(yaw)
	, m_fCamPitch(pitch)
	, m_fFOV(75.f)
	, m_fClipNear(0.1f)
	, m_fClipFar(1000.f)
	, m_bIsOrtho(false)
	, m_fAspectRatio(1.777778f)
	, m_bLMB(false)
	, m_bRMB(false)
	, m_bDragging(false)
{

}

//----------------------------------------------------------------------
inline void ICamera::ResetCamera(float x, float y, float z, float yaw, float pitch)
{
	SetPos(Vec3(x, y, z));

	m_fCamPitch = pitch;
	m_fCamYaw = yaw;

	m_bChange = true;

	UpdateMatrixView(0);
}

//----------------------------------------------------------------------
inline void ICamera::UpdateMatrixProj()
{
	// Calculate projection matrix.
	// Rather shouldn't be called every frame :)
	if (m_bIsOrtho)
		MatrixOrtographicLH(m_fWidth, m_fHeight, m_fClipNear, m_fClipFar, &m_MatProj);
	else
		MatrixPerspectiveFovLH(DegToRad(m_fFOV), m_fAspectRatio, m_fClipNear, m_fClipFar, &m_MatProj);


	// Calculate again viewProj
	MatrixMult(m_MatView, m_MatProj, &m_MatViewProj);
}


#endif