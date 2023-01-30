/*
////////////////////////////////////////////////////////////////////////////////////////
#	Author	:	mihindu paul (mihindupaul@gmail.com)
#	
#	Desc	:	Obeserver(gof) interface for CLine abstract.
#
#	Date	:	09-01-2008
#
#	Version	:	1.0
////////////////////////////////////////////////////////////////////////////////////////#	
*/
#pragma once
#include "gclib.h"

class CLine;

class ILineObserver
{
public:
	virtual int OnAnswerCallComplete(CLine& line) {return 0;};
	//	must implement
	virtual int OnPlayCompleted(CLine& line,int reason) = 0;
	virtual int OnAcceptCallComplete() {return 0;};
	//	app interested outgoing line notifications
	virtual int out_CallConnected(CLine* l) {return 0;};
	virtual void OnDropCallComplete(CLine& line,METAEVENT& evt){return;};
	virtual void OnReleaseCallComplete(CLine& line,METAEVENT& evt){return;};
	virtual int OnCallDisconnected(CLine& line,METAEVENT& evt) =0;
	virtual int out_CallAlerting(CLine& line,int code) {return 0;};
	virtual int out_CallStatus(int reason){return 0;};
	virtual void OnResetComplete(CLine& line, METAEVENT& evt){};
	virtual void OnCallOffered(CLine& line, METAEVENT& evt)	{};
	virtual void OnPlayToneComplete(CLine& line, int reason){};
	virtual void OnGetDigitComplete(CLine& line, int reason, const std::string& dtbuff){};
	virtual void OnSetChanState(CLine& line, METAEVENT& evt){};
	virtual void OnDialing(CLine& line, METAEVENT& evt)	{};
	virtual void OnUnblocked(CLine& line, METAEVENT& evt){};
	virtual void OnUserEvent(CLine& line, int code){}
};
