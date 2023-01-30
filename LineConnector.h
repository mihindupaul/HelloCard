//
//	Mediator(GOF) base class for connecting incomming and outgoing call.
//
#pragma once
#include "./line.h"
#include "./linehandler.h"
#include "answermachine.h"
#include "callprocessor.h"

//	basic line connector satus
#define CON_OPEN		CFS_USER + 1
#define	CON_DIALING		CFS_USER + 2
#define CON_ALERTING	CFS_USER + 3
#define CON_CONNECTED	CFS_USER + 4
#define CON_CONFAILED	CFS_USER + 5
#define CON_FINISHED	CFS_USER + 6

#define CONNECTOR_LOGGER	log4cplus::Logger::getInstance("default")

class CCallConnector: public CLineHandler, public CCallProcessor
{
public:

	CCallConnector(CLine& line);
	virtual ~CCallConnector();

	virtual int Transfer(CCallProcessor& amc,std::string& number,int duration = 360);
	bool IsBusy();
	virtual std::string ToString();

protected:

	CLine& IncommingLine(void);
	virtual void OnDropCallComplete(CLine& line, METAEVENT& evt);
	virtual void OnReleaseCallComplete(CLine& line, METAEVENT& evt);
	virtual int HandleMakeCallFailure(int reason, int cc_reason,int& cause,std::string& msg);
	void OnDialing(CLine& line, METAEVENT& evt);
	void SetTimer(int seconds);
	void CancelTimer(void);

private:
	bool		m_fTest;
	HANDLE		m_hTimer;
	int m_nAvailableTime;
	//	Timer Support

	int out_CallConnected(CLine* l);
	int OnCallDisconnected(CLine& line,METAEVENT& evt);
	int out_CallStatus(int reason);
	int OnAnswerCallComplete(CLine& line);
	int out_CallAlerting(CLine& line, int code);
	void OnUserEvent(CLine& line, int code);
	void OnUnblocked(CLine& line, METAEVENT& evt);
	void OnResetComplete(CLine& line, METAEVENT& evt);
};
