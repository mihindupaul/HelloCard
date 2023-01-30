#include "stdafx.h"
#include <time.h>
#include "databasesupport2.h"
#include "./cellcallflow.h"
#include "log4cplus/logger.h"

#define IVR_LOGGER	log4cplus::Logger::getInstance("IVR")

using namespace Tritel;

CTritelIVR::CTritelIVR(CLineManager& manager)
: CBaseIVR(manager)
{
}

CTritelIVR::~CTritelIVR(void)
{
}

int CTritelIVR::CommCallFlow(CLine& line)
{
	int nReturn=0,count=0,OStart=0,OEnd=0,i=0;
	CALLRATE c;
	bool Congestion=false;
	CCallConnector* pConnector;

	switch (m_nCallFlowState)
	{
	case START: //Start The Call Flow
		//if DNI is empty
		if(!line.dni)
		{
			return 1;
		}					
		
		if(strnicmp(line.dni,"44",2) == 0)
		{
			//	currently not suppported
			TotalBusiness(line,0,0);
		}
		else if(strnicmp(line.dni,"88", 2) == 0)
		{
			//strcat(digits1,"#");
			ListviewSetColumn(line.channel-1,3,"88");
			ListviewSetColumn(line.channel-1,6,"Recharge Account");
			//Application State

			//Commented by Bumalka on 19-09-2007
			//Not dialing 88(recharging digits) along with Pin No. Dialed only 88.
			//strncpy(line.chPinNo,line.dni+3, sizeof(line.dni) ); 

			//Callflow State
			m_nCallFlowState = CURRENTBALANCE;

			//Now we move to the Recharge Call Flow
			Recharge(line);

			return 1;
		} 
		else if(line.fCurrentBalance < 4)
		{
			//Balance Less Than 1
			m_nCallFlowState=BUSYTONE;
			dx_clrdigbuf(line.voiceh);
			ListviewSetColumn(line.channel-1,6,"Insufficient Balance");
			//	TODO: play insufficent balance message. not the Busy tone
			return line.Play(IDS_1021);
		}
		else 
		{
			strcpy(line.digits,line.dni);
			ListviewSetColumn(line.channel-1,3,line.digits);

			//Validate The IDD Destination Number
			if( (strnicmp(line.digits,"00",2) == 0) && (strlen(line.digits))<10)
			{
				line.Play(IDS_1050);
				line.state=ST_PLAYEND;
				return 1;
			}

			strcpy(c.chAuthorizationKey,line.ani);
			strcpy(c.chDestinationNumber,line.digits);
			c.nBusinessCode=line.nBusinessCode;
			c.nPlanID=line.nPlanID;
			strcpy(c.chAreaCode,line.chAreaCode);
			strcpy(c.chIPAddress,"0.0.0.0");	//	Set this from registry settings

			//Call the spCallRate Stored Procedure
			nReturn=CDatabaseSupport2::GetInstance()->GetCallRate(&c);

			if(nReturn==0)
			{
				ListviewSetColumn(line.channel-1,6,"Unable To Execute spCallRate Stored Procedure");
				line.Play(IDS_1019);
				line.state=ST_PLAYEND;
				return 1;
			}
			else if(nReturn==1)
			{
				switch(c.nResult)
				{
				case 4: // Insufficient Balance to Proceed
					ListviewSetColumn(line.channel-1,6,"Insufficient Fund");
					line.Play(IDS_1050);
					line.state=ST_PLAYEND;
					return 1;
				case 5: //Invalid Destination Number
					ListviewSetColumn(line.channel-1,6,"Invalid Dest No");
					line.Play(IDS_1050);
					line.state=ST_PLAYEND;
					return 1;
				case 0: // Customer Can Make a Call
					//	copy db values to line
					line.nBlock1=c.nBlock1;
					line.nBlock2=c.nBlock2;
					line.nBlock3=c.nBlock3;
					line.fCost1=c.fCost1;
					line.fCost2=c.fCost2;
					line.fCost3=c.fCost3;
					line.nRateID=c.nRateID;

					strcpy(line.chCallType,c.chCallType);
					strcpy(line.chDestinationCode,c.chDestinationCode);
					strcpy(line.chDialString,c.chDialString);

					strcpy(line.chRoutingMethod,c.chRoutingMethod);
					strcpy(line.chSpCode,c.chSpCode);
					line.fAUCharge=c.fAUCharge;

					if(m_Manager.PickOutLineConnector(&pConnector) == 0) 
					{
						ListviewSetColumn(line.channel-1,6,"Congestion");
						//	goto end of this call flow
						//this->Exit(IDS_1021a);

						line.Play(IDS_1021a);
						line.state=ST_PLAYEND;
						return 1;
					}
					//	hand over the line to call connector 
					pConnector->Connect(&line,line.chDialString,c.nAvailableTime);
					break;

				default:
					//Play Busy Tone
					ListviewSetColumn(line.channel-1,6,"Unknown Status");
					line.Play(IDS_1050);
					line.state=ST_PLAYEND;
					return 1;
					break;
				}
			}
		}

		break;
	case BUSYTONE:

		ListviewSetColumn(line.channel-1,6,"Get Digits");							
		m_nCallFlowState=GETDIGIT3;
		line.GetDigit(4,DM_P,300);

		break;
	}
	return 1;
}


