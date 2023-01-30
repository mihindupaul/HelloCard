#pragma once
#include "stdafx.h"

class CCriticalSection
{
public:
	CCriticalSection(void);
	CCriticalSection(int spincount);
	virtual ~CCriticalSection(void);

	virtual void Lock(void);
	void Unlock(void);
	bool TryLock();

private:
	CRITICAL_SECTION m_cs;
};
