#include "PCH.h"
#include "D3DProfiler.h"

SGPUTimerState::SGPUTimerState()
	: m_pQueryGPUBegin(nullptr)
	, m_pQueryGPUEnd(nullptr)
	, m_bTimestampQueryInFlight(false)
{

}

bool SGPUTimerState::Init(ID3D11Device* pDevice)
{
	if (!pDevice)
		return false;

	D3D11_QUERY_DESC qDesc;
	qDesc.Query = D3D11_QUERY_TIMESTAMP;
	qDesc.MiscFlags = 0;

	if (FAILED(pDevice->CreateQuery(&qDesc, &m_pQueryGPUBegin)))
	{
		return false;
	}

	if (FAILED(pDevice->CreateQuery(&qDesc, &m_pQueryGPUEnd)))
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------
void SGPUTimerState::Shutdown()
{
	SAFE_RELEASE(m_pQueryGPUBegin);
	SAFE_RELEASE(m_pQueryGPUEnd);
}


//------------------------------------------------------------------------
// D3DProfiler
//------------------------------------------------------------------------
D3DProfiler::D3DProfiler()
	: m_timestamps(0)
	, m_bDisjointQueryInFlight(false)
{
	
}

bool D3DProfiler::Initialize(ID3D11Device* pDevice, int timestamps)
{
	m_timestamps = timestamps;
	m_bDisjointQueryInFlight = false;

	// Initialize timestamp disjoint query
	D3D11_QUERY_DESC qDesc;
	qDesc.MiscFlags = 0;
	qDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

	if (FAILED(pDevice->CreateQuery(&qDesc, &m_pQueryTimestampDisjoint)))
	{
		DebugPrintf("Could not create timestamp disjoint query!\n");
		return false;
	}

	m_Timers.resize( timestamps );
	
	for (int i=0; i < timestamps; ++i)
	{
		bool bResult = m_Timers[i].Init(pDevice);

		if (!bResult)
			return false;
	}
	
	return true;
}

//------------------------------------------------------------------------
void D3DProfiler::Shutdown()
{
	SAFE_RELEASE( m_pQueryTimestampDisjoint );

	for (uint32 j=0; j < m_timestamps; ++j)
	{
		m_Timers[j].Shutdown();
	}

	m_Timers.clear();
}

//------------------------------------------------------------------------
void D3DProfiler::BeginFrame(ID3D11DeviceContext* pDevCon)
{
	if (!m_bDisjointQueryInFlight)
	{
		pDevCon->Begin(m_pQueryTimestampDisjoint);
	}

	StartTimer(pDevCon, 0);
}

//------------------------------------------------------------------------
void D3DProfiler::EndFrame(ID3D11DeviceContext* pDevCon)
{
	EndTimer(pDevCon, 0);

	if (!m_bDisjointQueryInFlight)
	{
		pDevCon->End(m_pQueryTimestampDisjoint);
	}
	m_bDisjointQueryInFlight = true;

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointTimestampValue;
	if (pDevCon->GetData(m_pQueryTimestampDisjoint, &disjointTimestampValue, sizeof(disjointTimestampValue), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK)
	{
		m_bDisjointQueryInFlight = false;

		if (FALSE == disjointTimestampValue.Disjoint)
		{
			const double InvFrequencyMS = 1000.0 / disjointTimestampValue.Frequency;

			for (UINT i=0; i < m_timestamps; ++i)
			{
				if (m_Timers[i].m_bTimestampQueryInFlight)
				{
					UINT64 timestampValueBegin;
					UINT64 timestampValueEnd;

					if ((pDevCon->GetData(m_Timers[i].m_pQueryGPUBegin, &timestampValueBegin, sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK) &&
						(pDevCon->GetData(m_Timers[i].m_pQueryGPUEnd, &timestampValueEnd, sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK))
					{
						m_Timers[i].m_bTimestampQueryInFlight = false;	
						m_Timers[i].m_fGPUTimeinMS = float( double(timestampValueEnd - timestampValueBegin) * InvFrequencyMS);
					}
				}
				else
				{
					m_Timers[i].m_fGPUTimeinMS = 0.0f;
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void D3DProfiler::StartTimer(ID3D11DeviceContext* pDevCon, timestampID id)
{
	if (!m_Timers[id].m_bTimestampQueryInFlight)
	{
		pDevCon->End(m_Timers[id].m_pQueryGPUBegin);
	}
}

//------------------------------------------------------------------------
void D3DProfiler::EndTimer(ID3D11DeviceContext* pDevCon, timestampID id)
{
	if (!m_Timers[id].m_bTimestampQueryInFlight)
	{
		pDevCon->End(m_Timers[id].m_pQueryGPUEnd);
	}

	m_Timers[id].m_bTimestampQueryInFlight = true;
}

//------------------------------------------------------------------------
float D3DProfiler::GetGPUTimeinMS(timestampID id)
{
	if (id < m_Timers.size())
	{
		return m_Timers[id].m_fGPUTimeinMS;
	}
	return 0.0f;
}