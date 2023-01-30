// tester.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <stddef.h>
#include "../databasesupport2.h"
#include <list>
#include <sstream>

using namespace Tritel;
using namespace std;

void TestDatabase();
int CommLog(char* msg);
std::list<CDatabaseSupport2*> x;

#define MAXCHAN   60               /* max. number of channels in system */  

int _tmain(int argc, _TCHAR* argv[])
{
    int            vbnum = 0;           /* virtual board number (1-based) */  
    int            vch = 0;             /* voice channel number (1-based) */  
	int number = 0;
	float MoneyValue = 500;
//	TestDatabase();
	number = (int)(MoneyValue/ 100);

	MoneyValue = MoneyValue - number*100;


	cout << "Done. Press ENTER key.";
	cin.get();
	return 0;
}

void TestDatabase()
{
	using namespace std;
	using namespace Tritel;

	int n;
	CUSTOMERTYPE ct;
	CALLRATE cr;
	

	for(n=0;n<1;n++)
	{
		cout <<  "begin Testing..." << n << endl;

		::memset(&cr,0,sizeof(CALLRATE));
		::memset(&ct,0,sizeof(CUSTOMERTYPE));

		strcpy(ct.chCallerID,"119976491");

	//	mem_fun_ref<

		// fill the Customer information
		if(CDatabaseSupport2::GetInstance()->GetCustomerType(ct)) //Execute the Stored Procedure Success !!!
		{
			//	Now CT has been Taken. Get The Call Rate for this customer
			strcpy(cr.chAuthorizationKey,ct.chCallerID);
			strcpy(cr.chAreaCode,ct.chAreaCode);
			strcpy(cr.chDestinationNumber,"0332273748");
			strcpy(cr.chIPAddress, "192.168.2.7");
			cr.nPlanID = ct.nPlanID;
			cr.nBusinessCode = ct.nBusinessCode;
			
			//
			if(CDatabaseSupport2::GetInstance()->GetCallRate(cr))
			{
				std::cout << "Customer:" << ct.chLanguage << ct.nCustomerType << ct.fCurrentBalance << std::endl;
				std::cout << "Call Rate" << cr.chDialString << cr.chCallType << std::endl;
			}
			//CommLog("MIHINDU");
			//UPDATECALLCHARGE uc;
			//memset(&uc,0,sizeof(uc));
			//CDatabaseSupport2::GetInstance()->SetUpdateCallCharge(uc);
			//COMRECHARGE comr;
			//strcpy(	comr.chCallerID , "119976491");
			//strcpy(comr.chPinNumber,"9999999999");
			//CDatabaseSupport2::GetInstance()->SetComRecharge(comr);
		}
	}

	CDatabaseSupport2::Cleanup();
}

int CommLog(char* msg)
{
	COMCALLLOG cl; 

//	sprintf(cl.chTime, ctime(&start_time));

	////Now Update The Log		
	cl.nCallDuration = 12;
	cl.nUnits = 12;
	cl.fCallCharge = 45;
	cl.fActualCost = 3 * 6;
	cl.nOutgoingLineNumber =  7;

	cl.nPlanID = 1;
	cl.nRateID	= 1;
	cl.nBusinessCode = 1;

	cl.fUnitCharge = 12;

	////	strings are deep copied.	
	strcpy(cl.chCallerID,"10222222");
	strcpy(cl.chDNI,"2222222");
	
	strcpy(cl.chDate,"");

	
	return CDatabaseSupport2::GetInstance()->SetComCallLog(cl);
}
