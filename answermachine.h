#pragma once
#include "./phoneline.h"
#include "log4cplus/logger.h"
#include "./linehandler.h"
#include "./callprocessor.h"

//
//	Call flow definitions
//
class CAnswerMachine:  public CLineHandler, public CCallProcessor
{
public:
	CAnswerMachine(CLine& line);
	virtual ~CAnswerMachine();

	virtual int Close();
	virtual int Begin(CLine& line) = 0;
	CLine& IncommingLine(void);

protected:

	virtual void OnReleaseCallComplete(CLine& line, METAEVENT& evt);
	virtual int OnCallDisconnected(CLine& line, METAEVENT& evt);

private:
	virtual int OnAcceptCallComplete();
	void OnDropCallComplete(CLine& line, METAEVENT& evt);
	void OnCallOffered(CLine& line, METAEVENT& evt);
protected:
	void OnUnblocked(CLine& line, METAEVENT& evt);
};



