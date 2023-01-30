// hellospCustomerType.h : Declaration of the ChellospCustomerType

#pragma once

// code generated on Sunday, January 18, 2009, 1:24 PM

[
	#error Security Issue: The connection string may contain a password
// The connection string below may contain plain text passwords and/or
// other sensitive information. Please remove the #error after reviewing
// the connection string for any security related issues. You may want to
// store the password in some other form or use a different user authentication.
db_source(L"Provider=SQLOLEDB.1;Password=sa;Persist Security Info=True;User ID=sa;Initial Catalog=HelloCard;Data Source=192.168.2.7;Use Procedure for Prepare=1;Auto Translate=True;Packet Size=4096;Workstation ID=MIHINDU-LAP;Use Encryption for Data=False;Tag with column collation when possible=False"),
	db_command(L"{ ? = CALL hello.spCustomerType(?,?,?,?,?,?,?) }")
]
class ChellospCustomerType
{
public:




	[ db_aram(1, DBPARAMIO_OUTPUT) ] LONG m_RETURN_VALUE;
	[ db_param(2, DBPARAMIO_INPUT) ] TCHAR m_vcCallerID[16];
	[ db_param(3, DBPARAMIO_INPUT | DBPARAMIO_OUTPUT) ] TCHAR m_vcAreaCode[6];
	[ db_param(4, DBPARAMIO_INPUT | DBPARAMIO_OUTPUT) ] TCHAR m_vcLanguage[21];
	[ db_param(5, DBPARAMIO_INPUT | DBPARAMIO_OUTPUT) ] LONG m_nPlanID;
	[ db_param(6, DBPARAMIO_INPUT | DBPARAMIO_OUTPUT) ] double m_fCurrentBalance;
	[ db_param(7, DBPARAMIO_INPUT | DBPARAMIO_OUTPUT) ] LONG m_nBusinessCode;
	[ db_param(8, DBPARAMIO_INPUT | DBPARAMIO_OUTPUT) ] LONG m_nCustomerType;

	void GetRowsetProperties(CDBPropSet* pPropSet)
	{
		pPropSet->AddProperty(DBPROP_CANFETCHBACKWARDS, true, DBPROPOPTIONS_OPTIONAL);
		pPropSet->AddProperty(DBPROP_CANSCROLLBACKWARDS, true, DBPROPOPTIONS_OPTIONAL);
	}
};


