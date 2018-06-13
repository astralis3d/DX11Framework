#pragma once

#include <vector>

// Forward decls
struct ID3D11Query;

struct SGPUTimerState
{
	SGPUTimerState();

	bool Init(ID3D11Device* pDevice);
	void Shutdown();

	ID3D11Query*	m_pQueryGPUBegin;
	ID3D11Query*	m_pQueryGPUEnd;
	bool			m_bTimestampQueryInFlight;
	float			m_fGPUTimeinMS;
};


typedef uint32 timestampID;

class D3DProfiler
{
public:
	D3DProfiler();

	bool Initialize(ID3D11Device* pDevice, int timestamps);
	void Shutdown();

	void BeginFrame(ID3D11DeviceContext* pDevCon);
	void EndFrame(ID3D11DeviceContext* pDevCon);

	void StartTimer(ID3D11DeviceContext* pDevCon, timestampID id);
	void EndTimer(ID3D11DeviceContext* pDevCon, timestampID id);

	float GetGPUTimeinMS(timestampID id);

private:
	std::vector<SGPUTimerState>	m_Timers;
	ID3D11Query*				m_pQueryTimestampDisjoint;	
	
	uint32	m_timestamps;
	bool	m_bDisjointQueryInFlight;
};