int CTritelIVR::TotalBusiness(CLine& line, int code, int data)
{
	char chPassword[5];
	COMTOTALBUSINESS ct;
	int nReturn;

	switch(m_nCallFlowState)
	{
	case WELCOMEPASSWORD:

		line.nTry=0;

		dx_clrdigbuf(line.voiceh);

		//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1027);

		m_nCallFlowState=GETPASSWORD;

		ListviewSetColumn(line.channel-1,6,"Get Password");

		return line.Play(line.chLanguage,IDS_1027);

		break;

	case GETPASSWORD:

		line.nTry=line.nTry+1;

		m_nCallFlowState=GETDIGIT;

		return line.GetDigit(3,DM_P,300);

		break;

	case GETDIGIT:

		if (code == TDX_GETDIG) //GetDigit Terminated
		{

			strcpy(chPassword,line.dtbuf.dg_value);

			if(strlen(chPassword)==3)
			{

				strcpy(ct.chCallerID,line.ani);
				strcpy(ct.chPassWord,chPassword);

				nReturn=CDatabaseSupport2::GetInstance()->GetComTotalBusiness(&ct);

				if(nReturn==1)
				{

					if(ct.nResult==0)
					{	
						if(ct.fTotalAmount>=0)
						{
							ListviewSetColumn(line.channel-1,6,"Play Total Business");

							//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1030a);

							//Play(pline,msgpath);
							line.Play(line.chLanguage,IDS_1030a);
							line.MoneyValue=ct.fTotalAmount;

							m_nCallFlowState=PLAYTOTAL;
						}
						else	
						{

							//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1049);

							line.Play(line.chLanguage,IDS_1049);
							//Play(pline,msgpath);

							line.state=ST_PLAYEND;

							return 1;

						}

					}
					else
					{

						ListviewSetColumn(line.channel-1,6,"Invalid Password");
						line.Play(line.chLanguage,IDS_1028);

						//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1028);

						//Play(pline,msgpath);

						//line.state=ST_PLAYEND;
						m_nCallFlowState=INVALIDPASSWORD;

						return 1;

					}


				}
				else
				{
					//Please Call Customer Service For Assistance

					//sprintf(msgpath, "%s%s", line.voxpath,IDS_1019);
					//Play(pline,msgpath);

					line.Play(IDS_1019);
					line.state=ST_PLAYEND;

					return 1;

				}

			}
			else
			{
				//Invalid Password
				ListviewSetColumn(line.channel-1,6,"Invalid Password");
				line.Play(line.chLanguage,IDS_1028);
				//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1028);

				//Play(pline,msgpath);

				//line.state=ST_PLAYEND;
				m_nCallFlowState=INVALIDPASSWORD;

				return 1;

			}
		}

		break;


	case INVALIDPASSWORD:

		if(line.nTry<3)
		{

			m_nCallFlowState=GETPASSWORD;

			dx_clrdigbuf(line.voiceh);

			//sprintf(msgpath, "%s%s", line.voxpath,IDS_1028);
			//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1027);

			ListviewSetColumn(line.channel-1,6,"Get Password");
			return line.Play(line.chLanguage,IDS_1027);
			//return(PlayFile(pline,msgpath));
		}
		else
		{
			//sprintf(msgpath, "%s%s", line.voxpath,IDS_1003);
			//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1003);

			ListviewSetColumn(line.channel-1,6,"Exceeded Number Of Tries");

			//Play(pline,msgpath);
			line.Play(line.chLanguage,IDS_1003);

			line.state=ST_PLAYEND;

			return 1;


		}


		break;

	case PLAYTOTAL:

		if(line.MoneyValue>=1000)
		{
			PlayMoneyValue(&line,1000);
		}
		else if((line.MoneyValue<1000) && (line.MoneyValue>=100))
		{
			PlayMoneyValue(&line,100);
			if((strcmp(line.chLanguage,"English") == 0)) //Only for English
			{
				if(line.MoneyValue!=0)
					m_nCallFlowState=PLAYAND;
			}
		}
		else if((line.MoneyValue<100)&&(line.MoneyValue>=0))
		{
			PlayMoneyValue(&line,1);
		}

		if(line.MoneyValue==0)
			m_nCallFlowState=PLAYRUPEES;
		break;

	case PLAYAND:

		//sprintf(msgpath, "%s%s", line.voxpath,AND);
		//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,AND);

		//Play(pline,msgpath);
		line.Play(line.chLanguage,AND);
		m_nCallFlowState=PLAYTOTAL;

		break;

	case PLAYRUPEES:

		//sprintf(msgpath, "%s%s", line.voxpath,IDS_1030b);
		///sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1030b);
		line.Play(line.chLanguage,IDS_1030b);
		//Play(pline,msgpath);

		line.state=ST_PLAYEND;

		break;


	}

	return 1;
}

