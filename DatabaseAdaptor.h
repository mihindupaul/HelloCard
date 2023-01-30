#pragma once
#include "databasesupport.h"

class CDatabaseAdaptor
{
public:
	CDatabaseAdaptor(void);
	virtual ~CDatabaseAdaptor(void);
private:
	// Existing class to adapt
	CDatabaseSupport m_pDatabaseSupport;
public:
	BOOL SetPinUnlock(PINUNLOCK* pPINUNLOCK);
	BOOL GetCallRate(CALLRATE* pCALLRATE);
};
