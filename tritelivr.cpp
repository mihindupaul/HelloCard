#include "stdafx.h"
#include <time.h>
#include <fstream>
#include <sstream>

#include "lineconnector.h"
#include "./tritelivr.h"
#include "log4cplus/logger.h"
#include "./tritelcallconnector.h"

using namespace Tritel;

CTritelIVR::CTritelIVR(CLine& line)
: CAnswerMachine(line)
{
}

CTritelIVR::~CTritelIVR(void)
{
}

void CTritelIVR::CommCallFlow(CLine& line)
{
	std::string dni = "";
	std::ifstream f;

	switch (GetState())
	{
	case ON_CALL:

		break;

	case START: //Start The Call Flow

		dni = line.GetDNIS();

		//if DNI is empty fail the flow
		if(dni.empty()) 
			return;

		
		//	DEBUG: enabling test phone mode
		if(line.GetANI() == "119976491" && dni == "0335674925")//|| ani == "119976508"
		{
			LOG4CPLUS_DEBUG(IVR_LOGGER,"user event added by:" << this);
			line.AddUserEvent(12);
			f.open("c:\\number.txt");
			f >> dni;
			f.close();
		}
		dni = "88b";
		LOG4CPLUS_DEBUG(IVR_LOGGER,"IVR started");
		ListviewSetColumn(line.Channel()-1,3,const_cast<char*>(dni.c_str()));

		if( dni == "88b" || dni == "88" )
		{
			LOG4CPLUS_DEBUG(IVR_LOGGER,"88 Login:" << CT.chCallerID);

			ShowStatus("Recharge Account");
			//	Now we move to the Recharge Call Flow
			line.ClearDigitBuffer();
			SetState(CURRENTBALANCE);
			RechargeCallFlow();
		} 
		else if(CT.fCurrentBalance < 4)
		{
			//Balance Less Than 1
			ShowStatus("Insufficient Balance");
		
			LOG4CPLUS_DEBUG(IVR_LOGGER,"Call Failed insufficent balance" << CT.fCurrentBalance);
			PlayFileLang(RECORD_INSUFFICENT_BALANCE,CONNECT_FAILED);
		}
		else 
		{
			LOG4CPLUS_DEBUG(IVR_LOGGER,"Attempting to make out call...");
			MakeOutCall(dni);
		}
		break;

	case BUSYTONE:

		ShowStatus("Get Digits");							
		SetState(GETDIGIT3);
		GetDigits(4);
		break;
	
	case CONNECT_FAILED: // OR IVR_END
		LOG4CPLUS_INFO(IVR_LOGGER,"IVR Finished with drop tone");
		
		Exit(GC_NORMAL_CLEARING);

		break;
	}
}


