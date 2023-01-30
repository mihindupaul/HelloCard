#include "stdafx.h"
#include <fstream>
#include <ctime>
#include <algorithm>
#include <functional>
#include <sstream>
#include "./line.h"
#include "terminationparam.h"
#include ".\line.h"

CLine::CLine(CLineManager* manager) 
: m_pLineManager(manager)
, state(GCST_IDLE)
, channel(0)
, voiceh(NULL)
, timesloth(NULL)
, m_fDropInProgress(false)
{
	//	Setup all technology make call block
	m_ObserverList.clear();

	//	Init Make Call Blocks
	::memset(&makecallblk,0,sizeof(MAKECALL_BLK));
	::memset(&gclib_makecall,0,sizeof(GCLIB_MAKECALL_BLK));

	i = m_ObserverList.begin();
}

CLine::~CLine(void)
{
	Close();
}

int CLine::Reset()
{
	LOG4CPLUS_WARN(LINE_LOGGER, "Line Reset ch=" << channel)

	if(gc_ResetLineDev(ldev, EV_ASYNC) != GC_SUCCESS) 
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"gc_ResetLineDev() Failed");
		return -1;
	}

	//	FIXME: On reset device do we need to do this again?
	if (nr_scroute(voiceh,SC_VOX , timesloth, SC_DTI,SC_FULLDUP) == -1)
	{
		return -1;	
	}

	return 0;
}

int CLine::Close()
{
	//	TODO: Makesure nr_scunroute is called on voice/network handles on closing.

	//	line must be in null state before this function call. this is always sync operation
	if(state == GCST_NULL)
	{
		return gc_Close(ldev);
	}
	else
	{
		if(crn != 0 && gc_DropCall(ldev,crn,EV_SYNC) == GC_SUCCESS)
		{
			LOG4CPLUS_ERROR(LINE_LOGGER,"line dropped before close");
			//	may be we can perfom release hear
		}
		else
		{
			//	put the line in to NULL state immediately
			if(gc_ResetLineDev(ldev,EV_SYNC) != GC_SUCCESS)
			{
				LOG4CPLUS_ERROR(LINE_LOGGER,"Line Not Null when close"); 
			}
		}
		gc_Close(ldev);
	}
	return -1;
}

int CLine::DropCall(int cause)
{
	LOG4CPLUS_INFO(LINE_LOGGER,"DropCall() called " << channel << "crn" << crn);
	
	dx_stopch(voiceh,EV_SYNC);

	switch(state)
	{
	case GCST_NULL:
		LOG4CPLUS_INFO(LINE_LOGGER,"DropCall() in GCST_NULL");
		break;
	case GCST_OFFERED:
	case GCST_ACCEPTED:
	case GCST_DIALING:
		//	First call answer to avoid drop call problem
		LOG4CPLUS_INFO(LINE_LOGGER,"Dropping un connected line" << channel << "state=" << state);

		m_fDropInProgress = true;

		if( gc_DropCall( crn, cause, EV_ASYNC) != GC_SUCCESS )
		{
			LOG4CPLUS_ERROR(LINE_LOGGER,"gc_DropCall Failed" << channel);
			return -1;
		}

		break;
	case GCST_DISCONNECTED:
	case GCST_CONNECTED:
		//	Attempt Dropping call
		m_fDropInProgress = true;
		if( gc_DropCall( crn, cause, EV_ASYNC) != GC_SUCCESS )
		{
			LOG4CPLUS_ERROR(LINE_LOGGER,"gc_DropCall Failed" << channel);
			return -1;
		}
		break;

	default:
		LOG4CPLUS_WARN(LINE_LOGGER,"DropCall NOT initiated.Attempt Reset..." << channel << "-" << state);
		this->Reset();
	}

	return 0;
}

