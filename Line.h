#pragma once
#include <list>
#include <string>
#include "tritel.h"
#include "./phoneline.h"
#include "./ilineobserver.h"
#include "./tone.h"
#include "log4cplus/logger.h"
#include "terminationparam.h"
#include "observablemixin.h"

#define LINE_LOGGER	log4cplus::Logger::getInstance("default")

class CLineManager;

class CLine : public CObservableMixin
{

public:

	//	For Event pump.
	friend CLineManager;//::Process_Event();

	CLine(CLineManager* manager);
	virtual ~CLine();

	int Reset();
	int DropCall(int cause = GC_NORMAL_CLEARING);
	int AnswerCall();
	void ClearDigitBuffer();
	void SetTrace(bool value = true);
	CLineManager& GetManager() const;
	
	virtual int MakeCall(std::string number,std::string caller_number) = 0;
	virtual int Open(int index,int board,int time_slot,int vb,int vch) = 0;
	virtual int Close();	
	LINEDEV& GetVoiceH() ;
	LINEDEV& GetNetworkH(void);
	virtual std::string GetANI() const = 0;
	virtual std::string GetDNIS() const = 0;
	virtual std::string ToString();

	CLineManager& GetLineManager();
	int WaitCall();
	int GetState(void);
	int EnableCPA(void);
	int Channel(void);
	int Play(const CTone& tone, const TPT& tpt, int mode=EV_ASYNC);
	int Play(const std::string& path, const TPT& tpt);
	int SetChannelState(int state);
	virtual int GetDigit(const TPT& tpt, int mode = EV_ASYNC);
	virtual int AcceptCall(int rings);
	virtual int SetVoice(bool enable);
	virtual int ReleaseCall(void);

	void StopPlay(void);
	bool IsConnected(void);

private:
	virtual int ProcessEvent(METAEVENT* pMetaEvent);
	virtual int ProcessGlobalCallEvent(METAEVENT* pMetaEvent);

	DV_DIGIT dtbuf;     // Buffer for DTMFs

protected:
	virtual void OnDisconnected(METAEVENT* pMetaEvent);
	virtual void OnCallOffered(METAEVENT& evt);
	virtual void OnAcceptCallComplete(CRN crn);
	virtual void OnDropCallComplete(METAEVENT& evt);
	virtual void OnHoldCall(METAEVENT& evt);
	virtual void OnResetComplete(METAEVENT& evt);
	virtual int OnAnswerCallComplete(METAEVENT& evt);
	virtual int OnPlayComplete(long reason);
	virtual void OnGetDigitComplete(METAEVENT& evt);
	virtual int OnTaskFail(METAEVENT* pMetaEvent);
	virtual int OnMakeCallStatus(CRN crn,int err);
	virtual int OnMakeCallAlerting(METAEVENT& evt);
	virtual int OnMakeCallConnected(CRN crn);
	virtual int OnDialing(METAEVENT& evt);
	virtual int OnOpened(METAEVENT& metaevent);
	virtual int OnReleaseCallComplete(METAEVENT& evt);
	virtual int OnPlayToneComplete(METAEVENT& evt);
	std::string GetLastErrorDetails();
	virtual int OnSetChanState(METAEVENT& evt);
	virtual void OnUnblocked(METAEVENT& evt);

	//	MakeCall Structures
	GC_MAKECALL_BLK		makecallblk;
	GCLIB_MAKECALL_BLK	gclib_makecall;

	//	line manager
	CLineManager*	m_pLineManager;
	std::string		m_sDeviceName;
	int state;
	DX_IOTT iott[1];
	CRN crn;				// GCAPI call Referecne Number 
    LINEDEV	voiceh;			// Voice channel handle  
	LINEDEV timesloth;      // Timeslot handle 
	LINEDEV	ldev;			// Network line device handle  
	int channel;
//	TODO: Get rid of this
public:
	bool m_fDropInProgress;
	int AddUserEvent(int code);
protected:
	void OnUserEvent(METAEVENT& evt);
};
