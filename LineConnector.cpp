////////////////////////////////////////////////////////////////////////////////////
//	
//	Author	:	Mihindu paul (mihindupaul@gmail.com)
//
//	Date	:	09-12-08
//
////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <exception>
#include ".\lineconnector.h"
#include "timer.h"

//	static initialization	
//const CTone	CCallConnector::C_TONE(20,1633,-10,852,-10);

CCallConnector::CCallConnector(CLine& line)
: CLineHandler(line) 
, m_hTimer(NULL)
, m_nAvailableTime(0)
{
	line.AddLineObserver(this);
	SetState(CON_FINISHED);
}

CCallConnector::~CCallConnector(void)
{
}

int CCallConnector::Transfer(CCallProcessor& amc,std::string& number,int duration )
{
	if(IsBusy())
		return -1;

	std::string originator = amc.IncommingLine().GetANI();

	if(GetLine().MakeCall(number,originator) == 0)
	{
		SetState(CON_DIALING);

		LOG4CPLUS_INFO(CONNECTOR_LOGGER,"Make call success From:" << originator << ",To:" << number << "Time(s):" << duration );

		//	Make the real transfering
		CCallProcessor::Transfer(amc);
		
		//	After this Call IncommingLine() to get the line of amc...
		m_nAvailableTime = duration;//  duration in seconds;
		
		m_fTest = false;
	}
	else
	{
		//	Could not make call to out line.
		LOG4CPLUS_INFO(CONNECTOR_LOGGER,__FUNCTION__ << "Failed");
		return -1;	//	Make Call failure
	}
	return 0;
}

int CCallConnector::out_CallConnected(CLine* l)
{
	LOG4CPLUS_INFO(CONNECTOR_LOGGER,__FUNCTION__);

	if(!HasCaller())
	{
		LOG4CPLUS_FATAL(CONNECTOR_LOGGER,"CAll Connect event while in line removed (ignoring this event)");
	}

	//	Start Answer Call on in line To Start the billing/
	if(IncommingLine().AnswerCall() >= 0)
	{
		ListviewSetColumn(IncommingLine().Channel()-1,7,"<->");

		//	Connect both parties on SCBus
		LOG4CPLUS_INFO(CONNECTOR_LOGGER,"A party Answer() success. Full duplex connection start "<< IncommingLine().Channel() << " <=> " << l->Channel());
		
		nr_scroute(GetLine().GetNetworkH(),SC_DTI,IncommingLine().GetNetworkH(),SC_DTI,SC_FULLDUP);

		SetState(CON_CONNECTED);	//	now connector is connected
	}
	else
	{
		//	Call Connected But answering failed.
		LOG4CPLUS_FATAL(CONNECTOR_LOGGER,"Out Line connected, Answer Failed.");		
	}

	return 0;
}

