#include "stdafx.h"
#include ".\tone.h"

CTone::CTone()
: m_nTones(0)
{
	//	Create Empty tone
}

CTone::CTone(short duration, unsigned short freq1, short ampl1, unsigned short freq2, short ampl2,short offtime)
: m_nTones(0)
{
	AddTone(duration,freq1,ampl1,freq2,ampl2,offtime);
	Cycles(1);
}

CTone::~CTone(void)
{
}

void CTone::Cycles(int count)
{
	m_Candence.cycles = count;
}

void CTone::Clear(void)
{
	m_nTones = 0;
}

int CTone::AddTone(short duration, unsigned short freq1, short ampl1, unsigned short freq2, short ampl2,short offtime)
{
	if(m_nTones <= 3)
	{
		dx_bldtngen(&m_Candence.tone[m_nTones], freq1, freq2, ampl1, ampl2, duration);
		m_Candence.offtime[m_nTones] = offtime;
		m_Candence.numsegs = m_nTones+1;
		m_nTones++;
		return 0;
	}
	return -1;
}

bool CTone::IsCandence(void) const
{
	return (m_nTones > 0);
}

const TN_GEN& CTone::GetTone(void) const
{
	return m_Candence.tone[0];
}

const TN_GENCAD& CTone::GetCandence(void) const
{
	return m_Candence;
}
