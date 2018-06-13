#include "PCH.h"
#include "Timer.h"


CTimer::CTimer()
{
	LARGE_INTEGER largeInt;

	// Frequency
	QueryPerformanceFrequency(&largeInt);
	m_fFrequency = static_cast<float>(largeInt.QuadPart);

	// Initial time
	QueryPerformanceCounter(&largeInt);
	m_startTime = largeInt.QuadPart;
	m_elapsed = largeInt.QuadPart - m_startTime;

	m_fElapsedSeconds = m_elapsed / m_fFrequency;
	m_fElapsedMiliseconds = m_fElapsedMiliseconds * 1000.f;

	m_fDeltaSeconds = 0.f;
	m_fDeltaMiliseconds = 0.f;
}

CTimer::~CTimer()
{

}

void CTimer::Update()
{
	LARGE_INTEGER largeInt;
	QueryPerformanceCounter(&largeInt);

	int64_t currentTime = largeInt.QuadPart - m_startTime;
	int64_t delta = currentTime - m_elapsed;
	
	m_fDeltaSeconds = delta / m_fFrequency;
	m_fDeltaMiliseconds = m_fDeltaSeconds * 1000.f;
	
	m_elapsed = currentTime;

	m_fElapsedSeconds = m_elapsed / m_fFrequency;
	m_fElapsedMiliseconds = m_fElapsedSeconds * 1000.f;

}

float CTimer::ElapsedSeconds() const
{
	return static_cast<float>(m_fElapsedSeconds);
}

float CTimer::ElapsedMiliseconds() const
{
	return static_cast<float>( m_fElapsedMiliseconds );
}

float CTimer::DeltaSeconds() const
{
	return m_fDeltaSeconds;
}

float CTimer::DeltaMiliseconds() const
{
	return m_fDeltaMiliseconds;
}
