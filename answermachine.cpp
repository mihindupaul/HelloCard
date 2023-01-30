#include "stdafx.h"
#include <cassert>
#include ".\answermachine.h"

CAnswerMachine::CAnswerMachine(CLine& line)
: CLineHandler(line)
{
	//	Register this as a observer to provided incomming line
	line.AddLineObserver(this);
}

CAnswerMachine::~CAnswerMachine(void)
{
}

int CAnswerMachine::OnAcceptCallComplete()
{
	//	This is the Starting point of IVR
	LOG4CPLUS_DEBUG(IVR_LOGGER,"Begin() Called upon Accept call complete :");

	if(GetState() == CFS_BLOCKED)
	{
		if(Begin(GetLine()))
		{
			LOG4CPLUS_INFO(IVR_LOGGER,"IVR Begin() failed. Drop the line");
			GetLine().DropCall();
		}
	}
	return	0;	//	Call the subclass version of Call Control
}

int CAnswerMachine::Close(void)
{
	return 0;
}

//	Answer machine always (re)wait for callls
void CAnswerMachine::OnReleaseCallComplete(CLine& line, METAEVENT& evt)
{
	if(!IsMyLine(line))
		return;

	LOG4CPLUS_INFO(IVR_LOGGER,"Line released" << line.Channel() );
	
	//Reset information row on UI	
	ListviewResetRow(line.Channel()-1);	
	
	Reset();

	if(line.WaitCall() == 0)
	{
		ListviewSetColumn(line.Channel()-1,6,"Re-Wait Call");
	}
	else
	{
		ListviewSetColumn(line.Channel()-1,6,"BLOCKED");
	}
}


CLine& CAnswerMachine::IncommingLine(void)
{
	return GetLine();
}

int CAnswerMachine::OnCallDisconnected(CLine& line, METAEVENT& evt)
{
	if(IsSuspended())
		return -1;

	if(!line.m_fDropInProgress)
	{
		LOG4CPLUS_INFO(IVR_LOGGER,"DISCONNECT IN IVR");
		//	Cancel current play
		//	Cancel Current GetDigits
		if(IsVoiceBusy())
		{
			LOG4CPLUS_INFO(IVR_LOGGER,"voice device Was Busy when the disconnection (Wait till completion)");
			//	so we CANOT just dorp the line hear. instd stop can be called with an indication
			//	that the line should be dropped after play/getdigit complete

			//	Can we synchronouly stop the Voice device???
			GetLine().StopPlay();
			
			//	this could generate either play/getdigit complete (Only One of them is possible)
			m_fDropAfterVoiceCompletion = true;

			line.DropCall();
		}
		else
		{
			/*
				no voice activity on the line when disconnect happened. so we can just drop hear
			*/
			line.DropCall();
		}
	}
	LOG4CPLUS_INFO(IVR_LOGGER,"IVR Line disconnected." << line.Channel() );
	
	return 0;
}

void CAnswerMachine::OnDropCallComplete(CLine& line, METAEVENT& evt)
{
	ListviewSetColumn(line.Channel()-1,6,"Releasing...");
	
	//	good place to remove TDM bus connectivity (if exist)
	line.SetVoice(false);

	line.ReleaseCall();
}

void CAnswerMachine::OnCallOffered(CLine& line, METAEVENT& evt)
{
	// Answer the incoming call (sethook OFF)
	LOG4CPLUS_INFO(LINE_LOGGER, "Call Offered. Accepting..." << line.Channel());

	if(line.AcceptCall(0) == GC_SUCCESS)
	{
		ListviewSetColumn(line.Channel()-1,1,line.GetANI());
		ListviewSetColumn(line.Channel()-1,3,line.GetDNIS());
		ListviewSetColumn(line.Channel()-1,6,"Accepting...");
	}
	else
	{
		LOG4CPLUS_ERROR(IVR_LOGGER,"AcceptCall() Failed.");
	}
}

void CAnswerMachine::OnUnblocked(CLine& line, METAEVENT& evt)
{
	if(line.WaitCall() != 0)
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,"In Line unblocked. wait failed" << line.Channel());
	}
		//	show it on user interface
	ListviewResetRow(line.Channel()-1);
	ListviewSetColumn(line.Channel()-1,6,"Wait For Call");
}