int CTritelIVR::Recharge(CLine& line)
{
	switch(m_nCallFlowState)
	{
	case CURRENTBALANCE:

		//Your current balance is
		ListviewSetColumn(line.channel-1,6,"Play Current Balance");
		line.Play(line.chLanguage,IDS_1034a);
		line.MoneyValue=line.fCurrentBalance;
		m_nCallFlowState=PLAYTOTAL;
		break;


	case PLAYTOTAL:

		if(line.MoneyValue>=1000)
		{
			PlayMoneyValue(&line,1000);
		}
		else if((line.MoneyValue<1000) && (line.MoneyValue>=100))
		{
			PlayMoneyValue(&line,100);
			if((strcmp(line.chLanguage,"English") == 0)) //Only for English
			{
				if(line.MoneyValue!=0)
					m_nCallFlowState=PLAYAND;
			}
		}
		else if((line.MoneyValue<100)&&(line.MoneyValue>=0))
		{
			PlayMoneyValue(&line,1);
		}

		if(line.MoneyValue==0)
			m_nCallFlowState=PLAYRUPEES;
		break;

	case PLAYAND:

		line.Play(line.chLanguage,AND);
		m_nCallFlowState=PLAYTOTAL;

		break;

	case PLAYRUPEES:

		line.Play(line.chLanguage,IDS_1030b);
		m_nCallFlowState=GETPIN;
		break;

	case GETPIN:

		line.nTry=0;

		//Your current balance is
		ListviewSetColumn(line.channel-1,6,"Get New PinNumber");

		line.Play(line.chLanguage,IDS_1031);

		m_nCallFlowState=GETDIGIT;

		break;

	case GETDIGIT:

		line.nTry=line.nTry+1;

		m_nCallFlowState=AUTHENTICATE;

		return line.GetDigit(10,DM_P,300);
		break;


	case INVALIDPINNUMBER:

		if(line.nTry<3)
		{

			m_nCallFlowState=GETDIGIT;

			dx_clrdigbuf(line.voiceh);

			ListviewSetColumn(line.channel-1,6,"Get Pin Number");

			return line.Play(line.chLanguage,IDS_1031);

		}
		else
		{
			//sprintf(msgpath, "%s%s", line.voxpath,IDS_1003);
			//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1003);

			ListviewSetColumn(line.channel-1,6,"Exceeded Number Of Tries");

			//Play(pline,msgpath);
			line.Play(line.chLanguage,IDS_1003);

			line.state=ST_PLAYEND;

			return 1;


		}
		break;

	case NEW_BALANCE:
		dx_stopch(line.voiceh, EV_SYNC);
		ListviewSetColumn(line.channel-1,6,"New Balance");
		line.Play(line.chLanguage,IDS_1035a);
		line.MoneyValue=line.fCurrentBalance;
		m_nCallFlowState=PLAYNEWTOTAL;
		break;

	case PLAYNEWTOTAL:

		if(line.MoneyValue>=1000)
		{
			PlayMoneyValue(&line,1000);
		}
		else if((line.MoneyValue<1000) && (line.MoneyValue>=100))
		{
			PlayMoneyValue(&line,100);
			if((strcmp(line.chLanguage,"English") == 0)) //Only for English
			{
				if(line.MoneyValue!=0)
					m_nCallFlowState=PLAYNEWAND;
			}
		}
		else if((line.MoneyValue<100)&&(line.MoneyValue>=0))
		{
			PlayMoneyValue(&line,1);
		}

		if(line.MoneyValue==0)
			m_nCallFlowState=PLAYNEWRUPEES;
		break;

	case PLAYNEWAND:

		//sprintf(msgpath, "%s%s", line.voxpath,AND);
		//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,AND);

		//Play(pline,msgpath);
		line.Play(line.chLanguage,AND);
		m_nCallFlowState=PLAYNEWTOTAL;

		break;

	case PLAYNEWRUPEES:

		line.Play(line.chLanguage,IDS_1030b);
		line.state=ST_PLAYEND;

		break;

	}

	return 1;
}

