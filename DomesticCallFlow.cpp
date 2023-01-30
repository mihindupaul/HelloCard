#include "stdafx.h"
#include ".\domesticcallflow.h"

CDomesticCallFlow::CDomesticCallFlow(CLineManager& manager)
: CCallFlow(manager)
{
}

CDomesticCallFlow::~CDomesticCallFlow(void)
{
}

int CDomesticCallFlow::ProcessCallFlow(CLine& line, int code, int data)
{
	char digit[2];

	int nReturn=0,i=0,minute,count=0;

	CDatabaseSupport DatabaseSupport;
	CALLRATE c;	DIGITS d;	
	PINAUTHENTICATE pa;
	PINLOCK PL;


	CLine *plineo;


	switch(line.callflowstate)
	{

	case STARTCLI:

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
				line.nBusinessCode=pa.nBusinessCode;
				line.nPlanID=pa.nPlanID;
				strcpy(line.chSerialNo,pa.chSerialNo);
				strcpy(line.chBusinessDescription,pa.chBusinessDescription);


				strcpy(line.chLanguage,"English");

				ListviewSetColumn(line.channel-1,4,line.chBusinessDescription);
				ListviewSetColumn(line.channel-1,6,"WelCome Greeting");

				line.callflowstate=GETDESTNO;

				dx_clrdigbuf(line.voiceh);

				//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1011);


				ListviewSetColumn(line.channel-1,6,"Get Destination Number");
		
				//return(PlayFile(pline,msgpath));
				return line.Play(line.chLanguage,IDS_1011);


			}
			else if(pa.nResult==8) //Insufficient Fund
			{
				ListviewSetColumn(line.channel-1,6,"Insufficient Fund");

				//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1009);

				//Play(pline,msgpath);
				line.Play(line.chLanguage,IDS_1009);
				line.state=ST_PLAYEND;

				return 1;

			}
			else if(pa.nResult==6) //EXPIRED CARD card
			{
				ListviewSetColumn(line.channel-1,6,"Expired Card");

				//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1007);

				//Play(pline,msgpath);
				line.Play(line.chLanguage,IDS_1007);
				line.state=ST_PLAYEND;

				return 1;

			}
			else if(pa.nResult==7) //Pin Number Is Lock
			{
				ListviewSetColumn(line.channel-1,6,"Pin Number Is Lock");

				//sprintf(msgpath, "%s%s", line.voxpath,IDS_1019);

				//Play(pline,msgpath);
				line.Play(IDS_1019);
				line.state=ST_PLAYEND;

				return 1;
			}
		}

		break;


	case START:

		line.nTry=0;

		line.callflowstate=PINNUMBER;

		dx_clrdigbuf(line.voiceh);

		ListviewSetColumn(line.channel-1,6,"Get Pin Number");
		return line.Play(IDS_1005);
		//return(PlayFile2(pline,pinfh));


	case PINNUMBER:

		line.callflowstate=GETPIN;
		return line.GetDigit(10,DM_P,300);
		//return(GetDigit(pline,10,DM_P,300));

		break;


	case GETPIN:	

		line.nTry=line.nTry+1;

		strcpy(line.chPinNo,line.dtbuf.dg_value);

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
					line.nBusinessCode=pa.nBusinessCode;
					line.nPlanID=pa.nPlanID;
					strcpy(line.chSerialNo,pa.chSerialNo);
					strcpy(line.chBusinessDescription,pa.chBusinessDescription);


					ListviewSetColumn(line.channel-1,4,line.chBusinessDescription);

					ListviewSetColumn(line.channel-1,6,"WelCome Greeting");

					//Play the WelCome Message
					//sprintf(msgpath, "%s\\%s", line.voxpath,pa.chWelcome);
					line.Play(pa.chWelcome);

					//Play(pline,msgpath);

					line.callflowstate=VALIDPINNUMBER;



				}
				else
					if(pa.nResult==8) //Insufficient Fund
					{
						ListviewSetColumn(line.channel-1,6,"Insufficient Fund");

						//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1009);

						//Play(pline,msgpath);
						line.Play(line.chLanguage,IDS_1009);
						line.state=ST_PLAYEND;

						return 1;

					}
					else
						if(pa.nResult==6) //EXPIRED CARD card
						{
							ListviewSetColumn(line.channel-1,6,"Expired Card");

							//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1007);

							//Play(pline,msgpath);
							line.Play(line.chLanguage,IDS_1007);
							line.state=ST_PLAYEND;

							return 1;

						}
						else
							if(pa.nResult==7) //Pin Number Is Lock
							{
								ListviewSetColumn(line.channel-1,6,"Pin Number Is Lock");

								//sprintf(msgpath, "%s%s", line.voxpath,IDS_1019);

								//Play(pline,msgpath);
								line.Play(IDS_1019);
								line.state=ST_PLAYEND;

								return 1;

							}
							else //Invalid Pin Number
							{

								//sprintf(msgpath, "%s%s", line.voxpath,IDS_1006);

								ListviewSetColumn(line.channel-1,6,"Invalid PinNumber");
								line.Play(IDS_1006);
								//Play(pline,msgpath);

								line.callflowstate=INVALIDPINNUMBER;

							}


			}
			else //Please Customer Service For Assistance
			{

				//sprintf(msgpath, "%s%s", line.voxpath,IDS_1019);

				ListviewSetColumn(line.channel-1,6,"Unable To Connect DB");
				line.Play(IDS_1019);
				//Play(pline,msgpath);

				line.state=ST_PLAYEND;

				return 1;


			}


		}
		else //Invalid Pin Number
		{

			//sprintf(msgpath, "%s%s", line.voxpath,IDS_1006);


			ListviewSetColumn(line.channel-1,6,"Invalid PinNumber");
			line.Play(IDS_1006);
			//Play(pline,msgpath);

			line.callflowstate=INVALIDPINNUMBER;

		}

		break;

	case INVALIDPINNUMBER:

		if(line.nTry<3)
		{
			line.callflowstate=PINNUMBER;

			dx_clrdigbuf(line.voiceh);

			ListviewSetColumn(line.channel-1,6,"Get Pin Number");

			//return (PlayFile2(pline,pinfh));
			return line.Play(IDS_1005);

		}
		else
		{
			//sprintf(msgpath, "%s%s", line.voxpath,IDS_1003);

			ListviewSetColumn(line.channel-1,6,"Exceeded Number Of Tries");
			line.Play(IDS_1003);
			//Play(pline,msgpath);

			line.state=ST_PLAYEND;

			return 1;


		}


		break;


	case VALIDPINNUMBER:

		line.nTry=0;

		line.callflowstate=LANGUAGE;

		dx_clrdigbuf(line.voiceh);

		//Select 1 For English 2 For Sinhala 3 For Tamil

		ListviewSetColumn(line.channel-1,6,"Language Option");

		return line.Play(IDS_1001);
		//return(PlayFile2(pline,languagefh));


		break;

	case LANGUAGE:

		line.callflowstate=GETLANGUAGE;

		return line.GetDigit(1,DM_P,300);
		break;


	case GETLANGUAGE:

		line.nTry=line.nTry+1;

		strcpy(digit,line.dtbuf.dg_value);

		if(strcmp(digit,"1") == 0)
			strcpy(line.chLanguage,"English");
		else
			if(strcmp(digit,"2") == 0)
				strcpy(line.chLanguage,"Sinhala");
			else
				if(strcmp(digit,"3") == 0)
					strcpy(line.chLanguage,"Tamil");
				else
				{

					//sprintf(msgpath, "%s%s", line.voxpath,IDS_1002);

					ListviewSetColumn(line.channel-1,6,"Invalid Language Option");

					//Play(pline,msgpath);
					line.Play(IDS_1002);
					line.callflowstate=INVALIDLANGUAGE;

				}


				if(strcmp(digit,"1") == 0||strcmp(digit,"2") == 0||strcmp(digit,"3") == 0)		
				{
					//Play You Have
					ListviewSetColumn(line.channel-1,6,"Play Card Balance");

					//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1010a);
					line.Play(line.chLanguage,IDS_1010a);
					//Play(pline,msgpath);

					line.callflowstate=CURRENTBALANCE;

				}



				break;

	case INVALIDLANGUAGE:

		if(line.nTry<3)
		{

			line.callflowstate=LANGUAGE;

			dx_clrdigbuf(line.voiceh);

			//Select 1 For English 2 For Sinhala 3 For Tamil

			ListviewSetColumn(line.channel-1,6,"Language Option");
			return line.Play(IDS_1001);
			//return(PlayFile2(pline,languagefh));

		}
		else
		{
			//sprintf(msgpath, "%s%s", line.voxpath,IDS_1003);

			//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1003);

			ListviewSetColumn(line.channel-1,6,"Exceeded Number Of Tries");
			line.Play(line.chLanguage,IDS_1003);
			//Play(pline,msgpath);

			line.state=ST_PLAYEND;

			return 1;


		}

		break;


	case CURRENTBALANCE:

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
					line.callflowstate=PLAYAND;
			}
		}
		else if((line.MoneyValue<100)&&(line.MoneyValue>=0))
		{
			PlayMoneyValue(&line,1);
		}

		if(line.MoneyValue==0)
			line.callflowstate=PLAYRUPEES;

		break;

	case PLAYAND:


		//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,AND);

		//Play(pline,msgpath);
		line.Play(line.chLanguage,AND);
		line.callflowstate=CURRENTBALANCE;

		break;

	case PLAYRUPEES:


		//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1010b);
		line.Play(line.chLanguage,IDS_1010b);
		//Play(pline,msgpath);

		line.callflowstate=PLAYDESTINATION;

		break;

	case PLAYDESTINATION:

		line.nTry=0;

		line.callflowstate=GETDESTNO;

		dx_clrdigbuf(line.voiceh);

		//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1011);

		ListviewSetColumn(line.channel-1,6,"Get Destination Number");

		//return(PlayFile(pline,msgpath));
		return line.Play(line.chLanguage,IDS_1011);
		break;

	case GETDESTNO:


		line.callflowstate=GETDIGIT;

		return line.GetDigit(3,DM_P,300);

		break;

	case GETDIGIT:

		if (code == TDX_GETDIG) //GetDigit Terminated
		{
			line.nTry=line.nTry+1;

			strcpy(line.digits,line.dtbuf.dg_value);

		}

		if((strnicmp(line.digits,"00",2) == 0))
		{
			line.callflowstate=GETDIGIT2;
			return line.GetDigit(20,DM_P,300);
			//return(GetDigit(pline,20,DM_P,300));
		}
		else
			if((strnicmp(line.digits,"0",1) == 0)) //NationWide
			{

				//Get The Remaining Digits From The DataBase

				strcpy(d.chPrefixDestNo,line.digits);
				strcpy(d.chAreaCode,line.chAreaCode);

				nReturn=DatabaseSupport.GetDigits(&d);

				if(nReturn==1)
				{

					if(d.nResult==0)
					{

						line.callflowstate=GETDIGIT2;
						return line.GetDigit(d.nDestinationDigits-3,DM_P,300);
						//return(GetDigit(pline,d.nDestinationDigits-3,DM_P,300));
					}
					else
					{
						ListviewSetColumn(line.channel-1,6,"Invalid Code");


						//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1012);

						//Play(pline,msgpath);
						line.Play(line.chLanguage,IDS_1012);
						line.callflowstate=INVALIDDESTINATION;

						return 1;
					}

				}
				else
				{
					//Please Call Customer Service For Assistance

					//sprintf(msgpath, "%s%s", line.voxpath,IDS_1019);

					ListviewSetColumn(line.channel-1,6,"Unable To Connect DB");

					//Play(pline,msgpath);
					line.Play(IDS_1019);
					line.state=ST_PLAYEND;

					return 1;

				}


			}
			else //No Local Calls
			{
				ListviewSetColumn(line.channel-1,6,"Invalid Destination Number");

				//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1012);

				//Play(pline,msgpath);
				line.Play(line.chLanguage,IDS_1012);
				line.callflowstate=INVALIDDESTINATION;

				return 1;


			}


			break;

	case INVALIDDESTINATION:


		if(line.nTry<3)
		{

			line.callflowstate=GETDESTNO;

			dx_clrdigbuf(line.voiceh);


			//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1011);


			ListviewSetColumn(line.channel-1,6,"Get Destination Number");

			//return(PlayFile(pline,msgpath));
			return line.Play(line.chLanguage,IDS_1011);

		}
		else
		{

			//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1003);


			ListviewSetColumn(line.channel-1,6,"Exceeded Number Of Tries");

			//Play(pline,msgpath);
			line.Play(line.chLanguage,IDS_1003);
			line.state=ST_PLAYEND;

			return 1;


		}


		break;

	case GETDIGIT2:


		if (code == TDX_GETDIG) //GetDigit Terminated
		{

			strcat(line.digits,line.dtbuf.dg_value);

			ListviewSetColumn(line.channel-1,3,line.digits);
		}
		else
		{

		}

		/***************************Validate The IDD Destination Number********************/

		if((strnicmp(line.digits,"00",2) == 0)&&(strlen(line.digits))<10)
		{

			ListviewSetColumn(line.channel-1,6,"Invalid Number");

			//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1012);

			//Play(pline,msgpath);
			line.Play(line.chLanguage,IDS_1012);
			line.callflowstate=INVALIDDESTINATION;

			return 1;

		}


		//Database Work

		strcpy(c.chAuthorizationKey,line.chPinNo);
		strcpy(c.chDestinationNumber,line.digits);
		c.nBusinessCode=line.nBusinessCode;
		c.nPlanID=line.nPlanID;
		strcpy(c.chAreaCode,line.chAreaCode);
		//	TODO: What is this IP address?
		// strcpy(c.chIPAddress,IPAddress);

		//Call the spCallRate Stored Procedure

		nReturn=DatabaseSupport.GetCallRate(&c);

		if(nReturn==0)
		{

			//Please Call Customer Service For Assistance

			//sprintf(msgpath, "%s%s", line.voxpath,IDS_1019);

			ListviewSetColumn(line.channel-1,6,"Unable To Execute spCallRate Stored Procedure");

			//Play(pline,msgpath);
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

					//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1009);
					line.Play(line.chLanguage,IDS_1009);

					//Play(pline,msgpath);

					line.state=ST_PLAYEND;

					return 1;



				case 5: //Invalid Destination Number


					ListviewSetColumn(line.channel-1,6,"Invalid Number");

					//sprintf(msgpath, "%s%s%s%s", line.voxpath,"\\",line.chLanguage,IDS_1012);
					line.Play(line.chLanguage,IDS_1012);
					//Play(pline,msgpath);

					line.callflowstate=INVALIDDESTINATION;

					return 1;



				case 0: // Customer Can Make a Call



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
					line.OStart=c.nOStart;
					line.OEnd=c.nOEnd;
					line.fAUCharge=c.fAUCharge;

					ListviewSetColumn(line.channel-1,6,"Play Available Time");

					//sprintf(msgpath, "%s\\%s%s",line.voxpath,line.chLanguage,IDS_1013a);	//You have 

					//Play(pline,msgpath);
					line.Play(line.chLanguage,IDS_1013a);
					//Aviable time in Sec
					line.nAvailableTime=c.nAvailableTime;
					line.PlayTime=line.nAvailableTime;

					if(strcmp(line.chLanguage,"Sinhala")==0)							//Only for Sinhala
					{
						if(line.PlayTime<60)
							line.callflowstate=AVIALABLESECONDS;	
						else
							line.callflowstate=AVIALABLEMINUTES;	
					}
					else 
						line.callflowstate=PLAYTIMEVALUE;	


					break;

				default:

					//Play Call Customer Service For Assistance

					//sprintf(msgpath, "%s%s", line.voxpath,IDS_1019);

					ListviewSetColumn(line.channel-1,6,"Customer Service");

					//Play(pline,msgpath);
					line.Play(IDS_1019);
					line.state=ST_PLAYEND;

					return 1;

					break;

				}
			}


			break;

	case PLAYTIMEVALUE:

		minute=line.PlayTime/60;
		if(minute>=1000)
		{
			PlayMinute(line,1000);
		}
		else if((minute<1000) && (minute>=100))
		{
			PlayMinute(line,100);
		}
		else if((minute<100) && (minute>=1))
		{
			PlayMinute(line,1);
		}
		else if(minute<1)
		{
			PlayMinute(line,0);
			if(strcmp(line.chLanguage,"Sinhala") == 0)						//Only for Sinhala
				line.callflowstate=TIMEREAMINS;
			else 
				line.callflowstate=AVIALABLESECONDS;

			return 1;
		}

		if((line.PlayTime/60)==0)
		{
			if(strcmp(line.chLanguage,"Sinhala") == 0)						//Only for Sinhala
			{
				if(line.PlayTime==0)
					line.callflowstate=TIMEREAMINS;
				else
					line.callflowstate=AVIALABLESECONDS;
			}
			else 
				line.callflowstate=AVIALABLEMINUTES;
		}
		else if((line.PlayTime/60)<100)
		{
			if(strcmp(line.chLanguage,"English") == 0)						//Only for English
				line.callflowstate=MINUTESAND;
		}

		break;

	case MINUTESAND:
		//sprintf(msgpath, "%s\\%s%s",line.voxpath,line.chLanguage,AND);			//And
		//Play(pline,msgpath);
		line.Play(line.chLanguage,AND);
		line.callflowstate=PLAYTIMEVALUE;
		break;

	case AVIALABLEMINUTES:
		//sprintf(msgpath, "%s\\%s%s",line.voxpath,line.chLanguage,MINUTES);		//Minutes
		//Play(pline,msgpath);
		line.Play(line.chLanguage,MINUTES);
		if(strcmp(line.chLanguage,"Sinhala") == 0)							//Only for Sinhala
		{
			line.callflowstate=PLAYTIMEVALUE;
		}
		else
		{
			if(line.PlayTime!=0)
			{
				if(strcmp(line.chLanguage,"English") == 0)					//Only for English
					line.callflowstate=MINUTESAND;
				else
					line.callflowstate=PLAYTIMEVALUE;
			}
			else
				line.callflowstate=TIMEREAMINS;
		}
		break;

	case AVIALABLESECONDS:
		//sprintf(msgpath, "%s\\%s%s",line.voxpath,line.chLanguage,SECONDS);		//Seconds
		//Play(pline,msgpath);
		line.Play(line.chLanguage,SECONDS);
		if(strcmp(line.chLanguage,"Sinhala") == 0)							//Only for Sinhala
			line.callflowstate=PLAYTIMEVALUE;
		else
			line.callflowstate=TIMEREAMINS;
		break;

	case TIMEREAMINS:
		//sprintf(msgpath, "%s\\%s%s",line.voxpath,line.chLanguage,IDS_1013b);	//On your card
		//Play(pline,msgpath);
		line.Play(line.chLanguage,IDS_1013b);
		line.callflowstate=MAKECALL;

		break;

	case MAKECALL:

		/*******************************************************************/
		/**************** Make A Outbound Call ****************************/
		/******************************************************************/

		//Find the Idel Outgoing Channel using random stratergy
		//	GetOutLine()
		if(m_Manager.GetFreeOutgoingLine(&plineo))
		{
			ListviewSetColumn(line.channel-1,6,"Congestion");
			//sprintf(msgpath, "%s%s", line.voxpath,IDS_1021a);
			//Play(pline,msgpath);
			line.Play(IDS_1021a);
			line.state=ST_PLAYEND;
			return 1;
		}

		line.connected=false;

		// introduce the outgoing line to outgoing line
		plineo->ichannel=line.channel;
		line.ochannel=plineo->channel;

		//Lock The Pin Number
		strcpy(PL.chPinNumber,line.chPinNo);
		nReturn=DatabaseSupport.SetPinLock(&PL);

		if(line.state == ST_DIALING)
		{			
			plineo->MakeCall("the_target_number");
		}
	}
	return 1;
}

int CDomesticCallFlow::GetId(void)
{
	return CALL_FLOW_DOMESTIC;
}

int CDomesticCallFlow::PlayMinute(CLine& line, int flag)
{
	return 0;
}