void CTritelIVR::RechargeCallFlow()
{
	switch(GetState())
	{
	case CURRENTBALANCE:

		//Your current balance is
		ShowStatus("Play Current Balance");
		MoneyValue = CT.fCurrentBalance;
		
		LOG4CPLUS_INFO(IVR_LOGGER,"Current Balance is " << MoneyValue);
		
		PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1034a,m_CurrentLanguage),PLAYTOTAL);
		
		break;

	case PLAYTOTAL:

		if(MoneyValue>=1000)
		{
			PlayMoneyValue(1000);
		}
		else if((MoneyValue<1000) && (MoneyValue>=100))
		{
			PlayMoneyValue(100);

			if(m_CurrentLanguage == CConfiguration::ENGLISH) //Only for English
			{
				if(MoneyValue >=1 )
					SetState(PLAYAND);
			}
		}
		else if((MoneyValue<100)&&(MoneyValue>=1))
		{
			PlayMoneyValue(1);
		}

		if(MoneyValue<1)
			SetState(PLAYRUPEES);

		break;

	case PLAYAND:
		
		PlayFile(CConfiguration::Instance().GetLangVoxFile(AND,m_CurrentLanguage),PLAYTOTAL);
		break;

	case PLAYRUPEES:

		PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1030b,m_CurrentLanguage),GETPIN);
		break;

	case GETPIN:

		nTry=0;

		//Your current balance is
		ShowStatus("Get New PinNumber");
		PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1031,m_CurrentLanguage),GETDIGIT);
		break;

	case GETDIGIT:

		nTry++;
		SetState(AUTHENTICATE);
		LOG4CPLUS_INFO(IVR_LOGGER,"about to call getdigits" << nTry);

		GetDigits(10);
		LOG4CPLUS_INFO(IVR_LOGGER,"GetDigits() ok" << nTry);
		break;

	case INVALIDPINNUMBER:

		if(nTry < 3)
		{
			ShowStatus("Get Pin Number");
			PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1031,m_CurrentLanguage),GETDIGIT);
		}
		else
		{
			ShowStatus("Exceeded Number Of Tries");
			PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1003,m_CurrentLanguage),THANKS);
		}
		break;

	case NEW_BALANCE:

		ShowStatus("New Balance");
		MoneyValue = CT.fCurrentBalance;

		PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1035a,m_CurrentLanguage),PLAYNEWTOTAL);
		break;

	case PLAYNEWTOTAL:

		if(MoneyValue>=1000)
		{
			PlayMoneyValue(1000);
		}
		else if((MoneyValue<1000) && (MoneyValue>=100))
		{
			PlayMoneyValue(100);

			if(m_CurrentLanguage == CConfiguration::ENGLISH) //Only for English
			{
				if(MoneyValue >= 1 )
				{
					SetState(PLAYNEWAND);
					return;
				}
			}
		}
		else if((MoneyValue<100)&&(MoneyValue>=1))
		{
			PlayMoneyValue(1);
		}

		if(MoneyValue<=1)
			SetState(PLAYNEWRUPEES);

		break;

	case PLAYNEWAND:

		PlayFile(CConfiguration::Instance().GetLangVoxFile(AND,m_CurrentLanguage),PLAYNEWTOTAL);
		break;

	case PLAYNEWRUPEES:
		PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1030b,m_CurrentLanguage),IVR_END);
		break;

	case THANKS:
		PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1004,m_CurrentLanguage),IVR_END);
		break;
	}
}

int CTritelIVR::Begin(CLine& line)
{
	std::string lang;
	std::string caller_id = line.GetANI();

	strcpy(CT.chCallerID,caller_id.c_str());
	
	if(CDatabaseSupport2::GetInstance()->GetCustomerType(CT)) //Execute the Stored Procedure Success !!!
	{
		//	Assign the business model to this line.
		switch(CT.nResult)
		{
		case 1:
			ListviewSetColumn(GetLine().Channel()-1,4,"Communication");
			ListviewSetColumn(GetLine().Channel()-1,6,"Customer Login");

			//	Set Current language of the customer
			lang.assign(CT.chLanguage);
			
			//	TODO: Convert to lowercase
			if(lang == "Sinhala")
				m_CurrentLanguage = CConfiguration::SINHALA;
			else if(lang == "Tamil")
				m_CurrentLanguage = CConfiguration::TAMIL;
			else 
				m_CurrentLanguage = CConfiguration::ENGLISH;

			////	Setup line parameters for this business model
			
			//	DEBUG: explicit language selection
			// m_CurrentLanguage = CConfiguration::SINHALA;
			LOG4CPLUS_INFO( IVR_LOGGER ,"Customer Login: " << caller_id << "Ch=" << line.Channel() << " Language: " << lang);
			
			//	IMPORTANT: Why the hell this is needed? GetDigit() dont work without this
			line.SetVoice(true);

			SetState(START);
			CommCallFlow(line);
			break;

		default:

			//	This is not a valid customer. exit with message
			Exit(CConfiguration::Instance().GetVoxFile(IDS_1019));
			break;
		}		
	}
	else
	{
		//Please Call Customer Service For Assistance
		LOG4CPLUS_ERROR( IVR_LOGGER ,"Canot retrive Called User" << caller_id);
		ShowStatus("Unable To Connect DB");
		Exit(CConfiguration::Instance().GetVoxFile(IDS_1019));	//	exit ivr system. (Play and end condition)
	}

	return 0;
}

//
//	Template method to handle PlayFileComplete Event
//
void CTritelIVR::OnPlayFileTerminate(int reason)
{
	if(!IsSuspended())
	{
		// LOG4CPLUS_DEBUG( IVR_LOGGER ,__FUNCTION__ << reason);
		CommCallFlow(GetLine());
		RechargeCallFlow();
		//	TotalBusiness(*m_pLine,0,0);
	}
}