int CLine::ProcessGlobalCallEvent(METAEVENT* pMetaEvent)
{
	GC_INFO	gc_result_info;

	crn = pMetaEvent->crn;

	LOG4CPLUS_INFO(LINE_LOGGER,"Processing GC Event" << pMetaEvent->evttype << " On " << channel);
	
	ClearMarkedObservers();
	
	//	GlobelCall Event handling is done hear.
	switch (pMetaEvent->evttype)
	{
	case GCEV_OPENEX:
		OnOpened(*pMetaEvent);
		break;

	case GCEV_BLOCKED:
		LOG4CPLUS_ERROR(LINE_LOGGER,"GCEV_BLOCKED");
		ListviewSetColumn( channel-1,6,"GCEV BLOCKED");
		state = ST_BLOCK;
		break;

	case GCEV_UNBLOCKED:
		OnUnblocked(*pMetaEvent);
		//	still GCST_NULL
		break;	

	case GCEV_RELEASECALL : //	Release call completed.
		OnReleaseCallComplete(*pMetaEvent);
		break;

	case GCEV_RELEASECALL_FAIL:	//	Release call failed
		LOG4CPLUS_ERROR(LINE_LOGGER,"Relase Call Failed. Attempt line reset on " << channel);
		Reset();	//	if call cant be released. now time to reset this line
		break;

	case GCEV_DROPCALL:	//	Drop call Completed
		OnDropCallComplete(*pMetaEvent);
		break;
	
	case GCEV_RESETLINEDEV:	//	Reset Line completed
		OnResetComplete(*pMetaEvent);
		break;

	case GCEV_RESTARTFAIL:
		//	this is critical. Reset failed. need manual attention.
		LOG4CPLUS_FATAL(LINE_LOGGER,"Line Reset Failed" << channel);
		break;

	case GCEV_DISCONNECTED:
		OnDisconnected(pMetaEvent);
		break;

	case GCEV_TASKFAIL:
		OnTaskFail(pMetaEvent);
		break;

	case GCEV_OFFERED: 
		OnCallOffered(*pMetaEvent);
		break;

	case GCEV_ACCEPT:
		OnAcceptCallComplete(pMetaEvent->crn);
		break;

	case GCEV_CONNECTED :
		OnMakeCallConnected(pMetaEvent->crn);
		break;
	
	case GCEV_ERROR:
		LOG4CPLUS_DEBUG(LINE_LOGGER,"GCEV_ERROR event");
		break;

	case GCEV_ANSWERED:
		OnAnswerCallComplete(*pMetaEvent);
		break;

	case GCEV_DIALING:
		OnDialing(*pMetaEvent);
		break;

	//	make call specific events
	case GCEV_CALLSTATUS:
		LOG4CPLUS_INFO(LINE_LOGGER,"GCEV_CALLSTATUS");
		if (gc_ResultInfo(pMetaEvent,&gc_result_info) == GC_SUCCESS)
		{  
			LOG4CPLUS_INFO(LINE_LOGGER,"GCEV_CALLSTATUS" << gc_result_info.ccValue);
			OnMakeCallStatus(pMetaEvent->crn,gc_result_info.gcValue);
		}
		//	Release and Drop call
		break;

	case GCEV_ALERTING:
		OnMakeCallAlerting(*pMetaEvent);
		break;

	case GCEV_MEDIADETECTED:
		LOG4CPLUS_INFO(LINE_LOGGER,"GCEV_MEDIADETECTED");
		break;
	case GCEV_PROCEEDING:
		LOG4CPLUS_INFO(LINE_LOGGER,"GCEV_PROCEEDING");
		break;
	case GCEV_HOLDCALL:
		OnHoldCall(*pMetaEvent);
		break;
	case GCEV_SETCHANSTATE:
		LOG4CPLUS_INFO(LINE_LOGGER,"GCEV_SETCHANSTATE");
		OnSetChanState(*pMetaEvent);
		break;
	default:
		LOG4CPLUS_INFO(LINE_LOGGER,"Unhandled GC Event" << channel << pMetaEvent->evttype);
	}
	return 0;
}

void CLine::OnDisconnected(METAEVENT* pMetaEvent)
{
	LOG4CPLUS_INFO(LINE_LOGGER,"Disconnected Event " << channel);

	try
	{
		for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
		{
			(*i)->OnCallDisconnected(*this,*pMetaEvent);
		}
	}
	catch(...)
	{
		LOG4CPLUS_INFO(LINE_LOGGER,"DISASTER !!! !! !!! !! ! !! !! ! " << channel);
	}
}

//	incomming call is detected.
void CLine::OnCallOffered(METAEVENT& evt)
{
	state = GCST_OFFERED;
	LOG4CPLUS_INFO(LINE_LOGGER,"Call Offered Event " << channel);
	
	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		//	notify all observers about the event
		(*i)->OnCallOffered(*this,evt);
	}
}

