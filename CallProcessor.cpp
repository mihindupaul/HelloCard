#include "stdafx.h"
#include ".\callprocessor.h"

CCallProcessor::CCallProcessor(void)
: m_nCallFlowState(1) // : m_nCallFlowState(CFS_BLOCKED)
, m_fSuspend(false)
, m_pSourceProcessor(NULL)
, m_CurrentLanguage(CConfiguration::ENGLISH)
, m_nLastPlayDropCause(0)
, m_fGetDigitsBusy(false)
, m_fPlayFileBusy(false)
, m_fDropAfterVoiceCompletion(false)
{
}

CCallProcessor::~CCallProcessor(void)
{
}
//	End the call
int CCallProcessor::Exit(int cause)
{
	LOG4CPLUS_INFO(IVR_LOGGER,"Dropping with cause" << cause);
	//	Simpley end the IVR flow with DropCall.
	m_nCallFlowState = CFS_BLOCKED;
	ListviewSetColumn(IncommingLine().Channel()-1,6,"Dropping...");
	IncommingLine().DropCall(cause);
	return 0;
}

//	End the call with message
int CCallProcessor::Exit(std::string last_msg,int cause)
{
	//	Exit the IVR call flow, after playing 'msg'
	LOG4CPLUS_INFO(IVR_LOGGER,"Exit Requested. Last State=" << m_nCallFlowState << ",Last Msg=" << last_msg);
	
	m_nCallFlowState = CFS_LAST_PLAY;

	m_nLastPlayDropCause = cause;
	
	LOG4CPLUS_INFO(IVR_LOGGER,"Befor Display X");
	
	ListviewSetColumn(IncommingLine().Channel()-1,6,"Dropping...");
	
	LOG4CPLUS_INFO(IVR_LOGGER,"Befor StopPlay");

//	IncommingLine().StopPlay();	//	stop progress play
	IncommingLine().SetVoice(true);	//	i want to play a message

	PlayFile(last_msg);
	
	return 0;
}

// Transfer the control of one call processor to another
int CCallProcessor::Transfer(CCallProcessor& source)
{
	//	After suspend, Source processor doesnt process any events though recive.
	//	The Current processor is responsible for all event handlings
	if(m_pSourceProcessor == NULL)
	{
		m_pSourceProcessor = &source;	//	Keep reference to SOURCE for future references

		source.Suspend();
		//	inherit the language 
		this->m_CurrentLanguage = source.m_CurrentLanguage;
		//	This becomes a observer of source's line
		source.IncommingLine().AddLineObserver(this);
		//	From this point onword i am getting all events for SOURCE
		//  .....
	}
	else
	{
		//	Processor Trasfer failed
		LOG4CPLUS_INFO(IVR_LOGGER,"IVR Transfer Failed");
		return -1;
	}
	return 0;
}

void CCallProcessor::Suspend(void)
{
	m_fSuspend = true;
	LOG4CPLUS_INFO(LINE_LOGGER,"IVR Suspended " << IncommingLine().Channel());
}

void CCallProcessor::Resume(void)
{
	m_fSuspend = false;
	LOG4CPLUS_INFO(LINE_LOGGER,"IVR Resumed " << IncommingLine().Channel());
}

void CCallProcessor::Reset(void)
{
	m_fSuspend = false;
	m_nCallFlowState = CFS_BLOCKED;
	LOG4CPLUS_DEBUG(IVR_LOGGER,"Processor Reset:" << IncommingLine().Channel());
}

CCallProcessor& CCallProcessor::SourceProcessor(void)
{
	if(m_pSourceProcessor == NULL)
		return *this;

	return *m_pSourceProcessor;
}

int CCallProcessor::OnPlayCompleted(CLine& line, int reason)
{
	m_fPlayFileBusy = false;

	if(reason == TM_USRSTOP)
	{
		LOG4CPLUS_INFO(IVR_LOGGER,"PlayCompleted() stop by user");
	}

	if(m_fDropAfterVoiceCompletion)
	{
		LOG4CPLUS_INFO(IVR_LOGGER,"the line must drop now" << __FUNCTION__);
		
		m_fDropAfterVoiceCompletion = false;
		
		//line.DropCall();
	}

	if(line.m_fDropInProgress)
	{
		LOG4CPLUS_INFO(IVR_LOGGER,"Play complete event while drop in progress ignored");
		return 0;
	}

	if(m_nCallFlowState == CFS_LAST_PLAY)	//	plaing last message is done
	{
		Reset();	//	Reset IVR for next execution

		LOG4CPLUS_INFO(IVR_LOGGER,"Last message play complete:" << line.Channel() << "reason=" << reason);
		
		//	Drop the line with the provided Cause value
		line.DropCall(m_nLastPlayDropCause);
	}
	else if( m_nCallFlowState != CFS_BLOCKED) //	or CFS_STOPPED
	{	
		try
		{
			LOG4CPLUS_INFO(IVR_LOGGER,"Play Completed On State:" << m_nCallFlowState);
			OnPlayFileTerminate(reason);
		}
		catch(...)
		{
			LOG4CPLUS_FATAL(IVR_LOGGER,"PlayFileTerminat() IVR Exception");
		}
	}
	else
	{
		LOG4CPLUS_ERROR(IVR_LOGGER,__FUNCTION__ << "Play Complete While " << m_nCallFlowState);
	}
	return 0;
}

void CCallProcessor::SetState(int state)
{
	if(state >= CFS_USER)
	{
		m_nCallFlowState = state;
		OnStateChange(state);
	}
	else
	{
		LOG4CPLUS_FATAL(IVR_LOGGER,"invalid Callflow user state");
	}
}

