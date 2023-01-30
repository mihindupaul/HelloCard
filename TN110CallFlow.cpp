#include "stdafx.h"
#include ".\tn110callflow.h"
#include "registry.h"

CTN110CallFlow::CTN110CallFlow(CLineManager& manager)
: CCallFlow(manager)
{
}

CTN110CallFlow::~CTN110CallFlow(void)
{
}

int CTN110CallFlow::ProcessCallFlow(CLine& line, int code, int data)
{
	char chCustomerType[300];
	int nReturn=0,i=0,count=0;
	char digits2[30]="";

	CRegistry Registry;	
	CDatabaseSupport DatabaseSupport;
	PINLOCK PL;
	CALLRATE c;	
	PINAUTHENTICATE pa;

	CLine* plineo;

	switch(line.callflowstate)
	{

	case START:						
		ListviewSetColumn(line.channel-1,6,"Hello Booth - Play C Tone");	
		dx_clrdigbuf(line.voiceh);
		line.state = ST_AUTHENTICATION;
		line.callflowstate=GETPIN;
		//PlayCtone(line);
		line.PlayTone('C');
		break;
	case GETPIN:
		ListviewSetColumn(line.channel-1,6,"Get The PIN");
		line.callflowstate=GOTPIN;
		dx_clrdigbuf(line.voiceh);
		return line.GetDigit(11,DM_P,500);
		//return(GetDigit(line,11,DM_P,500));		
	case GOTPIN:	
		strcat(line.chPinNo,line.dtbuf.dg_value);
		//ListviewSetColumn(line.channel-1,3,line.chPinNo);

		if(strlen(line.chPinNo)>=1)
		{
			strncpy( digits2, line.chPinNo, strlen(line.chPinNo)-1);
			strcpy( line.chPinNo,digits2);
		}
		else 
		{
			StoreErrCode(line, 1);
			line.PlayTone('*');
			ListviewSetColumn(line.channel-1,6,"Invalid PinNumber");
			return 1;
		}
		//MessageBox(NULL,line.digits,NULL,MB_OK);	
		//strcpy(line.chPinNo,line.dtbuf.dg_value);
		dx_clrdigbuf(line.voiceh);
		//MessageBox(NULL,line.chPinNo,NULL,MB_OK);			
		if(strlen(line.chPinNo)==10)
		{
			//Validate Pin Number 
			strcpy(pa.chPinNumber,line.chPinNo);
			pa.nCustomerType=line.nCustomerType;
			nReturn=DatabaseSupport.GetPinAuthentication(&pa);
			if(nReturn==1)
			{
				if(pa.nResult==0) //Vaild Pin Number
				{	
					line.fCurrentBalance=pa.fCurrentBalance;
					line.MoneyValue=pa.fCurrentBalance;
					strcpy(line.chSerialNo,pa.chSerialNo);
					strcpy(line.chBusinessDescription,pa.chBusinessDescription);

					//User Interface
					strcpy(chCustomerType,"TN110 Booth- ");
					strcat(chCustomerType,line.chBusinessDescription);
					strcpy(line.chBusinessDescription,chCustomerType);

					ListviewSetColumn(line.channel-1,4,chCustomerType);
					line.callflowstate=PLAYBALANCE;							
					StoreBalance(line);
					ListviewSetColumn(line.channel-1,6,"Play The Balance");
					//PlaySilence(line, balance_time);
					line.PlayTone(' ');
					return 1;
				}
				else
					if(pa.nResult==8) //Insufficient Fund
					{
						StoreErrCode(line, 3);
						//line.PlayTone('*');
						line.PlayTone('*');
						ListviewSetColumn(line.channel-1,6,"Insufficient Fund");
						return 1;
					}
					else
						if(pa.nResult==6) //EXPIRED CARD card
						{
							StoreErrCode(line, 2);
							line.PlayTone('*');
							ListviewSetColumn(line.channel-1,6,"Expired Card");								
							return 1;
						}
						else //Invalid Pin Number
						{
							StoreErrCode(line, 1);
							line.PlayTone('*');
							ListviewSetColumn(line.channel-1,6,"Invalid Pin Number");
							return 1;
						}							
			}
			else //Please Customer Service For Assistance
			{
				StoreErrCode(line, 5);
				line.PlayTone('*');
				ListviewSetColumn(line.channel-1,6,"Database Error");
				return 1;
			}
		} 
		else //Invalid Pin Number
		{
			StoreErrCode(line, 1);
			line.PlayTone('*');
			ListviewSetColumn(line.channel-1,6,"Invalid PinNumber");
			return 1;
		}
		break;
	case PLAYBALANCE:
		PlayBalance(line);
		break;
	case GETDN:
		dx_clrdigbuf(line.voiceh);
		line.callflowstate=GETDIGIT;
		ListviewSetColumn(line.channel-1,6,"Get Dest Number");
		//			dx_clrdigbuf(line.voiceh);
		//return(GetDigit(line,20,DM_P,500));
		return line.GetDigit(20,DM_P,500);
		break;
	case GETDIGIT:
		if (code == TDX_GETDIG) //GetDigit Terminated
		{
			if(strlen(line.dtbuf.dg_value)>7){
				strcat(line.digits,line.dtbuf.dg_value);
				//MessageBox(NULL,line.digits,NULL,MB_OK);	
				strncpy( digits2, line.digits, strlen(line.digits)-1);
				strcpy( line.digits,digits2);
				ListviewSetColumn(line.channel-1,3,line.digits);
			} else {
				ListviewSetColumn(line.channel-1,3,line.digits);
				ListviewSetColumn(line.channel-1,6,"Invalid DN");
				StoreErrCode(line, 6);
				line.PlayTone('*');
				return 1;					
			}
		}
		else
		{
		}					
		//Database Work
		strcpy(c.chAuthorizationKey,line.chPinNo);
		strcpy(c.chDestinationNumber,line.digits);
		c.nBusinessCode=line.nBusinessCode;
		c.nPlanID=line.nPlanID;
		strcpy(c.chAreaCode,line.chAreaCode);

//		strcpy(c.chIPAddress,IPAddress);

		//Call the spCallRate Stored Procedure
		nReturn=DatabaseSupport.GetCallRate(&c);
		if(nReturn==0)
		{
			//indicate platform error
			StoreErrCode(line, 5);
			line.PlayTone('*');
			return 1;
		}
		else
			if(nReturn==1)
			{
				switch(c.nResult)
				{	
				case 4: // Insufficient Balance to Proceed

					ListviewSetColumn(line.channel-1,6,"Insufficient Fund");
					StoreErrCode(line, 3);
					line.PlayTone('*');
					return 1;
				case 5: //Invalid Destination Number							
					ListviewSetColumn(line.channel-1,6,"Invalid Number");
					StoreErrCode(line, 6);
					line.PlayTone('*');
					return 1;					
				case 0: // Customer Can Make a Call
					{	line.nBlock1=c.nBlock1;
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
					line.OStart=c.nOStart;
					line.OEnd=c.nOEnd;
					ListviewSetColumn(line.channel-1,3,line.chDialString);
					ListviewSetColumn(line.channel-1,6,"Play Available Time");
					//Aviable time in Sec
					line.nAvailableTime=c.nAvailableTime;
					line.PlayTime=line.nAvailableTime;
					line.callflowstate=PLAYTIME;
					StoreTime(line);
					//PlaySilence(line, duration_time);
					line.PlayTone(' ',duration_time);
					break;
					}
				default:						
					//Play Call Customer Service For Assistance							
					ListviewSetColumn(line.channel-1,6,"Database error");
					StoreErrCode(line, 5);
					line.PlayTone('*');
					return 1;
					break;
				}			
			}
			break;
	case PLAYTIME:
		PlayTime(line);
		break;
	case DIALOUT:
		dx_clrdigbuf(line.voiceh);
		//	acquire free line from my manager's outline pool
		if(m_Manager.GetFreeOutgoingLine(&plineo)) // Failed acquire
		{
			ListviewSetColumn(line.channel-1,6,"Congestion");
			//sprintf(msgpath, "%s%s", line.voxpath,IDS_1021a);
			//Play(line,msgpath);
			line.Play(IDS_1021a);
			line.state=ST_PLAYEND;
			return 1;
		}

		line.connected=false;
		plineo->ichannel=line.channel;
		line.ochannel=plineo->channel;				
		//Lock The Pin Number
		strcpy(PL.chPinNumber,line.chPinNo);
		nReturn=DatabaseSupport.SetPinLock(&PL);
		
		if(line.state == ST_DIALING)
		{			
			plineo->MakeCall("user_dialed_number");
			ListviewSetColumn(line.channel-1,6,"Dialling Out");
		}

		break;
	case PLAYERRORTONES:
		PlayErrTones(line);
		break;
	}
	return 1;
}