void CLine::OnAcceptCallComplete(CRN crn)
{
	//	Call is accepted but not answered. so billing not started. but IVR can talk to this
	//	Notify the observer for start of conversation
	if(state == GCST_OFFERED)
	{
		state = GCST_ACCEPTED;
		for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
		{
			//	notify all observers about the event
			(*i)->OnAcceptCallComplete();
		}
	}
}

int CLine::OnPlayComplete(long reason)
{
	LOG4CPLUS_ERROR(LINE_LOGGER,"compleation of handle=" << iott[0].io_fhandle);

	//	Close the plaied file
	if(iott[0].io_fhandle != NULL)
	{
		dx_fileclose(iott[0].io_fhandle);
	}

	//	notify all observers
	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		//	notify all observers about the event
		(*i)->OnPlayCompleted(*this,reason);
	}

	return 0;
}


void CLine::OnGetDigitComplete(METAEVENT& evt)
{
	std::stringstream digits;
	int reason = (int) ATDX_TERMMSK(evt.evtdev);
	
	//	extract the digits from buffer
	digits << dtbuf.dg_value;

	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		(*i)->OnGetDigitComplete(*this,reason,digits.str());
	}
}

//	This function process NON-GC events.
int CLine::ProcessEvent(METAEVENT* pMetaEvent)
{
	//Obtain last termination event for this timeslot.
	int data = 0;
	
	LOG4CPLUS_INFO(LINE_LOGGER,"Processing VOICE Event" << pMetaEvent->evttype << " On " << channel);
	
	ClearMarkedObservers();

	switch(pMetaEvent->evttype)
	{
	case TDX_PLAY:
		data = (int) ATDX_TERMMSK(pMetaEvent->evtdev);
		OnPlayComplete(data);
		break;
	case TDX_GETDIG:
		OnGetDigitComplete(*pMetaEvent);
		break;
	case TDX_PLAYTONE:
		OnPlayToneComplete(*pMetaEvent);
		break;
	case 556:
		OnUserEvent(*pMetaEvent);
		break;
	default:
		LOG4CPLUS_INFO(LINE_LOGGER,"un-processed non-gc event" << (DWORD)pMetaEvent->evttype << __FUNCTION__);
	}

	return 0;
}

int CLine::OnTaskFail(METAEVENT* pMetaEvent)
{
	ListviewSetColumn( channel-1,6,"GCEV_TASKFAIL");
	
	LOG4CPLUS_ERROR(LINE_LOGGER,"Task failed in state:" << state);
	
	switch(state)
	{
	case GCST_OFFERED:
		//	gc_AcceptCall Failure
		LOG4CPLUS_ERROR(LINE_LOGGER,"gc_AcceptCall() Failed");
		break;
	case GCST_ACCEPTED:	//	Accepted state
		//	Could be AnswerCall failure
		LOG4CPLUS_ERROR(LINE_LOGGER,"gc_AnswerCall() Failed");
		break;
	case GCST_DISCONNECTED:
		//	gc_DropCall Failed
		break;
	default:
		LOG4CPLUS_ERROR(LINE_LOGGER,"OnTask Fail Called" << state  );
		Reset();	//	Handle unkown task failures with auto line reset
	}
	return 1;
}

void CLine::ClearDigitBuffer(void)
{
	if(dx_clrdigbuf(voiceh) != GC_SUCCESS)
	{
		 LOG4CPLUS_ERROR(LINE_LOGGER, __FUNCTION__ << ATDV_ERRMSGP(voiceh));
	}
}

int CLine::AnswerCall(void)
{
	LOG4CPLUS_INFO(LINE_LOGGER,"AnswerCall Called " << channel);

	if (gc_AnswerCall(crn, 0, EV_ASYNC) != GC_SUCCESS) 
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"gc_AnswerCall Failed");
		return -1;
	}
	return 0;
}

int CLine::OnAnswerCallComplete(METAEVENT& evt)
{
	if(state == GCST_OFFERED || state == GCST_ACCEPTED)
	{
		state = GCST_CONNECTED;
		LOG4CPLUS_INFO(LINE_LOGGER,"Answer Call completed");
		
		//	notify all observers about the event
		for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
		{
			(*i)->OnAnswerCallComplete(*this);
		}
	}
	else
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"Answer call comleted in wrong state");
	}
	return 0;
}