int CTritelIVR::Begin(CLine& line)
{
	CUSTOMERTYPE CT;
	PINCLI PC;
		
	strcpy(CT.chCallerID,m_pLine->ani);

	int nReturn= CDatabaseSupport2::GetInstance()->GetCustomerType(&CT);

	if(nReturn == 0) //Unable To Execute the Stored Procedure Failure !!!
	{
		//Please Call Customer Service For Assistance
		ListviewSetColumn(m_pLine->channel-1,6,"Unable To Connect DB");
		LOG4CPLUS_ERROR( IVR_LOGGER ,"Canot retrive Called User" << m_pLine->ani);
		Exit(IDS_1019);	//	exit ivr system.
	}
	else if(nReturn == 1) //Execute the Stored Procedure Success !!!
	{
		//	Assign the business model to this line.
		switch(CT.nResult)
		{
		case 1:
			ListviewSetColumn(m_pLine->channel-1,4,"Communication");
			ListviewSetColumn(m_pLine->channel-1,6,"Customer Login");

			//	Setup line parameters for this business model
			strcpy(m_pLine->chAreaCode,CT.chAreaCode);
			strcpy(m_pLine->chLanguage,CT.chLanguage);
			m_pLine->nPlanID=CT.nPlanID;
			m_pLine->fCurrentBalance=CT.fCurrentBalance;
			m_pLine->nBusinessCode=CT.nBusinessCode;
			m_pLine->nCustomerType=CT.nCustomerType;

			//	set to correct callflow state
			m_nCallFlowState = START;
			CommCallFlow(line);
			break;

		default:
			//	This is not a valid customer
			//	play the message. and exit
			break;
		}		
	}
	return 0;
}

int CTritelIVR::OnPlayCompleted(CLine& line,int reason)
{
	//	any of follwing function can handle the state
	CommCallFlow(line);
	Recharge(line);
	TotalBusiness(line,0,0);
	return 0;
}

int CTritelIVR::OnGetDigitCompleted(CLine& line,int reason)
{
	switch(m_nCallFlowState)
	{
	case AUTHENTICATE:

		switch(reason)
		{
		case TM_DIGIT:
			strcpy(pinno,line.dtbuf.dg_value);
			pinno[strlen(pinno)-1]='\0';
			break;
		case TM_MAXTIME:
		case TM_IDDTIME:
		case TM_MAXDTMF:
			strcpy(pinno,line.dtbuf.dg_value);
			break;
		default:
			strcpy(pinno,line.dtbuf.dg_value);
			pinno[strlen(pinno)-1]='\0';
			break;
		}

		//if(strlen(line.chPinNo)==10)
		if(strlen(pinno)==10)
		{
			strcpy(cr.chCallerID,line.ani);
			cr.nBusinessCode=line.nBusinessCode;
			//strcpy(cr.chPinNumber,line.chPinNo);
			strcpy(cr.chPinNumber,pinno);

			nReturn=CDatabaseSupport2::GetInstance()->SetComRecharge(&cr);

			if(nReturn=1)
			{
				if(cr.nResult==0)
				{
					dx_clrdigbuf(line.voiceh);
					//sprintf(msgpath, "%s%s", voxpath,IDS_1033);
					//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1033);
					line.fCurrentBalance=cr.fCurrentValue;
					//Play(pline,msgpath);
					line.Play(line.chLanguage,IDS_1033);
					m_nCallFlowState=NEW_BALANCE;
				}
				else
				{
					line.ClearDigitBuffer();
					line.Play(line.chLanguage,IDS_1032);
					//line.state=ST_PLAYEND;
					m_nCallFlowState=INVALIDPINNUMBER;
				}
			}
			else
			{
				dx_clrdigbuf(line.voiceh);
				line.Play(IDS_1019);
				line.state=ST_PLAYEND;
			}
		}
		else
		{
			dx_clrdigbuf(line.voiceh);
			line.Play(line.chLanguage,IDS_1032);
			m_nCallFlowState=INVALIDPINNUMBER;
			//	line.state=ST_PLAYEND;
		}
		break;
	}
	return 0;
}

int CTritelIVR::out_CallConnected(CLine* l)
{
	return 0;
}
