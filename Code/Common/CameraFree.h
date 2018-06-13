#ifndef __CameraFree_h__
#define __CameraFree_h__

#include "ICamera.h"
#include "quat.h"

class CCameraFree : public ICamera
{
public:
	CCameraFree();
	CCameraFree(float posX, float posY, float posZ, float movSpeed, float rotSpeed, float yaw, float pitch);
	CCameraFree(const f32 yaw, const f32 pitch);
	virtual ~CCameraFree() {}

	virtual void HandleMousemove();
	virtual void HandleInput(class CInput* input, float dt);	
	virtual void UpdateMatrixView(float dt);

	virtual void OnRMBDown();
	virtual void OnRMBUp();

	void* operator new(size_t size);	
	void operator delete(void* pObj);

	

private:
	bool	m_speedIncreases;
	Quaternion	m_quatRotation;
};

#endif