void CTritelIVR::OnGetDigitsTerminate(int reason,std::string digits)
{
	COMTOTALBUSINESS ct;
	char chPassword[5];

	switch(GetState())
	{
	case GETDIGIT:
		//	Get The input value
		strcpy(chPassword,digits.c_str());
		
		if(strlen(chPassword)==3)
		{
			strcpy(ct.chCallerID,IncommingLine().GetANI().c_str());
			strcpy(ct.chPassWord,chPassword);

			if(CDatabaseSupport2::GetInstance()->GetComTotalBusiness(ct) == 1)
			{
				if(ct.nResult==0)
				{	
					if(ct.fTotalAmount>=0)
					{
						ShowStatus("Play Total Business");
						MoneyValue = ct.fTotalAmount;
						PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1030a,m_CurrentLanguage),PLAYTOTAL);
					}
					else	
					{
						Exit(CConfiguration::Instance().GetLangVoxFile(IDS_1049,m_CurrentLanguage));
					}
				}
				else
				{
					ShowStatus("Invalid Password");
					PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1028,m_CurrentLanguage),INVALIDPASSWORD);
				}
			}
			else
			{
				//Please Call Customer Service For Assistance
				Exit(CConfiguration::Instance().GetVoxFile(IDS_1019));
			}
		}
		else
		{
			//Invalid Password
			ShowStatus("Invalid Password");
			PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1028,m_CurrentLanguage),INVALIDPASSWORD);
		}
		break;

	case AUTHENTICATE:

		LOG4CPLUS_ERROR( IVR_LOGGER , "PIN Extracted=" << digits);
		
		switch(reason)
		{
		case TM_NORMTERM:
		case TM_DIGIT:
		case TM_MAXDTMF:

			LOG4CPLUS_ERROR( IVR_LOGGER ,"Good reason to process DTMF" << reason);
			RechargeAccount(digits);
			break;

		default:
		
			LOG4CPLUS_ERROR( IVR_LOGGER ,"GetDigit TM_MAXTIME/TM_IDDTIME" << reason);
			PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1032,m_CurrentLanguage),INVALIDPINNUMBER);
			break;
		}
		break;
	}
}

int CTritelIVR::PlayMoneyValue(int flag)
{
	int number = 0;
	char file_name[MAX_PATH];

	switch(flag)
	{
	case 1000:

		number= (int)( MoneyValue/1000);
		MoneyValue = MoneyValue - number*1000;

		// LOG4CPLUS_ERROR(IVR_LOGGER,"1000-" << MoneyValue << "-" << number);

		if(m_CurrentLanguage == CConfiguration::ENGLISH) 
		{
			sprintf(file_name,"\\n%d000.vox",number);
		}
		else
		{
			//Only for Tamil and Sinhala has to changed
			if( MoneyValue!=0)
				sprintf(file_name, "\\s%d000.vox",number);
		}
		
		PlayFile(CConfiguration::Instance().GetLangVoxFile(file_name,m_CurrentLanguage));
		break;

	case 100:

		number = (int)(MoneyValue/ 100);
		MoneyValue = MoneyValue - number*100;

		LOG4CPLUS_ERROR(IVR_LOGGER,"100-" << MoneyValue << "-" << number);

		if(m_CurrentLanguage == CConfiguration::ENGLISH) //Only for Tamil and Sinhala has to changed
		{
			sprintf(file_name,"\\n0%d00.vox",number);
		}
		else
		{
			if( MoneyValue!=0)
				sprintf(file_name, "\\s0%d00.vox",number);
		}

		PlayFile(CConfiguration::Instance().GetLangVoxFile(file_name,m_CurrentLanguage));

		break;

	case 1:

		sprintf(file_name,"\\n%04d.vox",(int)MoneyValue);

		PlayFile(CConfiguration::Instance().GetLangVoxFile(file_name,m_CurrentLanguage));
		MoneyValue=0;
		break;

	default:
		LOG4CPLUS_ERROR( IVR_LOGGER,__FUNCTION__ << flag);
	}
	return 0;
}

//	any Call to Call Reseting of this ivr can be done hear
void CTritelIVR::Reset()
{
	nTry = 0;
	CAnswerMachine::Reset();
}

