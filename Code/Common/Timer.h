#ifndef __timer_h__
#define __timer_h__

#ifdef _MSC_VER
# pragma once
#endif

#include "PCH.h"

class CTimer
{
public:
	CTimer();
	~CTimer();

	void Update();

	float ElapsedSeconds()		const;
	float ElapsedMiliseconds()	const;

	float DeltaSeconds()		const;
	float DeltaMiliseconds()	const;
	
private:
	int64_t m_startTime;

	int64_t m_elapsed;
	float	m_fElapsed;

	int64_t m_elapsedSeconds;
	float	m_fElapsedSeconds;

	int64_t	m_elapsedMiliseconds;
	float	m_fElapsedMiliseconds;

	float	m_fDeltaSeconds;
	float	m_fDeltaMiliseconds;

	float	m_fFrequency;
};

#endif