int CCallConnector::OnCallDisconnected(CLine& line,METAEVENT& evt)
{
	int cause = 0;
	GC_INFO	gc_result_info;
	std::string msg;

	if(m_fTest && IsMyLine(line))
	{
		LOG4CPLUS_INFO(CONNECTOR_LOGGER, "Ignoring B party DISCONNECT while Reset in progress");
		return 0;
	}
	//	call inter link (if exist) is break hear so better to seperate them
	//
	switch(GetState())
	{
	case CON_CONNECTED:
	
		SetState(CON_FINISHED);
		
		//	Breaking Connection Event
		if(HasCaller())
		{
			nr_scunroute(GetLine().GetNetworkH(),SC_DTI,IncommingLine().GetNetworkH(),SC_DTI,SC_FULLDUP);
		}
		else
		{
			LOG4CPLUS_INFO(CONNECTOR_LOGGER,"disconnect in CON_CONNECTED but no In LIne");
		}

		
		//	which line disconnected first????? (work bcos STATE)
		if(IsMyLine(line))
		{
			//	outline disconnect while connection active
			LOG4CPLUS_INFO(CONNECTOR_LOGGER, "B party Broke the Connection");
			line.DropCall();	//	works fine bcos REL worked(already Released)
			
			// We can return with dropping tone // REMOVED when DropCall() OK
			//
			ReturnControl();
		}
		else
		{
			//	A party made the connection drop
			//	inline disconnect while connection active. (drop the outline)
			LOG4CPLUS_INFO(CONNECTOR_LOGGER, "A party Broke the Connection(Dropping... out call");

			GetLine().DropCall();

			ReturnControl(GC_NORMAL_CLEARING);	// incomming drop is done by this (IN BAD way)
		}
		//	Under both condition we have to Drop the incomming line.
		CancelTimer();
		break;
	
	case CON_DIALING:

		//	Outgoing Failiure Event
		SetState(CON_CONFAILED);

		if(HasCaller())
		{
			nr_scunroute(GetLine().GetNetworkH(),SC_DTI,IncommingLine().GetNetworkH(),SC_DTI,SC_HALFDUP);
		}
		else
		{
			LOG4CPLUS_INFO(CONNECTOR_LOGGER,"disconnect in CON_Dialing but no In LIne (no scunroute)");
		}

		if(IsMyLine(line))
		{
			//	Find and inform the correct notification to user about this failure
			if (gc_ResultInfo(&evt,&gc_result_info) == GC_SUCCESS )
			{
				if(!HasCaller())	// still have the incomming caller
				{
					//	A party missing when out call failure
					//	this is less likely to happen. as during Dialing in line is always present
					LOG4CPLUS_FATAL(CONNECTOR_LOGGER,"UNEXPECTED!!!! A party Is missing when Dialling failure occurs");
					line.DropCall();
					return 0;
				}

				LOG4CPLUS_INFO(CONNECTOR_LOGGER,"Calling out call handling function.." << "gc" <<gc_result_info.gcValue << "cc" << gc_result_info.ccValue);

				//	this is still safe as i alredy checked HasCaller()     
				switch(HandleMakeCallFailure(gc_result_info.gcValue,gc_result_info.ccValue,cause,msg))
				{
				case 1:
					LOG4CPLUS_FATAL(CONNECTOR_LOGGER,"MakeCallFailureHandler gives a File to play" << msg);
					
					line.DropCall(); //	Drop Out line (Works bcos out line connection not established)

					//	Using play while dropping...
					ReturnControl(msg);
					break;

				default:
					line.DropCall();
					ReturnControl(cause);
				}
			}
			else
			{
				LOG4CPLUS_FATAL(CONNECTOR_LOGGER,"Disconnected Reason not detected ERr=")
				
				line.DropCall();

				ReturnControl();
			}
		}
		else	//	incomming line disconnected while dialing out
		{
			//	Drop the currently dialing out line
			LOG4CPLUS_INFO(CONNECTOR_LOGGER,"A party Disconnect during dialling...")
			
			GetLine().DropCall();

			ReturnControl();
		}
		break;
	case CON_CONFAILED:
		//	this is occcur when outline disconnection event recived after connection failure
		if(IsMyLine(line))
		{
			LOG4CPLUS_FATAL(CONNECTOR_LOGGER,"Outline Disconnnect after CON_FAILED,");
			//	testing...			
			//SetState(CON_OPEN);
			line.DropCall();
		}
		break;
	default:
		LOG4CPLUS_FATAL(CONNECTOR_LOGGER,"Disconnection on Unexpected state " << GetState() )
	}

	return 0;
}

int CCallConnector::out_CallStatus(int reason)
{
	LOG4CPLUS_INFO(CONNECTOR_LOGGER,__FUNCTION__);
	return 0;
}

bool CCallConnector::IsBusy(void)
{
	return HasCaller() || GetState() != CON_OPEN;
}

std::string CCallConnector::ToString(void)
{
	std::string ret = "Connector";
	
	//if(HasCaller())
	//	ret += IncommingLine().ToString();

	if(&GetLine() != NULL)
		ret += GetLine().ToString() ;
		
	return ret;
}

int CCallConnector::OnAnswerCallComplete(CLine& line)
{
	if(!IsMyLine(line))
	{
		//	Start Billing ( now dialog plays the C tone)
		LOG4CPLUS_DEBUG(CONNECTOR_LOGGER,"In line answered. play C Tone (dummy)");

		//	Start Timer for Auto disconnection
		SetTimer(m_nAvailableTime); // Time is in seconds

		//	Create timer for disconnection for expriy
	}
	return 0;
}

