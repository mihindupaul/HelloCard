#include "stdafx.h"
#include ".\criticalsection.h"

CCriticalSection::CCriticalSection(void)
{
	::InitializeCriticalSection(&m_cs);
}

CCriticalSection::CCriticalSection(int spincount)
{
	::InitializeCriticalSectionAndSpinCount(&m_cs,spincount);
}

CCriticalSection::~CCriticalSection(void)
{
	::DeleteCriticalSection(&m_cs);
}

void  CCriticalSection::Lock(void)
{
	::EnterCriticalSection(&m_cs);
}

void  CCriticalSection::Unlock(void)
{
	::LeaveCriticalSection(&m_cs);
}

bool  CCriticalSection::TryLock()
{
	return TryEnterCriticalSection(&m_cs) >0;
}
