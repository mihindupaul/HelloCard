#include "stdafx.h"
#include ".\databaseadaptor.h"

CDatabaseAdaptor::CDatabaseAdaptor(void)
{
}

CDatabaseAdaptor::~CDatabaseAdaptor(void)
{
}

BOOL CDatabaseAdaptor::SetPinUnlock(PINUNLOCK* pPINUNLOCK)
{
	return m_pDatabaseSupport.SetPinLock(pPINUNLOCK);
}

BOOL CDatabaseAdaptor::GetCallRate(CALLRATE* pCALLRATE)
{
	return m_pDatabaseSupport.GetCallRate(pCALLRATE);
}