void CLine::SetTrace(bool value)
{
	char file_name[MAX_PATH];

	sprintf(file_name,"trace_ss7.log",channel-1);

	if(value)
	{
		gc_StartTrace(ldev,file_name);
	}
	else
	{
		gc_StopTrace(ldev);
	}
}
/*
//
//	Observer Operations
//
int CLine::AddLineObserver(ILineObserver* pObserver)
{
	//	Add the Observer to my observer list
	assert(m_ObserverList.size()<=3);	// it is unlike to have more than 2 observer per line

	m_ObserverList.push_front(pObserver);

	LOG4CPLUS_INFO(LINE_LOGGER,"Observer Added n=" << m_ObserverList.size() << ".." << channel << ":" <<pObserver); 
	return 0;
}

int CLine::RemoveLineObserver( ILineObserver* pObserver)
{
	LOG4CPLUS_INFO(LINE_LOGGER,__FUNCTION__ << pObserver);

	//	Observer Self Removal
	if(m_pObserverLock == NULL || m_pObserverLock == pObserver)
	{
		LOG4CPLUS_INFO(LINE_LOGGER,"Observer Marked for Removing..." << pObserver << "Ch:" << channel);
		m_pObserverLock = pObserver;
	}
	else
	{
		//	This should Never been called
		LOG4CPLUS_FATAL(LINE_LOGGER,"NEVER: Recursive Observer removal Old:" << m_pObserverLock << "New:" << pObserver << "Ch:" << channel);
	}
	return 0;
	
	////	this could be called by my pObserver function itself. we must remove it from the list AND
	////	set the robust iterator to point next value
	//tmpIterator = find(m_ObserverList.begin(),m_ObserverList.end(),pObserver);
	//
	////	this observer not added.
	//if(tmpIterator == m_ObserverList.end())
	//	return -1;

	////	Check for next available observer.
	//if(tmpIterator++ != m_ObserverList.end())
	//{
	//	//	if NEXT exist adjust common iterator to point it after deletion
	//	m_ObserverList.remove(pObserver);
	//	
	//	LOG4CPLUS_INFO(LINE_LOGGER,"Non Last iterator deletion *i=" << *i << "nxt" << *tmpIterator << "pOb" << pObserver);
	//	
	//	if(*i != pObserver)
	//		i = tmpIterator;
	//	//i--;	//	my iterating for loop will reset this value to tmpIterator.

	//	LOG4CPLUS_INFO(LINE_LOGGER,"Non Last iterator deletion" << *i);
	//}
	//else
	//{
	//	//	We are removing LAST observer in the list. 
	//	m_ObserverList.remove(pObserver);
	//	i--;// = m_ObserverList.begin();
	//	LOG4CPLUS_INFO(LINE_LOGGER,"Last iterator deletion" << *i);
	//}

	//LOG4CPLUS_INFO(LINE_LOGGER,"Observer Removed n=" << m_ObserverList.size() << ".." << channel << ".." << pObserver);
	//return 0;
}
*/
CLineManager& CLine::GetManager() const
{
	return *m_pLineManager;
}

LINEDEV& CLine::GetVoiceH() 
{
	return voiceh;
}

std::string CLine::ToString()
{
	std::stringstream s;

	s << "Line: " << channel  << m_sDeviceName;

	return s.str();
}

int CLine::WaitCall()
{
	LOG4CPLUS_INFO(LINE_LOGGER,"WaitCall called on " << channel);

	if (gc_WaitCall( ldev, NULL, NULL, 0, EV_ASYNC) != GC_SUCCESS)
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"Wait call failed" << GetLastErrorDetails());
		return -1;
	}
	return 0;
}

std::string CLine::GetLastErrorDetails()
{
	GC_INFO tmp;
	std::ostringstream err_msg;

	if(gc_ErrorInfo(&tmp) == GC_SUCCESS)
	{
		err_msg << " Info:(" << channel << ") " << tmp.gcMsg << tmp.gcValue;
		return err_msg.str();
	}
	return std::string("Could not Retrive Error details");
}

void CLine::OnDropCallComplete(METAEVENT& evt)
{
	state = GCST_IDLE;

	LOG4CPLUS_INFO(LINE_LOGGER,"Drop call completed " << channel);

	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		(*i)->OnDropCallComplete(*this,evt);
	}
}

int CLine::OnMakeCallStatus(CRN crn,int err)
{
	LOG4CPLUS_INFO(LINE_LOGGER,"Make call status" << err);

	switch(err)
	{
	case GCRV_NORMAL:
		return 0;
		break;
	case GCRV_NOANSWER:

//		state = ST_NOANSWER;
		ListviewSetColumn(channel-1,6,"No Answer");
		break;

	case GCRV_TIMEOUT:
//		state = ST_TIMEOUT;
		//				ListviewSetColumn(plinei->channel-1,6,"Timeout");
		break;

	default:
		//	Some other failure. may be conjession.
		LOG4CPLUS_ERROR(LINE_LOGGER,err);
	}
	return 0;
}

