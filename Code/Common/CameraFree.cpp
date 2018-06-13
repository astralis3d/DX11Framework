#include "PCH.h"
#include "CameraFree.h"
#include "Input.h"

Vec3 interpolate(const Vec3& a, const Vec3& b, float alpha)
{
	Vec3 vOut;

	vOut.x = a.x * alpha + b.x * (1.f - alpha);
	vOut.y = a.y * alpha + b.y * (1.f - alpha);
	vOut.z = a.z * alpha + b.z * (1.f - alpha);

	return vOut;
}

namespace NMCameraEditor
{
	POINT	m_curPosBeginDrag;
	POINT	pt;

	int		 prevMouseX, prevMouseY;

	float	m_camSpeedStart;
}


CCameraFree::CCameraFree()
{
	memset(this, 0, sizeof(CCameraFree));
}

CCameraFree::CCameraFree(float posX, float posY, float posZ, float movSpeed, float rotSpeed, float yaw, float pitch)
	: ICamera(posX, posY, posZ, movSpeed, rotSpeed, yaw, pitch)
	, m_speedIncreases(false)	
{
	NMCameraEditor::m_camSpeedStart = movSpeed;

	UpdateMatrixView(0);
}

//------------------------------------------------------------------------
CCameraFree::CCameraFree(const f32 yaw, const f32 pitch)
	: ICamera(0.0f, 0.0f, 0.0f, 10.0f, 0.5f, yaw, pitch)
{
	NMCameraEditor::m_camSpeedStart = 0.5f;
	UpdateMatrixView(0);
}

void CCameraFree::HandleMousemove()
{
	using namespace NMCameraEditor;

	static int diff_x, diff_y;

	RECT rect;	
	GetClientRect((HWND)m_hWnd, &rect);
	const int height = rect.bottom - rect.top;
	const int width = rect.right - rect.left;



	
	GetCursorPos(&pt);
	ScreenToClient((HWND)m_hWnd, &pt);


	// Only when RMB is being holded
	if (IsRMB())
	{
		/*
		if (pt.x < 25 || pt.x > (rect.right - 25) )
		{
			POINT newpt;
			newpt.x = width / 2;
			newpt.y = pt.y;

			ClientToScreen((HWND)m_hWnd, &newpt);
			SetCursorPos(newpt.x, newpt.y);
		}
		else if (pt.y < 25 || pt.y > (rect.bottom - 25) )
		{
			POINT newpt;
			newpt.x = pt.x;
			newpt.y = height / 2;

			ClientToScreen((HWND)m_hWnd, &newpt);
			SetCursorPos(newpt.x, newpt.y);
		}
		*/

		POINT newpt;
		

		if (pt.x < 40)
		{
			newpt.x = rect.right - 40;
			newpt.y = pt.y;			

			ClientToScreen((HWND)m_hWnd, &newpt);
			SetCursorPos(newpt.x, newpt.y);
		}
		else if (pt.x > (rect.right - 40))
		{
			newpt.x = 40;
			newpt.y = pt.y;

			ClientToScreen((HWND)m_hWnd, &newpt);
			SetCursorPos(newpt.x, newpt.y);
		}

		else if (pt.y < 40 )
		{
			newpt.x = pt.x;
			newpt.y = rect.bottom - 40;

			ClientToScreen( (HWND)m_hWnd, &newpt );
			SetCursorPos( newpt.x, newpt.y );
		}

		else if ( pt.y > (rect.bottom - 40) )
		{
			newpt.x = pt.x;
			newpt.y = 40;

			ClientToScreen((HWND)m_hWnd, &newpt);
			SetCursorPos(newpt.x, newpt.y);
		}
	}

	//DebugPrintf("PrevMouseXY: %d %d\n", prevMouseX, prevMouseY);
	
	diff_x = pt.x - prevMouseX;
	diff_y = pt.y - prevMouseY;

	if ( abs(diff_x) > 0 )
	{
		if (abs(diff_x) > (width/2) - 30)
			diff_x = 0;

		prevMouseX = pt.x;	
	}

	if ( abs(diff_y) > 0 )
	{			
		if (abs(diff_y) > ( (height/2) -  50 ) )		
		{
			diff_y = 0;
		}

		prevMouseY = pt.y;		
	}


	if ( IsRMB() )
	{
		float fPreviousYaw = m_fCamYaw;
		float fNewYaw = m_fCamYaw + (((float)diff_x * GetSpeedRot()) / 300.f);

		float fPreviousPitch = m_fCamPitch;
		float fNewPitch = m_fCamPitch + (((float)-diff_y * GetSpeedRot()) / 300.f);

		m_fCamYaw = lerp( fPreviousYaw, fNewYaw, 0.55f );
		m_fCamPitch = lerp( fPreviousPitch, fNewPitch, 0.55f );
		

		/*
		DirectX::XMVECTOR quatPrev = DirectX::XMQuaternionRotationRollPitchYaw( fPreviousPitch, fPreviousYaw, 0.0f );
		DirectX::XMVECTOR quatNew = DirectX::XMQuaternionRotationRollPitchYaw( fNewPitch, fNewYaw, 0.0f );

		auto quat = DirectX::XMQuaternionSlerp( quatPrev, quatNew, 0.35 );

		SQuat q;
		q.x = DirectX::XMVectorGetX( quat );
		q.y = DirectX::XMVectorGetY( quat );
		q.z = DirectX::XMVectorGetZ( quat );
		q.w = DirectX::XMVectorGetW( quat );

		//m_fCamYaw = q.GetPitch();
		//m_fCamPitch = q.GetYaw();


		//DebugPrintf("CamYaw: %f, CamPitch: %f\n", RadToDeg(m_fCamYaw), RadToDeg(m_fCamPitch) );
		*/

		/*
		m_fCamYaw += (((float)diff_x * GetSpeedRot())  / 600.f);
		m_fCamPitch += (((float)-diff_y * GetSpeedRot()) / 600.f);
		*/

		UpdateMatrixView(0);
	}
}

