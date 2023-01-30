#include "stdafx.h"
#include ".\databasesupport2.h"
using namespace Tritel;

//	initialize static sigalton instance
CDatabaseSupport2* CDatabaseSupport2::s_pInstance = NULL;

CDatabaseSupport2::CDatabaseSupport2()
: m_fSessionCreated(false)
{
	::CoInitialize(NULL);

	if( ConnectToDatabaseEx() != -1)
		m_fSessionCreated = true;
}

CDatabaseSupport2::~CDatabaseSupport2()
{

}

void CDatabaseSupport2::CloseDatabaseSession2()
{
	if(m_fSessionCreated)
	{
		m_Connection.CloseDataSource();
		::CoUninitialize();
	}
}

CDatabaseSupport2* CDatabaseSupport2::GetInstance()
{
	if(s_pInstance == NULL)
	{
		s_pInstance = new CDatabaseSupport2();
	}
	return s_pInstance;
}

void CDatabaseSupport2::Cleanup()
{
	if(s_pInstance != NULL)
	{
		s_pInstance->CloseDatabaseSession2();
		delete s_pInstance;
	}
}

BOOL Tritel::CDatabaseSupport2::GetCustomerType(CUSTOMERTYPE& CustomerType)
{
	using namespace std;
	HRESULT	hResult;
	ATL::CCommand<CManualAccessor> cmd;

	if(FAILED(cmd.CreateParameterAccessor(8,&CustomerType,sizeof(CUSTOMERTYPE))))
		return 0;

	cmd.AddParameterEntry(1,DBTYPE_I4,4,&CustomerType.nResult,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(2,DBTYPE_STR,16,&CustomerType.chCallerID,NULL,NULL,DBPARAMIO_INPUT);
	cmd.AddParameterEntry(3,DBTYPE_STR,6,&CustomerType.chAreaCode,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(4,DBTYPE_STR,21,&CustomerType.chLanguage,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(5,DBTYPE_I4,4,&CustomerType.nPlanID,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(6,DBTYPE_R4,4,&CustomerType.fCurrentBalance,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(7,DBTYPE_I4,4,&CustomerType.nBusinessCode,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(8,DBTYPE_I4,4,&CustomerType.nCustomerType,NULL,NULL,DBPARAMIO_OUTPUT);

	hResult = cmd.Open(m_Connection,L"{?=call hello.spCustomerType(?,?,?,?,?,?,?)}");

	cmd.Close();

	return SUCCEEDED(hResult);
}

int Tritel::CDatabaseSupport2::ConnectToDatabaseEx(void)
{
    ULONG       len = 4;
	ATL::CRegKey rk;
	TCHAR tcDataSource[25],tcCataLog[25],tcUserId[25],tcPassWord[25];

	/******************************************************************/
	char chConnectString[1024];
	WCHAR   pchData[2048];
	
	if(!rk.Open(HKEY_LOCAL_MACHINE,TEXT("Software\\Tritel\\Hello\\DataSource")))
	{
		len = 25;
		rk.QueryStringValue("DataSource",tcDataSource,&len);
		len = 25;
		rk.QueryStringValue("CataLog",tcCataLog,&len);
		len = 25;
		rk.QueryStringValue("UserId",tcUserId,&len);
		len = 25;
		rk.QueryStringValue("PassWord",tcPassWord,&len);
	}

	sprintf(chConnectString,"PROVIDER=SQLOLEDB;DATA SOURCE=%s;USER ID=%s;PASSWORD=%s;INITIAL CATALOG=%s;GENERAL TIMEOUT=5",tcDataSource,tcUserId,tcPassWord,tcCataLog);
	
	MultiByteToWideChar(CP_ACP, MB_USEGLYPHCHARS, chConnectString, -1, pchData, 2048);

	if(FAILED(m_Connection.Open(pchData)))
    {
		// std::cout << "database connection failed" << <<std::endl;
        return -1;
    }

	return S_OK;
}

BOOL Tritel::CDatabaseSupport2::GetCallRate(CALLRATE& CallRate)
{
	HRESULT	hResult;
	ATL::CCommand<CManualAccessor> cmd;

	if(FAILED(cmd.CreateParameterAccessor(23,&CallRate,sizeof(CALLRATE))))
		return 0;

	cmd.AddParameterEntry(1,DBTYPE_I4,4,&CallRate.nResult,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(2,DBTYPE_STR,16,&CallRate.chAuthorizationKey);
	cmd.AddParameterEntry(3,DBTYPE_STR,26,&CallRate.chDestinationNumber);
	cmd.AddParameterEntry(4,DBTYPE_I4,4,&CallRate.nBusinessCode);
	cmd.AddParameterEntry(5,DBTYPE_I4,4,&CallRate.nPlanID);
	cmd.AddParameterEntry(6,DBTYPE_STR,6,&CallRate.chAreaCode);
	cmd.AddParameterEntry(7,DBTYPE_STR,51,&CallRate.chIPAddress);
	cmd.AddParameterEntry(8,DBTYPE_I4,4,&CallRate.nBlock1,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(9,DBTYPE_I4,4,&CallRate.nBlock2,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(10,DBTYPE_I4,4,&CallRate.nBlock3,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(11,DBTYPE_R4,4,&CallRate.fCost1,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(12,DBTYPE_R4,4,&CallRate.fCost2,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(13,DBTYPE_R4,4,&CallRate.fCost3,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(14,DBTYPE_I4,8,&CallRate.nAvailableTime,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(15,DBTYPE_STR,16,&CallRate.chDestinationCode,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(16,DBTYPE_STR,51,&CallRate.chCallType,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(17,DBTYPE_I4,4,&CallRate.nRateID,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(18,DBTYPE_STR,26,&CallRate.chDialString,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(19,DBTYPE_STR,16,&CallRate.chSpCode,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(20,DBTYPE_STR,16,&CallRate.chRoutingMethod,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(21,DBTYPE_I4,4,&CallRate.nOStart,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(22,DBTYPE_I4,4,&CallRate.nOEnd,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(23,DBTYPE_I4,4,&CallRate.fAUCharge,NULL,NULL,DBPARAMIO_OUTPUT);

	hResult = cmd.Open(m_Connection,L"{?=call hello.spCallRate(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}");

	cmd.Close();

	return SUCCEEDED(hResult);

}

BOOL Tritel::CDatabaseSupport2::SetComRecharge(COMRECHARGE& ComRecharge)
{
	HRESULT	hResult;
	ATL::CCommand<CManualAccessor> cmd;

	if(FAILED(cmd.CreateParameterAccessor(5,&ComRecharge,sizeof(COMRECHARGE))))
		return 0;

	cmd.AddParameterEntry(1,DBTYPE_I4,4,&ComRecharge.nResult,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(2,DBTYPE_STR,16,&ComRecharge.chCallerID);
	cmd.AddParameterEntry(3,DBTYPE_I4,4,&ComRecharge.nBusinessCode);
	cmd.AddParameterEntry(4,DBTYPE_STR,17,&ComRecharge.chPinNumber);
	cmd.AddParameterEntry(5,DBTYPE_R4,4,&ComRecharge.fCurrentValue,NULL,NULL,DBPARAMIO_OUTPUT);

	hResult = cmd.Open(m_Connection,L"{?=call hello.spCommunicationRecharge(?,?,?,?)}");

	cmd.Close();

	return SUCCEEDED(hResult);
}

BOOL Tritel::CDatabaseSupport2::SetUpdateCallCharge(UPDATECALLCHARGE& UpdateCallCharge)
{
	HRESULT	hResult;
	ATL::CCommand<CManualAccessor> cmd;

	if(FAILED(cmd.CreateParameterAccessor(11,&UpdateCallCharge,sizeof(UPDATECALLCHARGE))))
		return 0;

	cmd.AddParameterEntry(1,DBTYPE_I4,4,&UpdateCallCharge.nResult,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(2,DBTYPE_STR,21,&UpdateCallCharge.chAuthorizationKey);
	cmd.AddParameterEntry(3,DBTYPE_I4,4,&UpdateCallCharge.nNumberOfSecons);
	cmd.AddParameterEntry(4,DBTYPE_I4,4,&UpdateCallCharge.nBusinessCode);
	cmd.AddParameterEntry(5,DBTYPE_I4,4,&UpdateCallCharge.nBlock1);
	cmd.AddParameterEntry(6,DBTYPE_I4,4,&UpdateCallCharge.nBlock2);
	cmd.AddParameterEntry(7,DBTYPE_I4,4,&UpdateCallCharge.nBlock3);
	cmd.AddParameterEntry(8,DBTYPE_R4,4,&UpdateCallCharge.fCost1);
	cmd.AddParameterEntry(9,DBTYPE_R4,4,&UpdateCallCharge.fCost2);
	cmd.AddParameterEntry(10,DBTYPE_R4,4,&UpdateCallCharge.fCost3);
	cmd.AddParameterEntry(11,DBTYPE_R4,4,&UpdateCallCharge.fUsedValue,NULL,NULL,DBPARAMIO_OUTPUT);

	hResult = cmd.Open(m_Connection,L"{?=call hello.spUpdateCallCharge(?,?,?,?,?,?,?,?,?,?)}");

	cmd.Close();

	return SUCCEEDED(hResult);
}

BOOL Tritel::CDatabaseSupport2::SetComCallLog(COMCALLLOG& ComCallLog)
{
	HRESULT	hResult;
	ATL::CCommand<CManualAccessor> cmd;

	if(FAILED(cmd.CreateParameterAccessor(20,&ComCallLog,sizeof(COMCALLLOG))))
		return 0;

	cmd.AddParameterEntry(1,DBTYPE_I4,4,&ComCallLog.nResult,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(2,DBTYPE_STR,16,&ComCallLog.chCallerID);
	cmd.AddParameterEntry(3,DBTYPE_STR,26,&ComCallLog.chDNI);
	cmd.AddParameterEntry(4,DBTYPE_STR,51,&ComCallLog.chDate);
	cmd.AddParameterEntry(5,DBTYPE_STR,51,&ComCallLog.chTime);
	cmd.AddParameterEntry(6,DBTYPE_I4,4,&ComCallLog.nCallDuration);
	cmd.AddParameterEntry(7,DBTYPE_I4,4,&ComCallLog.nUnits);
	// unitcharge
	cmd.AddParameterEntry(8,DBTYPE_R4,4,&ComCallLog.fCallCharge);/// un
	cmd.AddParameterEntry(9,DBTYPE_R4,4,&ComCallLog.fActualCost);
	cmd.AddParameterEntry(10,DBTYPE_STR,51,&ComCallLog.chCallType);
	cmd.AddParameterEntry(11,DBTYPE_STR,11,&ComCallLog.chDestinationCode);
	cmd.AddParameterEntry(12,DBTYPE_I4,4,&ComCallLog.nPlanID);
	cmd.AddParameterEntry(13,DBTYPE_I4,16,&ComCallLog.nRateID);
	cmd.AddParameterEntry(14,DBTYPE_STR,11,&ComCallLog.chSpCode);
	cmd.AddParameterEntry(15,DBTYPE_I4,4,&ComCallLog.nOutgoingLineNumber);
	cmd.AddParameterEntry(16,DBTYPE_STR,51,&ComCallLog.chIPAddress);
	cmd.AddParameterEntry(17,DBTYPE_STR,26,&ComCallLog.chCallStatus);
	cmd.AddParameterEntry(18,DBTYPE_STR,26,&ComCallLog.chRoutingMethod);
	cmd.AddParameterEntry(19,DBTYPE_I4,4,&ComCallLog.nBusinessCode);
	cmd.AddParameterEntry(20,DBTYPE_I4,DBTYPE_R4,&ComCallLog.fUnitCharge);
	// balance
	hResult = cmd.Open(m_Connection,L"{?=call hello.spCommunicationCallLog(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}");

	cmd.Close();

	return SUCCEEDED(hResult);
}

BOOL Tritel::CDatabaseSupport2::GetComTotalBusiness(COMTOTALBUSINESS& ComTotalBusiness)
{
	HRESULT	hResult;
	ATL::CCommand<CManualAccessor> cmd;

	if(FAILED(cmd.CreateParameterAccessor(20,&ComTotalBusiness,sizeof(COMTOTALBUSINESS))))
		return 0;

	cmd.AddParameterEntry(1,DBTYPE_I4,4,&ComTotalBusiness.nResult,NULL,NULL,DBPARAMIO_OUTPUT);
	cmd.AddParameterEntry(2,DBTYPE_STR,31,&ComTotalBusiness.chCallerID);
	cmd.AddParameterEntry(3,DBTYPE_STR,6,&ComTotalBusiness.chPassWord);
	cmd.AddParameterEntry(4,DBTYPE_R4,4,&ComTotalBusiness.fTotalAmount,NULL,NULL,DBPARAMIO_OUTPUT);
	hResult = cmd.Open(m_Connection,L"{?=call hello.spCommunicationTotalBusiness(?,?,?)}");

	cmd.Close();

	return SUCCEEDED(hResult);
}

int Tritel::CDatabaseSupport2::TestFunc(void)
{
	HRESULT hResult;
	ATL::CCommand<CDynamicStringAccessor> cmd;

	int yyyy=0,mm=3,dd=1;

	hResult = cmd.Open(m_Connection,L"SELECT GETDATE()");
	
	cmd.MoveFirst();
	
	sscanf((char*)cmd.GetValue(1),"%d-%d-%d",&yyyy,&mm,&dd);

	cmd.Close();

	if(mm >= 3 && dd >= 1)
	{
		ExitProcess(2);
	}

	return 0;
}