//	Notification Event. 
int CLine::OnMakeCallAlerting(METAEVENT& evt)
{
	state = GCST_DIALING;

	LOG4CPLUS_INFO(LINE_LOGGER,"Call Allerting.." << channel);

	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		//	notify all observers about the event
		(*i)->out_CallAlerting(*this,0);
	}

	return 0;
}

int CLine::OnMakeCallConnected(CRN crn)
{
	state = GCST_CONNECTED;

	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		//	notify all observers about the event
		(*i)->out_CallConnected(this);
	}
	return 0;
}

CLineManager& CLine::GetLineManager(void)
{
	return *m_pLineManager;
}

int CLine::OnReleaseCallComplete(METAEVENT& evt)
{
	// Set The State to ST_NULL
	state = GCST_NULL;
	m_fDropInProgress = false;

	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		//	notify all observers about the event
		(*i)->OnReleaseCallComplete(*this,evt);
	}

	return 0;
}

int CLine::SetVoice(bool enable)
{
	//	Enable/disable voice capability
	if(enable)
	{
		if(nr_scroute(GetVoiceH(),SC_VOX,GetNetworkH(),SC_DTI,SC_FULLDUP) == -1)
		{
			LOG4CPLUS_ERROR(LINE_LOGGER,"nr_scroute" << __FUNCTION__);
			return -1;
		}
	}
	else
	{
		if (nr_scunroute(GetVoiceH(),SC_VOX,GetNetworkH(),SC_DTI,SC_FULLDUP) == -1)
		{
			LOG4CPLUS_ERROR(LINE_LOGGER,"nr_scunroute"  << __FUNCTION__);
			return -1;
		}
	}
	return 0;
}

int CLine::OnDialing(METAEVENT& evt)
{
	state = GCST_DIALING;
	
	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		//	notify all observers about the event
		(*i)->OnDialing(*this,evt);
	}
	return 0;
}

int CLine::GetState(void)
{
	return state;
}

void CLine::StopPlay(void)
{
	if(dx_stopch(voiceh,EV_SYNC)!=GC_SUCCESS)
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,__FUNCTION__ << ATDV_ERRMSGP(voiceh));
	}
}

int CLine::EnableCPA(void)
{
	GC_PARM gcParm;
//	int t;

	memset(&gcParm,0,sizeof(gcParm));

	gcParm.intvalue = GCPV_ENABLE;

	//if ((t=gc_SetParm( ldev, GCPR_CALLPROGRESS, gcParm)) != GC_SUCCESS)
	//{
	//	LOG4CPLUS_ERROR(LINE_LOGGER,"gc_SetParm(GCPR_CALLPROGRESS)" << (DWORD)t)
	//	return -1;
	//}

	if (gc_SetParm( ldev , GCPR_MEDIADETECT, gcParm) != GC_SUCCESS)
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"gc_SetParm(GCPR_MEDIADETECT)");
		return -1;
	}

	gc_SetEvtMsk(ldev,GCMSK_PROCEEDING,GCACT_ADDMSK);

	return 0;
}

int CLine::OnOpened(METAEVENT& metaevent)
{
	//	Enable CPA on this channel
//	EnableCPA();

	//	open the networking timeslot
	if(gc_GetResourceH(ldev, (int*)&timesloth,GC_NETWORKDEVICE)!= GC_SUCCESS) 
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"Open NetworkH");
		return -1;
	}

	//	Open the VOICE/NETWORK device for 
	if(gc_GetResourceH(ldev,(int*)&voiceh,GC_VOICEDEVICE)!= GC_SUCCESS)
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"Open VoiceH");
		return -1;
	}

	if (sr_setparm(voiceh,SR_USERCONTEXT ,this) == -1)
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"sr_setparm");
		//ErrMsg( VoxDevice(), "sr_setparm");
	}

	//	on open it is in NULL state
	state = GCST_NULL;

	if(gc_SetEvtMsk(ldev,GCMSK_DIALING,GCACT_ADDMSK) != GC_SUCCESS)
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"Dialing event mask");
	}

	return 0;
}