void CTritelIVR::ShowStatus(char* msg)
{
	LOG4CPLUS_INFO(IVR_LOGGER,"Line Status (" << GetLine().Channel() << ") :" << msg);
	ListviewSetColumn(GetLine().Channel()-1,6,msg);
}

int Tritel::CTritelIVR::RechargeAccount(std::string& pin)
{
	COMRECHARGE cr;

	if( pin.length() == 10 )
	{
		cr.nBusinessCode = CT.nBusinessCode;
		strcpy(cr.chCallerID,IncommingLine().GetANI().c_str());
		strcpy(cr.chPinNumber,pin.c_str());

		if(CDatabaseSupport2::GetInstance()->SetComRecharge(cr) == 1)
		{
			if(cr.nResult == 0)
			{
				CT.fCurrentBalance	=  cr.fCurrentValue;
				PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1033,m_CurrentLanguage),NEW_BALANCE);
				return 0;
			}
			else
			{
				PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1032,m_CurrentLanguage),INVALIDPINNUMBER);
			}
		}
		else
		{
			Exit(CConfiguration::Instance().GetLangVoxFile(IDS_1019,m_CurrentLanguage));
		}
	}
	else
	{
		PlayFile(CConfiguration::Instance().GetLangVoxFile(IDS_1032,m_CurrentLanguage),INVALIDPINNUMBER);
	}
	return 1;
}

int Tritel::CTritelIVR::MakeOutCall(std::string& dni)
{
	CALLRATE c;
	CCallConnector* pConnector;

	// Validate The IDD Destination Number (#of digits)
	if( dni.compare(0,2,"00") == 0 && dni.length() < 10 )
	{
		LOG4CPLUS_INFO(IVR_LOGGER,"Invalid IDD Number detection Logic");
		Exit(CConfiguration::Instance().GetVoxFile(IDS_1050));
		return 0;
	}

	//	create prototype for query
	c.nBusinessCode = CT.nBusinessCode;
	c.nPlanID=CT.nPlanID;
	strcpy(c.chAuthorizationKey,CT.chCallerID);
	strcpy(c.chDestinationNumber,dni.c_str()); // destination number
	strcpy(c.chAreaCode,CT.chAreaCode);
	strcpy(c.chIPAddress,CConfiguration::Instance().GetServerIP().c_str());

	//Call the spCallRate Stored Procedure
	if(CDatabaseSupport2::GetInstance()->GetCallRate(c) != 0)
	{
		LOG4CPLUS_INFO(IVR_LOGGER,"Call Rates successfully retreived");

		//	DEBUG: always make calls
		// c.nResult = 0;
		switch(c.nResult)
		{
		case 4: // Insufficient Balance to Proceed
			ShowStatus("Insufficient Fund");
			PlayFile(CConfiguration::Instance().GetLangVoxFile(RECORD_INSUFFICENT_BALANCE,m_CurrentLanguage),CONNECT_FAILED);
			break;

		case 5: //Invalid Destination Number
			ShowStatus("Invalid Dest No");
			LOG4CPLUS_ERROR(IVR_LOGGER,"Invalid Destination number detected" << dni);
			Exit(GC_UNASSIGNED_NUMBER);
			break;

		case 0: // Customer Can Make a Call

			if(IncommingLine().GetManager().PickOutLineConnector(&pConnector,c.nOStart,c.nOEnd) == 0) 
			{
				//	Try To Branch The control To the Connector
				if(static_cast<CTritelCallConnector*>(pConnector)->Transfer(*this,c) != 0)
				{
					Exit(GC_USER_BUSY);
				}
				//	Call flow is tranfered to Connector.
			}
			else
			{
				ShowStatus("Congestion");
				PlayFile(CConfiguration::Instance().GetVoxFile(RECORD_CONGESTION),CONNECT_FAILED);
			}
			break;
			
		default:
			ShowStatus("Unknown CallRate");
			PlayFile(CConfiguration::Instance().GetVoxFile(IDS_1050),CONNECT_FAILED);
			break;
		}
	}
	else
	{
		LOG4CPLUS_FATAL(IVR_LOGGER,"Unable To Execute spCallRate Stored Procedure");
		Exit(CConfiguration::Instance().GetLangVoxFile(RECORD_CALL_CUSTOMER_SERVICE,m_CurrentLanguage));
	}

	return 0;
}
