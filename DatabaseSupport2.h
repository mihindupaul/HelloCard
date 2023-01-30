#pragma once
#include < atldbcli.h>
#include "databasesupport.h"

//using namespace Tritel;
namespace Tritel{
class CDatabaseSupport2
{
public:
	CDatabaseSupport2();
	virtual ~CDatabaseSupport2();
	
	//	Singelton Acccess methods
	static void Cleanup();
	static CDatabaseSupport2* GetInstance();

private:
	void CloseDatabaseSession2();
	static CDatabaseSupport2* s_pInstance;
	bool m_fSessionCreated;
	CDataConnection m_Connection;

public:
	BOOL GetCustomerType(CUSTOMERTYPE& CustomerType);

private:
	int ConnectToDatabaseEx(void);
public:
	BOOL GetCallRate(CALLRATE& CallRate);
	BOOL SetComRecharge(COMRECHARGE& ComRecharge);
	BOOL SetUpdateCallCharge(UPDATECALLCHARGE& UpdateCallCharge);
	BOOL SetComCallLog(COMCALLLOG& ComCallLog);
	BOOL GetComTotalBusiness(COMTOTALBUSINESS& ComTotalBusiness);
	int TestFunc(void);
};



};