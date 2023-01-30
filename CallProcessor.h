#pragma once
#include "log4cplus/logger.h"
#include "line.h"
#include "ilineobserver.h"
#include "configuration.h"


#define CFS_BLOCKED				1
#define CFS_RESERVED			3
#define CFS_LAST_PLAY			0xff
//.... reserved for future internal states
#define CFS_USER		0x2000


//	This is the default logger for all IVR functions
#define IVR_LOGGER	log4cplus::Logger::getInstance("default")

class CCallProcessor: public ILineObserver
{
public:
	CCallProcessor(void);
	virtual ~CCallProcessor(void);


	virtual void Reset(void);
	virtual CLine& IncommingLine() = 0;
	bool HasCaller(void);
	// Return the control to calling (source) CCallprocessor
	virtual void ReturnControl(int cause = GC_NORMAL_CLEARING);
	virtual void OnPlayFileTerminate(int reason);
	int GetState(void);

	void Suspend(void);
	void Resume(void);
	bool IsSuspended(void);

protected:
	virtual int Exit(int cause);
	virtual int Exit(std::string last_msg,int cause = GC_NORMAL_CLEARING);
	// Transfer the control of one call processor to another
	virtual int Transfer(CCallProcessor& source);
	virtual int OnCallDisconnected(CLine& line, METAEVENT& evt);
	void PlayFile(std::string& file, int next_state);
	void PlayFile(std::string& file);
	virtual void OnStateChange(int state);
	void ReturnControl(std::string last_msg);
	CCallProcessor& SourceProcessor(void);
	int OnPlayCompleted(CLine& line, int reason);
	void SetState(int state);
	virtual void OnGetDigitsTerminate(int reason,std::string digits);
	// conviniant function for simple digit value get
	int GetDigits(int max_len, int terminate_dig = DM_P, int timeout = 300);	

	CConfiguration::Language m_CurrentLanguage;

private:
	void OnGetDigitComplete(CLine& line, int reason,const std::string& digits);

	int m_nCallFlowState;
	CCallProcessor* m_pSourceProcessor;
	bool m_fSuspend;
	int m_nLastPlayDropCause;
	bool m_fGetDigitsBusy;
private:
	bool m_fPlayFileBusy ;
protected:
	bool IsVoiceBusy(void);
	bool m_fDropAfterVoiceCompletion;
	void PlayFileLang(std::string file, int next_state);
};
