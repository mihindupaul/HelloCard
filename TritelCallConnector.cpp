#include "stdafx.h"
#include ".\tritelcallconnector.h"

Tritel::CTritelCallConnector::CTritelCallConnector(CLine& line)
:CCallConnector(line)
{
}

Tritel::CTritelCallConnector::~CTritelCallConnector(void)
{
}

int Tritel::CTritelCallConnector::UpdateComCharges()
{
	UPDATECALLCHARGE ub;

	nCallDuration=(int)difftime(finish_time,start_time);

	//	avoid -1 updation of call charges.
	if(nCallDuration > m_call_rate.nAvailableTime)
		nCallDuration = m_call_rate.nAvailableTime;

	strcpy(ub.chAuthorizationKey,m_call_rate.chAuthorizationKey);

	ub.nNumberOfSecons = nCallDuration;
	ub.nBusinessCode = m_call_rate.nBusinessCode;

	ub.nBlock1=m_call_rate.nBlock1;
	ub.nBlock2=m_call_rate.nBlock2;
	ub.nBlock3=m_call_rate.nBlock3;
	ub.fCost1=m_call_rate.fCost1;
	ub.fCost2=m_call_rate.fCost2;
	ub.fCost3=m_call_rate.fCost3;

	if(CDatabaseSupport2::GetInstance()->SetUpdateCallCharge(ub) == 1)
	{
		fUsedValue = ub.fUsedValue;
	}

	return 1;
}

int Tritel::CTritelCallConnector::Transfer(CCallProcessor& amc, CALLRATE& rate)
{
	std::string dial_no = "";
	char buffer[10];
	time_t ltime;

	time( &ltime );

	//	Area code conversion
	dial_no.assign(rate.chDialString);
	LOG4CPLUS_INFO(CONNECTOR_LOGGER,"DNI=" << rate.chDestinationNumber << " >> " << dial_no); 

	//	Update user interface about whats going on
	_itoa(GetLine().Channel(), buffer, 10 );
	ListviewSetColumn(amc.IncommingLine().Channel()-1,2,buffer);
	ListviewSetColumn(amc.IncommingLine().Channel()-1,3,dial_no);
	ListviewSetColumn(amc.IncommingLine().Channel()-1,5,ctime(&ltime));
	ListviewSetColumn(amc.IncommingLine().Channel()-1,6,"Calling...");

	_itoa(rate.nAvailableTime,buffer,10);

	ListviewSetColumn(amc.IncommingLine().Channel()-1,7,buffer);

	//	Save call rates for log updating
	m_call_rate = rate;
	nCallDuration = 0;	//	Reset the Value
	fUsedValue	= 0;
	time(&start_time);	//	Not the actual start time...

	//	TODO: send the dial_no as the destination
	return CCallConnector::Transfer(amc,dial_no,rate.nAvailableTime);
}

//	This is to make a db entry for finished calls
int Tritel::CTritelCallConnector::CommLog(char* msg)
{
	COMCALLLOG cl; 

	sprintf(cl.chTime, ctime(&start_time));

	////Now Update The Log		
	cl.nCallDuration = nCallDuration;
	cl.nUnits = nCallDuration;
	cl.fCallCharge = fUsedValue;
	cl.fActualCost = nCallDuration * m_call_rate.fAUCharge;
	cl.nOutgoingLineNumber =  GetLine().Channel();

	cl.nPlanID = m_call_rate.nPlanID;
	cl.nRateID	= m_call_rate.nRateID;
	cl.nBusinessCode = m_call_rate.nBusinessCode;

	cl.fUnitCharge = m_call_rate.fCost1/m_call_rate.nBlock1;

	////	strings are deep copied.	
	strcpy(cl.chCallerID,m_call_rate.chAuthorizationKey);
	strcpy(cl.chDNI,m_call_rate.chDestinationNumber);
	
	strcpy(cl.chDate,"");
	strcpy(cl.chCallType,m_call_rate.chCallType);
	strcpy(cl.chDestinationCode,m_call_rate.chDestinationCode);
	strcpy(cl.chSpCode,m_call_rate.chSpCode);  
	strcpy(cl.chIPAddress,m_call_rate.chIPAddress);
	strcpy(cl.chCallStatus,msg); //	Call Success String
	strcpy(cl.chRoutingMethod,m_call_rate.chRoutingMethod);
	
	return CDatabaseSupport2::GetInstance()->SetComCallLog(cl);
}

void Tritel::CTritelCallConnector::OnStateChange(int state)
{
	//	do my local functions
	switch(state)
	{
	case CON_FINISHED:	//	Call finished
		time(&finish_time);	//	Time of Finish
		UpdateComCharges();
		CommLog("SUCCESS");
		break;
	case CON_CONFAILED:
		CommLog("FAILURE");
		break;
	case CON_CONNECTED:
		if(HasCaller())
		{
			time(&start_time);	//	Re-get start time exactly on connection
			ListviewSetColumn(IncommingLine().Channel()-1,6,"Connected");
		}
	}
	CCallConnector::OnStateChange(state);
}