int CCallProcessor::OnCallDisconnected(CLine& line, METAEVENT& evt)
{
	//	Disconnection Event of IVR
	LOG4CPLUS_FATAL(IVR_LOGGER,__FUNCTION__);
	return 0;
}

bool CCallProcessor::HasCaller(void)
{
//	LOG4CPLUS_DEBUG(IVR_LOGGER,"Has Caller" << m_pSourceProcessor);
	return m_pSourceProcessor  != NULL;
}

// Return the control to calling (source) CCallprocessor
void CCallProcessor::ReturnControl(int cause)
{
	LOG4CPLUS_INFO(IVR_LOGGER,__FUNCTION__);

	if(HasCaller())
	{
		IncommingLine().RemoveLineObserver(this);
		SourceProcessor().Resume();
		SourceProcessor().Exit(cause);
		m_pSourceProcessor = NULL;
	}
	else
	{
		LOG4CPLUS_FATAL(IVR_LOGGER,__FUNCTION__ << "Called but no Caller to return");
	}
}

void CCallProcessor::OnPlayFileTerminate(int reason)
{

}

int CCallProcessor::GetState(void)
{
	return m_nCallFlowState;
}

void CCallProcessor::OnStateChange(int state)
{
	LOG4CPLUS_INFO(IVR_LOGGER,"Processor State changed:" << state );
}

bool CCallProcessor::IsSuspended(void)
{
	return m_fSuspend;
}

void CCallProcessor::ReturnControl(std::string last_msg)
{
	LOG4CPLUS_INFO(IVR_LOGGER,__FUNCTION__);

	if(HasCaller())
	{
		IncommingLine().RemoveLineObserver(this);
		
		SourceProcessor().Resume();

		SourceProcessor().Exit(last_msg);

		m_pSourceProcessor = NULL;
	}
	else
	{
		LOG4CPLUS_FATAL(IVR_LOGGER,__FUNCTION__ << "Called but no Caller to return");
	}
}

//	convinient function for playing file and goto next state
void CCallProcessor::PlayFile(std::string& file, int next_state)
{
	PlayFile(file);
	SetState(next_state);
}

void CCallProcessor::OnGetDigitsTerminate(int reason,std::string digits)
{
	LOG4CPLUS_FATAL(IVR_LOGGER,__FUNCTION__);
}

void CCallProcessor::OnGetDigitComplete(CLine& line, int reason,const std::string& digits)
{
	LOG4CPLUS_INFO(IVR_LOGGER,__FUNCTION__ << digits);
	
	//	seting this prior to OnGetDigitsTerminate() make sub class to call getdigits again
	m_fGetDigitsBusy = false;

	if(reason == TM_USRSTOP)
	{
		LOG4CPLUS_INFO(IVR_LOGGER,"GetDigit() stop by user");
	}

	if(m_fDropAfterVoiceCompletion)
	{
		LOG4CPLUS_INFO(IVR_LOGGER,"the line must drop now" << __FUNCTION__);
		m_fDropAfterVoiceCompletion = false;
		//line.DropCall();
	}
	try
	{
		if(m_nCallFlowState != CFS_BLOCKED && !line.m_fDropInProgress)
		{
			OnGetDigitsTerminate(reason,digits);
		}
		else
		{
			LOG4CPLUS_INFO(IVR_LOGGER,"GetDigit() Completion ignored");
		}
	}
	catch(...)
	{
		LOG4CPLUS_FATAL(IVR_LOGGER,"GetDigit() IVR Exception");
	}
}

// conviniant function for simple digit value get
int CCallProcessor::GetDigits(int max_len, int terminate_dig, int timeout)
{
	try
	{
	TPT tp;

	LOG4CPLUS_INFO(IVR_LOGGER,"GetDigits Request len=" << (DWORD)max_len); 
	
	if(IsVoiceBusy())
	{
		LOG4CPLUS_ERROR(IVR_LOGGER, "Voice Device is busy");
		return -1;
	}
	
	tp  << TPT::TP(DX_MAXDTMF,TF_MAXDTMF,max_len) 
		<< TPT::TP(DX_MAXTIME,TF_MAXTIME,timeout)
		<< TPT::TP(DX_IDDTIME,TF_IDDTIME,50)	//	inter digit delay 7s after first digit
		<< TPT::TP(DX_DIGMASK,TF_DIGMASK,terminate_dig);

		IncommingLine().ClearDigitBuffer();

		//	Clear any availble digits
		if(IncommingLine().GetDigit(tp) != GC_SUCCESS)
		{
			LOG4CPLUS_INFO(IVR_LOGGER,"GetDigits Failed");
			return -1;
		}
	}
	catch(...)
	{
		LOG4CPLUS_INFO(IVR_LOGGER,"fucking exception is in get digits");
	}
	//	Get digit busy
	m_fGetDigitsBusy = false;
	return 0;
}

void CCallProcessor::PlayFile(std::string& file)
{
	TPT tpt;
	
	LOG4CPLUS_INFO(IVR_LOGGER,"Play file request" << file); 

	if(IsVoiceBusy())
	{
		LOG4CPLUS_ERROR(IVR_LOGGER, "Voice Device is busy(Could not play)");
		return;
	}

	tpt << TPT::TP(DX_MAXDTMF,TF_MAXDTMF,1);

	if(IncommingLine().Play(file,tpt) != GC_SUCCESS)
	{
		LOG4CPLUS_INFO(IVR_LOGGER,"Play file Failed");
		return;
	}
	//	playfile is busy  now
	m_fPlayFileBusy = true;
}

bool CCallProcessor::IsVoiceBusy(void)
{
	return (m_fPlayFileBusy || m_fGetDigitsBusy);
}

void CCallProcessor::PlayFileLang(std::string file, int next_state)
{
	PlayFile(CConfiguration::Instance().GetLangVoxFile(file,m_CurrentLanguage),next_state);
}