void CTN110CallFlow::StoreErrCode(CLine& line, int errorCode)
{
	int number=0;
	line.digitsindex=0;
	line.callflowstate=PLAYERRORTONES;
	//1.Number of ten thousands
	/*	number= (int)(errorCode/10000);
	if(number!=0)
	errorCode=errorCode-number*10000;
	*/
	line.digitsarray[0]=0;

	//2.Number of Thousands
	/*	number=(int)errorCode/1000;
	if(number!=0)
	errorCode=errorCode-number*1000;
	*/
	line.digitsarray[1]=0;

	//3.Number of hundred
	/*number=(int)errorCode/100;
	if(number!=0)
	errorCode=errorCode-number*100;
	*/
	line.digitsarray[2]=0;

	//4.Number of hundred
	/*number=(int)errorCode/10;
	if(number!=0)
	errorCode=errorCode-number*10;
	*/
	line.digitsarray[3]=0;

	//5.Number of Tens
	/*number=(int)errorCode;
	//PlayTones(number,line);*/
	line.digitsarray[4]=errorCode;
}

int CTN110CallFlow::PlayBalance(CLine& line)
{
	int number;

	if(line.digitsindex>=6) 
	{
		line.callflowstate=GETDN;		
		//PlayHashtone(pline);
		line.PlayTone('#');
	}
	else 
	{
		number=(int)line.digitsarray[line.digitsindex++];
		switch(number)
		{
		case 0:	line.PlayTone('0'); break;
		case 1: line.PlayTone('1'); break;
		case 2: line.PlayTone('2'); break;
		case 3: line.PlayTone('3'); break;
		case 4: line.PlayTone('4'); break;
		case 5: line.PlayTone('5'); break;
		case 6: line.PlayTone('6'); break;
		case 7:	line.PlayTone('7'); break;
		case 8:	line.PlayTone('8'); break;
		case 9: line.PlayTone('9'); break;
		}
	}
	return 1;
}

void CTN110CallFlow::PlayErrTones(CLine& line)
{
	int number;

	if(line.digitsindex>=5) 
	{
		line.state=ST_ERROR;		
		//PlayHashtone(pline);
		line.PlayTone('#');
	}
	else 
	{
		number=(int)line.digitsarray[line.digitsindex++];
		//_itoa( number, buffer, 10 );
		//ListviewSetColumn(pline->channel-1,4,buffer);	
		switch(number)
		{
		case 0:	line.PlayTone('0'); break;
		case 1: line.PlayTone('1'); break;
		case 2: line.PlayTone('2'); break;
		case 3: line.PlayTone('3'); break;
		case 4: line.PlayTone('4'); break;
		case 5: line.PlayTone('5'); break;
		case 6: line.PlayTone('6'); break;
		case 7:	line.PlayTone('7'); break;
		case 8:	line.PlayTone('8'); break;
		case 9: line.PlayTone('9'); break;
		}
	}
}

int CTN110CallFlow::StoreBalance(CLine& line)
{
	return 0;
}

int CTN110CallFlow::StoreTime(CLine& line)
{
	return 0;
}

int CTN110CallFlow::PlayTime(CLine& line)
{
	return 0;
}