int CCallConnector::out_CallAlerting(CLine& line, int code)
{
	LOG4CPLUS_INFO(CONNECTOR_LOGGER,"Ringing Detected: " << code <<"," << line.Channel());
	
	if(HasCaller())
	{
		ListviewSetColumn(IncommingLine().Channel()-1,7,"Ringing:");
	}
	else
	{
		LOG4CPLUS_FATAL(CONNECTOR_LOGGER,"Ringing... in UNEXPECTED status" << GetState());
	}
	return 0;
}

CLine& CCallConnector::IncommingLine(void)
{
	if(!HasCaller())
	{
		LOG4CPLUS_FATAL(CONNECTOR_LOGGER,"CCallConnector query for caller while not Called" << GetLine().Channel());
		throw std::invalid_argument("Connector lost exception");
	}

	return SourceProcessor().IncommingLine();
}

//	TODO: Could this be handled by the default implementation???
void CCallConnector::OnDropCallComplete(CLine& line, METAEVENT& evt)
{
	if(IsMyLine(line))
	{
		//	outgoing line is relesed
		LOG4CPLUS_INFO(CONNECTOR_LOGGER,"Outline Drop complete" << line.Channel());
		//ListviewSetColumn( IncommingLine().Channel()-1,7,"Releasing...");
		line.ReleaseCall();
	}
}

void CCallConnector::OnReleaseCallComplete(CLine& line, METAEVENT& evt)
{
	if(!IsMyLine(line))
		return;

	LOG4CPLUS_INFO(CONNECTOR_LOGGER,"Outline Released and busy flag cleared" << line.Channel() );
	if(m_hTimer != NULL)
	{
		LOG4CPLUS_FATAL(CONNECTOR_LOGGER,"Timer is not well cleared on Release completion");
	}
	//	now line is well released. Ready for next OutCall
	SetState(CON_OPEN);
}

int CCallConnector::HandleMakeCallFailure(int reason, int cc_reason,int& cause,std::string& msg)
{
	//	defult dropping cause for all unhandled senarios
	cause = GC_USER_BUSY;

	switch(reason)
	{
	case GCRV_NORMAL:
		ListviewSetColumn(IncommingLine().Channel()-1,7,"NomalDcon");
		LOG4CPLUS_WARN(CONNECTOR_LOGGER, "Normal Disconnecton While dialing cc" << cc_reason );
		
		if(cc_reason == 0x10)
		{
			LOG4CPLUS_INFO(CONNECTOR_LOGGER,"Possible Call rejection");
			cause = GC_NORMAL_CLEARING;
		}
		break;

	case GCRV_TIMEOUT:
		LOG4CPLUS_INFO(CONNECTOR_LOGGER, "Make Call timeout" );
		ListviewSetColumn(IncommingLine().Channel()-1,7,"Timeout");
		cause = GC_USER_BUSY;
		break;
	case GCRV_REJECT:
	case GCRV_BUSY:

		ListviewSetColumn(IncommingLine().Channel()-1,7,"Busy");
		LOG4CPLUS_INFO(CONNECTOR_LOGGER, "B PARTY BUSY and record playied" );

		cause = GC_USER_BUSY;
		break;

	case GCRV_UNALLOCATED:
		//	invalid Tigo : 0724560294
		ListviewSetColumn(IncommingLine().Channel()-1,7,"Invalid");
		LOG4CPLUS_INFO(CONNECTOR_LOGGER, "B Number is invalid" );
		cause = GC_UNASSIGNED_NUMBER;
		
		break;
	case GCRV_NOANSWER:
		//	this happend when mobitel is switched off
		LOG4CPLUS_INFO(CONNECTOR_LOGGER, "Unhandled GCRV_NOANSWER" );
		break;

	case GCRV_CCLIBSPECIFIC:

		ListviewSetColumn(IncommingLine().Channel()-1,7,"SS7");
		LOG4CPLUS_ERROR(CONNECTOR_LOGGER, "SS7 specific reason Code=" << cc_reason );
		
		if(cc_reason == 0x313) // mobitel return thhis after call rejection/no answer with tone/no tone
		{
			//	Record should be "This number canot be contacted right now. please try again shortly"
			LOG4CPLUS_INFO(CONNECTOR_LOGGER, "Mobitel Reject/No Answer" << cc_reason );
		}
		else if(cc_reason == NO_CIRCUIT_AVAILABLE)
		{
			//	lankabell invalid numbers gives this
			cause = GC_UNASSIGNED_NUMBER;
		}
		else if(cc_reason == SEND_SIT)
		{
			cause = GC_SEND_SIT;
		}
		else if(cc_reason == UNSPECIFIED_CAUSE)
		{
			//	tempory disconnected suntel (REC): 0114527199
			//	invalid hutch (REC) : 0788679590
			//	disconnected citylink (No Record, a tone) : 0602075347
			//	disconnected lanka bell (REC) : 0115780315
			// no answer on Mobitel : 0714818711.
			cause = GC_SEND_SIT; // or something else
		}
		else if(cc_reason == 0x13 || cc_reason == INVALID_NUMBER_FORMAT)
		{
			//	0x13 given by NoAnswering SLT phone (No error info found. crazy SLT guys) ??? 
	
			// INVALID_NUMBER_FORMAT    was given by 033225674925 with recording
			cause = GC_SEND_SIT;
		}
		else
		{
			LOG4CPLUS_ERROR(CONNECTOR_LOGGER, "SS7 specific reason (Unhandled)" << cc_reason );
			cause = GC_NETWORK_CONGESTION;
		}
		break;

	default:
		//	Call State Change. (play busy tone on this senario)
		ListviewSetColumn(IncommingLine().Channel()-1,7,"EXCEPTION");
		LOG4CPLUS_FATAL(CONNECTOR_LOGGER, "Unknown Out Call failure GC=" << reason << ",CC=" << cc_reason);
	}
	
	return 0;
}

