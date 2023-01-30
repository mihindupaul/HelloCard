/******************************************************************************************/
/*                                                                                        */
/*	Source File		: PhoneLine.h														  */
/*																						  */	
/*	Version			: 3.0																  */
/*                                                                                        */
/*	Target OS		: Windows 2000	Service Pack 3.0									  */
/*																						  */
/*	Description		: Dialogic Board Configuration and Application Implementations		  */
/*																						  */
/*	Last Revision	: <2804/2003>  												      */
/*																						  */
/*	Release Stage	: BETA																  */
/*																						  */
/*	Author			: A.T.Lloyd	(c) 2002,  All rights reserved.							  */
/*					  Tritel Technologies (Pvt.) Ltd.									  */
/******************************************************************************************/
#pragma once

/*********************  Dialogic Header Files  ***********************/
#include <srllib.h>            // SRL header file             
#include <dxxxlib.h>           // Voice library header file   
#include <dtilib.h>            // DTI library header file     
#include <gclib.h>             // Global Call API header file 
#include <gcerr.h>             // Global Call API errors file 
#include <signal.h>
#include <fcntl.h>
#include "sctools.h"		   //For SCBUS Routing
#include "HelloCard.h"
#include "criticalsection.h"
#include <atldbcli.h>

#include <libgcs7.h>
#include <gcisdn.h> 

/*********************  System Header Files  ************************/
#include <vector>
#include <map>


/*********************  Constants  **********************************/

#define  MAXCHAN     360 // Maximum Channel 12E1=12*30

/*********************  Call state definition  ************************/

//#define  ST_TALK					0x103  // Call Established and Progrsseing 
//#define  ST_DISCONNECTED			0x110	// Drop call    
// v 4.0 addtions
//#define  ST_ON_IVR					ST_TALK

//	Thease are not logically mapped with call states. more relate with callflow states.
//#define  ST_PLAYEND					0x115	//	removed for IVR decopling
//#define  ST_TIMEOUT					0x111
#define  ST_BLOCK					0x118

//#define  ST_TIMEOUT2				0x112 
//#define  ST_PROTOCOL				0x113 
//#define  ST_UNKNOWN					0x114
//#define  ST_RECHARGE				0x116
//#define  ST_TOTALBUSINESS			0x117
//#define  ST_CALLHISTORY				0x119
//#define  ST_BNAMR					0x120
//#define  ST_CONTINUE				0x121
//#define  ST_AUTHENTICATION			0x122
//#define  ST_ERROR					0x123


/*********************  dtmf timing for TN110 **********************/
/*
1.	Time preceding sending balance	:  balance_time.
2.	Time preceding sending time     	:  silence_time
3.	On time duration of DTMF	s       	:  mark_time
4.	Off time duration of DTMFs  	: space_time
*/
#define balance_time		60
#define duration_time		60
#define mark_time			8
#define space_time			8


// v.4
#define MARK_TIME			mark_time
#define SPACE_TIME			space_time

//	v.4 SRL	User defined commands
#define DEFAULT_LOGGER	log4cplus::Logger::getInstance("default")

//	Event type for user events. make sure this is not used in GC libs.
#define SRL_USER_EVENT		555
#define SRL_USER_EVENT_X	556

//	User defined commands to send SRL event Queue
#define USER_CMD_RESET_ALL	0x1
#define USER_CMD_STOP		0x2
#define	USER_CMD_RESERVED	0x3

//	Forward declarations
class CLine;
class CAnswerMachine;
class CCallConnector;

class CLineManager  
{
public:
	CLineManager();
	virtual ~CLineManager();

	int Start();
	int Stop();
	void ResetAllChannels();
	int RemoveBlock();
	int Disconnect(int nIchannel);

	int PickOutLineConnector(CCallConnector** ppOutLine,int Start,int Stop);
	
	bool IsRunning() const;
	int Initialize();
	int Finalize();

public:
	int AddUserEvent(int data);

private:
	int ReadTimeSlotConfiguration();
	int Open_Device();
	int Dialogic_Board_Config();
	int Process_Event();
	static DWORD WINAPI PsudoThreadProc( LPVOID lpParameter);
	DWORD ThreadProc();
	void Close_Device();
	int RegisterLine(CLine* pLine);
	void CloseAllLines();

	//		TO BE TRANSFERED TO SUPER CLASS					//////
	///////////////////////////////////////////////////////////////
	int AddToIVRPool(CLine* pLine);
	int AddToConnectorPool(CLine* pLine);

private:
	//SS7 Configuration
	DWORD ss7is;
	DWORD ss7ie;
	DWORD ss7os;
	DWORD ss7oe;

	std::vector<CAnswerMachine*>	m_vecIVR;
	std::vector<CCallConnector*>	m_vecConnector;
	///////////////////////////////////////////////////////////////

	std::map<LINEDEV,CLine*>		m_mapLine;
	std::vector<CLine*>				m_vecLines;
	CCriticalSection				m_csEventLock;

	HANDLE	m_hThread;
	bool m_fGCStarted;
public:
	void TimerTrigger(void* pVoid);
protected:
	virtual int OnLineOpen(CLine& line);
	virtual bool ShouldOpen(int index);
};
