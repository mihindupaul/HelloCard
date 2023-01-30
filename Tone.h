#pragma once
#include <srllib.h>
#include <dxxxlib.h>

class CTone
{
public:
	CTone();
	CTone(short duration, unsigned short freq1, short ampl1=R2_DEFAMPL, unsigned short freq2=0, short ampl2=R2_DEFAMPL,short offtime = 0);
	virtual ~CTone(void);

	void Cycles(int count);
	void Clear(void);
	int AddTone(short duration, unsigned short freq1, short ampl1=R2_DEFAMPL, unsigned short freq2=0, short ampl2=R2_DEFAMPL,short offtime = 0);
	bool IsCandence(void) const;
	const TN_GEN& GetTone(void) const;
	const TN_GENCAD& GetCandence(void) const;

private:
	TN_GENCAD	m_Candence;
	int m_nTones;
};