//----------------------------------------------------------------------
void CCameraFree::HandleInput(class CInput* input, float dt)
{
	m_bChange = false;

	Vec3 vOldPos = GetPosition();
	Vec3 vNewPos = vOldPos;

	
	// Handle keyboard...
	if (input->keyDown(DIK_Q))
	{
		/*
		float f = m_pos.y;
		f += GetSpeedMove() * dt ;
		SetY(f);
		*/

		vNewPos.y += GetSpeedMove() * dt;

		m_bChange = true;
	}

	if (input->keyDown(DIK_E) || input->keyDown(DIK_Z) )
	{
		/*
		float f = m_pos.y;
		f -= GetSpeedMove() * dt ;
		SetY(f);
		*/

		vNewPos.y -= GetSpeedMove() * dt;

		m_bChange = true;
	}

	if (input->keyDown(DIK_W))
	{	
		vNewPos.x += m_MatView._13 * GetSpeedMove() * dt ;
		vNewPos.y += m_MatView._23 * GetSpeedMove() * dt ;
		vNewPos.z += m_MatView._33 * GetSpeedMove() * dt ;

		m_bChange = true;
	}


	if (input->keyDown(DIK_S))
	{
		vNewPos.x -= m_MatView._13 * GetSpeedMove() * dt ;
		vNewPos.y -= m_MatView._23 * GetSpeedMove() * dt ;
		vNewPos.z -= m_MatView._33 * GetSpeedMove() * dt ;

		m_bChange = true;
	}

	if (input->keyDown(DIK_X))
	{
		vNewPos.x -= sin(m_fCamYaw) * GetSpeedMove() * dt;
		vNewPos.z -= cos(m_fCamYaw) * GetSpeedMove() * dt;

		m_bChange = true;
	}

	if (input->keyDown(DIK_A))
	{
		vNewPos.x += (sinf(m_fCamYaw - DegToRad(90)) * GetSpeedMove()  * dt );
		vNewPos.z += (cosf(m_fCamYaw - DegToRad(90)) * GetSpeedMove()  * dt );

		m_bChange = true;

	}
	if (input->keyDown(DIK_D))
	{
		vNewPos.x -= (sinf(m_fCamYaw - DegToRad(90)) * GetSpeedMove() * dt );
		vNewPos.z -= (cosf(m_fCamYaw - DegToRad(90)) * GetSpeedMove() * dt );

		m_bChange = true;
	}


	if (m_bChange)
	{
		Vec3 vInterpPos = interpolate(vOldPos, vNewPos, 0.5f);
		SetPos(vInterpPos);

		UpdateMatrixView(dt);
	}
	
}

//----------------------------------------------------------------------
void CCameraFree::UpdateMatrixView(float dt)
{
	m_focus.x = sinf(m_fCamYaw) + m_pos.x;
	m_focus.y = m_pos.y;
	m_focus.z = cosf(m_fCamYaw) + m_pos.z;

	static Vec3 up(0.f, 1.f, 0.f);
	Vec3& eye = GetPosition();


	// Calculate view matrix
	Mat44 mLookAt, mRot;
	MatrixLookAtLH(eye, m_focus, up, &mLookAt);
	MatrixRotationYawPitchRoll(0.f, m_fCamPitch, 0.f, &mRot);

	MatrixMult(mLookAt, mRot, &m_MatView);

	// Calculate viewProj matrix
	MatrixMult(m_MatView, m_MatProj, &m_MatViewProj);
}

//----------------------------------------------------------------------
void* CCameraFree::operator new(size_t size)
{
	return _aligned_malloc(size, 16);
}

//----------------------------------------------------------------------
void CCameraFree::operator delete(void* pObj)
{
	if (pObj != NULL)
		_aligned_free(pObj);
}

//----------------------------------------------------------------------
void CCameraFree::OnRMBDown()
{
	SetRMB(true);

	GetCursorPos(&NMCameraEditor::m_curPosBeginDrag);
}

//----------------------------------------------------------------------
void CCameraFree::OnRMBUp()
{
	SetCursorPos(NMCameraEditor::m_curPosBeginDrag.x, NMCameraEditor::m_curPosBeginDrag.y);
	
	SetRMB(false);
}