void CCallConnector::OnResetComplete(CLine& line, METAEVENT& evt)
{
	//	now line is well released. Ready for next OutCall
	SetState(CON_OPEN);
	LOG4CPLUS_INFO( CONNECTOR_LOGGER,"Outline Reset complete and busy flag cleared " << line.Channel() );
}

void CCallConnector::OnDialing(CLine& line, METAEVENT& evt)
{
	if(HasCaller())
	{
		LOG4CPLUS_INFO( CONNECTOR_LOGGER,"Half duplex connection made on GCEV_DIALING");
		
	//	Let Caller to hear everything going on the outgoing line (make voice device to detach)
		nr_scroute(GetLine().GetNetworkH(),SC_DTI,IncommingLine().GetNetworkH(),SC_DTI,SC_HALFDUP);
	}
	else
	{
		LOG4CPLUS_ERROR( CONNECTOR_LOGGER, "GCEV_DIALING without Caller");
	}
}

void CCallConnector::OnUserEvent(CLine& line, int code)
{
	LOG4CPLUS_ERROR( CONNECTOR_LOGGER, "Caller Time is over");
	//	this event generated when timer expired
	if(GetState() == CON_CONNECTED && HasCaller())
	{
		LOG4CPLUS_INFO( CONNECTOR_LOGGER, "Disconnected by system Ch:" << line.Channel());

		GetLine().DropCall();	//	Drop out call

		SetState(CON_FINISHED);

		ReturnControl(GC_SEND_SIT);
	}
}

void CCallConnector::OnUnblocked(CLine& line, METAEVENT& evt)
{
	if(line.WaitCall() == 0)
	{
		LOG4CPLUS_INFO( CONNECTOR_LOGGER, "Experimental WaitCall on OUT line");
		SetState(CON_OPEN);
	}
}

void CCallConnector::SetTimer(int seconds)
{
	create_timer(&m_hTimer,seconds*1000,&GetLine());
}

void CCallConnector::CancelTimer(void)
{
	if(cancel_timer(m_hTimer))
	{
		m_hTimer = NULL;
	}
}