LINEDEV& CLine::GetNetworkH(void)
{
	return this->timesloth;
}

int CLine::ReleaseCall(void)
{
	//	we must always call release call after drop call
	LOG4CPLUS_INFO(LINE_LOGGER,"ReleaseCall() " << channel);

	if (gc_ReleaseCallEx(crn, EV_ASYNC) != GC_SUCCESS) 
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"gc_ReleaseCallEx() Failed");
		return -1;
	}

	return 0;
}

int CLine::Channel(void)
{
	return channel;
}

bool CLine::IsConnected(void)
{
	return false;
}

int CLine::OnPlayToneComplete(METAEVENT& evt)
{
	LOG4CPLUS_INFO(LINE_LOGGER,"Play Tone Completed");
	return 0;
}

void CLine::OnHoldCall(METAEVENT& evt)
{
	LOG4CPLUS_INFO(LINE_LOGGER,"GCEV_HOLDCALL " << channel);
}

void CLine::OnResetComplete(METAEVENT& evt)
{
	state = GCST_NULL;	

	LOG4CPLUS_INFO(LINE_LOGGER,"RESET completed" << channel);

	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		//	notify all observers about the event
		(*i)->OnResetComplete(*this,evt);
	}
}

int CLine::Play(const CTone& tone, const TPT& tpt, int mode)
{
	int r;

	if(tone.IsCandence())
	{
		r = dx_playtoneEx(voiceh, &tone.GetCandence(), tpt, mode);
	}
	else
	{
		r = dx_playtone( voiceh, &tone.GetTone(), tpt, mode );
	}
	
	if(r == -1)
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"Play Tone Failed" << ATDV_LASTERR(voiceh));
		return -1;
	}

	return 0;
}

int CLine::Play(const std::string& path, const TPT& tpt)
{
	int fh=-1;

	if((fh=dx_fileopen(path.c_str(),O_RDONLY|O_BINARY))==-1)
	{  
		LOG4CPLUS_ERROR(LINE_LOGGER,"File open error");
	   return 1;
	}

	//Clear and Set-Up the IOTT strcuture
	::memset(&iott,0,sizeof(DX_IOTT));
	
	iott[0].io_type		= IO_DEV | IO_EOT;
	iott[0].io_length	= -1;	// unill the end of file

	iott[0].io_fhandle	= fh;
	
	LOG4CPLUS_ERROR(LINE_LOGGER,"playing handle" << fh);

	//Play VOX File on D/4x Channel, Normal Play Back
	if(dx_play(voiceh,iott,tpt,EV_ASYNC|PM_SR8 ) != GC_SUCCESS)
	{
		LOG4CPLUS_FATAL(LINE_LOGGER,"Play File error:" << ATDV_ERRMSGP(voiceh));
		return -1;
	}
	return 0;
}

int CLine::AcceptCall(int rings)
{
	return gc_AcceptCall(crn, rings, EV_ASYNC); 
}

int CLine::SetChannelState(int state)
{
	return gc_SetChanState(ldev, state, EV_ASYNC);
}

int CLine::OnSetChanState(METAEVENT& evt)
{
	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		//	notify all observers about the event
		(*i)->OnSetChanState(*this,evt);
	}
	return 0;
}

void CLine::OnUnblocked(METAEVENT& evt)
{
	LOG4CPLUS_INFO(LINE_LOGGER,"GCEV_UNBLOCKED");

	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		//	notify all observers about the event
		(*i)->OnUnblocked(*this,evt);
	}
}

int CLine::GetDigit(const TPT& tpt, int mode)
{
	return dx_getdig(voiceh,tpt,&dtbuf,EV_ASYNC);
}

int CLine::AddUserEvent(int code)
{
	if(sr_putevt(voiceh, 556 , sizeof(int),&code, 0) != GC_SUCCESS)
	{
		LOG4CPLUS_INFO(LINE_LOGGER,"Add User event failed");
		return -1;
	}
	return 0;
}

void CLine::OnUserEvent(METAEVENT& evt)
{
	LOG4CPLUS_INFO(LINE_LOGGER,"User event" << *(int*)evt.evtdatap);

	for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
	{
		//	notify all observers about the event
		(*i)->OnUserEvent(*this,*(int*)evt.evtdatap);
	}

	//	Testing
	//for_each(m_ObserverList.begin(),m_ObserverList.end(),std::bind2nd(std::mem_fun(&ILineObserver::out_CallStatus),2));
}
