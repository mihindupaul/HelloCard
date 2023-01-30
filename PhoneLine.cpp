/******************************************************************************************/
/*                                                                                        */
/*	Source File		: PhoneLine.cpp														  */
/*																						  */	
/*	Version			: 3.0																  */
/*                                                                                        */
/*	Target OS		: Windows 2000	Service Pack 3.0									  */
/*																						  */
/*	Description		: Dialogic Board Configuration and Application Prototypes  */
/*																						  */
/*	Last Revision	: <29/04/2003>  												      */
/*																						  */
/*	Release Stage	: BETA																  */
/*																						  */
/*	Author			: A.T.Lloyd	(c) 2002,  All rights reserved.							  */
/*					  Tritel Technologies (Pvt.) Ltd.									  */
/*  Revisions		:
/*		08/15/08	STL Headers updated		MP
/******************************************************************************************/
#include "stdafx.h"
#include <string>
#include <algorithm>
#include "ss7line.h"
#include "./phoneline.h"

//	Tritel's BL components. Best implementation is FACTORY Method
#include "./tritelivr.h"
#include "./tritelcallconnector.h"
#include ".\phoneline.h"

CLineManager::CLineManager()
: m_hThread(NULL)
, m_fGCStarted(false)
, m_vecConnector(0)
, m_vecIVR(0)
, m_vecLines(400)
{
	m_vecConnector.clear();
}

CLineManager::~CLineManager()
{
}

int CLineManager::Dialogic_Board_Config()
{
	char board_name[MAX_BOARD_NAME_LENGTH];  

	int index = 1;
	int brd_handle      = 0;
	int brd, ch, devcnt = 0;
	int numvoxbrds      = 0;
	int totvoxchs       = 0;
	int	subdev = 0;
	CLine*	tmp_line;

	//	experimental
	int vb = 1;
	int vch = 1;


	//Voice Channel Configuration
	//Look for the number of voice boards, if none found then  inform the user  
	if ( sr_getboardcnt("Voice", &numvoxbrds) == -1) 
	{
		LOG4CPLUS_FATAL(DEFAULT_LOGGER,"Couldn't get the number of voice boards");
		MessageBox(NULL,"Couldn't get the number of voice boards",NULL,MB_OK);		
		return -1;
	}

	if ( numvoxbrds == 0)
	{
		LOG4CPLUS_FATAL(DEFAULT_LOGGER,"No voice boards found");
		MessageBox(NULL,"No voice boards found",NULL,MB_OK);
		return -1;
	}

	for (brd = 1; brd <= numvoxbrds; brd++) 
	{
		sprintf(board_name, "dxxxB%d",brd);

		//Open the board, if board fails to open inform the user 
		if ((brd_handle = dx_open(board_name,0)) ==-1)
		{
			MessageBox(NULL,"Board Failed to Open",NULL,MB_OK);
			continue;	//	with next board;
		}
		
		subdev = ATDV_SUBDEVS(brd_handle);
		
		if(subdev == AT_FAILURE)
			continue;

		totvoxchs += subdev;

		if(vch != 1)
		{
			vb++;	// new board start with new virtual board
			vch = 1; // new channel group
		}

		//Begin voice channel for loop Open each channel on the board individually
		for (ch = 1; ch <= subdev; ch++) 
		{
			//	Is this comparition needed??
			if(index >= ss7is && index <= ss7oe)
			{
				//	Create the line
				tmp_line = new CSS7Line(this);
				
				if(tmp_line->Open(index,((index-1)/30)+1,((index -1)% 30) + 1,vb,vch) == 0)
				{
					RegisterLine(tmp_line);

					//	depending on in/out mode we choose IVR/Connector embedding
					if(index >= ss7os)
					{
						AddToConnectorPool(tmp_line);
					}
					else
					{
						AddToIVRPool(tmp_line);
					}
				}
				//	Next voice device
				if(++vch > 4)
				{				
					vb++;
					vch =1;
				}
			}
			index++;
		}
		//Close the Open Board
		dx_close(brd_handle);
	}

	return 0;
}

int CLineManager::Open_Device()
{
	//	configured channel settings from registry
	ReadTimeSlotConfiguration();

	//	configure and open lines.
	if(Dialogic_Board_Config() == 0)
	{
		//	Sort The Out line pool
		//std::sort(m_vecConnector.begin(),m_vecConnector.end());
	}
	return 0;
}

//	Do the message dispatching
int CLineManager::Start()
{
	DWORD ThreadId;

	if(!m_fGCStarted || Open_Device() != 0)
	{
		LOG4CPLUS_FATAL(DEFAULT_LOGGER,"Could not open device");
		//return -1; // uncomment this to stop the process on open failure
	}

	m_hThread = ::CreateThread(NULL,100,PsudoThreadProc,this,0,&ThreadId);

	if(m_hThread == NULL)
	{
		LOG4CPLUS_ERROR(DEFAULT_LOGGER,"Canot create Thread" << ::GetLastError());
		return -1;
	}

	LOG4CPLUS_INFO(DEFAULT_LOGGER, "Main thread created" << ThreadId);

	return 0;
}

int CLineManager::Stop()
{
	//	Send Close command to event thread
	if( AddUserEvent(USER_CMD_STOP) == 0)
	{
		//	Wait 30s for process thread to complete
		if(::WaitForSingleObject(m_hThread,30000) == WAIT_OBJECT_0)
		{
			::CloseHandle(m_hThread);
			m_hThread = NULL;
		}
		else
		{
			LOG4CPLUS_FATAL(DEFAULT_LOGGER,"Thread Stop Failed" << ::GetLastError());
		}
	}
	else
	{
		LOG4CPLUS_FATAL(DEFAULT_LOGGER,"Stop Command failed.");
	}

	//	Whether its gracefull or not we must close the device.
	Close_Device();

	return 0;
}

int CLineManager::Process_Event()
{
	int	ret = -1;
	METAEVENT metaevent;
	CLine* pLine = NULL;
	long id;
	
	try
	{
		if(gc_GetMetaEvent(&metaevent) == GC_SUCCESS)
		{
			if( metaevent.flags & GCME_GC_EVENT )
			{
				//	only global call events contains -UserAttribute-
				reinterpret_cast<CLine*>(metaevent.usrattr)->ProcessGlobalCallEvent(&metaevent);
				ret = 0;	//	Set this from above call state
			}
			else if( metaevent.evttype == SRL_USER_EVENT)
			{
				//	user defined event, catch the event data value
				if(metaevent.evtlen >= sizeof(int))
				{
					ret = *reinterpret_cast<int*>(metaevent.evtdatap);

					LOG4CPLUS_INFO(DEFAULT_LOGGER,"User Event " << ret );
					if(ret > 30)
					{
						LOG4CPLUS_INFO(DEFAULT_LOGGER,"Resetting line dev " << ret );
						m_vecLines[ret]->Reset();
					}
				}
			}
			else if(metaevent.evttype == 556)
			{
				if( ( id = sr_getevtdev(0))>0 )
				{
					if(m_mapLine[id] != NULL)
						m_mapLine[id]->ProcessEvent(&metaevent);
				}
				ret = 0;
			}
			else if((metaevent.evttype & DT_DTI) == DT_DTI)
			{
				LOG4CPLUS_INFO(DEFAULT_LOGGER,"DT_DTI Event");
			}
			else
			{
				//	user Attribute is not attached with this Event. (non GC_ Event) Find the Line object belong the event
				// Get the context from the user context device parameter
				//if( sr_getparm( metaevent.evtdev, SR_USERCONTEXT, (void*) pLine ) == -1 )
				//{ 
				//	//return ErrMsg(metaEvent.evtdev, "sr_getparm");
				//}
				//pLine->ProcessEvent(&metaevent);
				m_mapLine[metaevent.evtdev]->ProcessEvent(&metaevent);
				ret = 0;
			}
		}
		else
		{
			LOG4CPLUS_FATAL(DEFAULT_LOGGER,"gc_GetMetaEvent() Failed");
		}
	}
	catch(std::invalid_argument& e)
	{
		LOG4CPLUS_FATAL(DEFAULT_LOGGER,"Connector In line Exception:" << __FUNCTION__ << e.what());
	}
	catch(...)
	{
		LOG4CPLUS_FATAL(DEFAULT_LOGGER,"Unknown Exception:" << __FUNCTION__ << metaevent.usrattr);
	}
	return ret;
}

void CLineManager::ResetAllChannels()
{
	LOG4CPLUS_INFO(DEFAULT_LOGGER,"Reset All Called");
	AddUserEvent(USER_CMD_RESET_ALL);
}

int CLineManager::RemoveBlock()
{
	return 1;
}

int CLineManager::ReadTimeSlotConfiguration()
{
	ATL::CRegKey rk;
	if(!rk.Open(HKEY_LOCAL_MACHINE,"Software\\Tritel\\Hello\\Board\\SS7"))
	{
		rk.QueryDWORDValue("ISTART",ss7is);
		rk.QueryDWORDValue("IEND",ss7ie);
		rk.QueryDWORDValue("OSTART",ss7os);
		rk.QueryDWORDValue("OEND",ss7oe);
	}

	return 1;
}

///	Pick a available free call connector
int CLineManager::PickOutLineConnector(CCallConnector** ppOutLine,int Start,int Stop)
{
	std::vector<CCallConnector*>::iterator i ;
	
	LOG4CPLUS_INFO(DEFAULT_LOGGER,"Connector Pick Request: Range From:" << Start << " To:" << Stop);
	
	if(Start < ss7os)
	{
		LOG4CPLUS_ERROR(DEFAULT_LOGGER,"Invalid Range Request");
		return -1;
	}

	//	Find a Connector which is NOT IsBusy()
	i = std::find_if(m_vecConnector.begin() + (Start - ss7os),	//	Start Possition
		m_vecConnector.end(),									//	End Possition
		std::not1(std::mem_fun(&CCallConnector::IsBusy)));
	
	if(i != m_vecConnector.end())
	{
		//	TODO: Check for Going over seeking
		*ppOutLine = *i;

		LOG4CPLUS_INFO(DEFAULT_LOGGER,"Connector Picked: " << (*ppOutLine)->ToString() << ",idx=" << (i - m_vecConnector.begin()));
		
		return 0;
	}
	//	no free connectors found.
	LOG4CPLUS_INFO(DEFAULT_LOGGER,"No more Connectors.(cong)")
	return -1;
}

int CLineManager::AddToConnectorPool(CLine* pLine)
{
	//	Create CallConnector which handles this outgoing line
	CCallConnector* new_connector = new Tritel::CTritelCallConnector(*pLine);

	//	Add this line to outgoing vector.
	if(new_connector != NULL)
	{
		//	add this to my connector pool
		m_vecConnector.push_back(new_connector);
		return 0;
	}

	return -1;
}

int CLineManager::AddToIVRPool(CLine* pLine)
{
	CAnswerMachine* new_ivr = NULL;

	//	Create a IVR to handle in call functions
	try
	{
		new_ivr = new Tritel::CTritelIVR(*pLine);

		if(new_ivr != NULL)
		{	
			m_vecIVR.push_back(new_ivr);
			return 0;
		}
	}
	catch(...)
	{
		LOG4CPLUS_DEBUG(DEFAULT_LOGGER,"Exception:" << __FUNCTION__);
	}
	return -1;
}

DWORD WINAPI CLineManager::PsudoThreadProc(LPVOID lpParameter)
{
	return reinterpret_cast<CLineManager*>(lpParameter)->ThreadProc();
}

DWORD CLineManager::ThreadProc(void)
{
	long timeleft;
	int	ret;

	do
	{
		// Wait for an event 
		do
		{
			timeleft = sr_waitevt(2000);

		}while(timeleft == -1);

		m_csEventLock.Lock(); 

		ret = Process_Event();
		
		m_csEventLock.Unlock();

		if(ret == -1)
		{
			LOG4CPLUS_FATAL(DEFAULT_LOGGER,"That Event Made a exception");
		}
		//	process other return codes hear.

	}while( ret != USER_CMD_STOP );

	LOG4CPLUS_INFO(DEFAULT_LOGGER,"Event Loop Finished");

	return 0x2;	
}

void CLineManager::Close_Device(void)
{
	std::vector<CCallConnector*>::iterator j;	
	std::vector<CAnswerMachine*>::iterator i;

	//	Close all incomming lines.
	for(i = m_vecIVR.begin();i!=m_vecIVR.end();i++)
	{
		(*i)->Close();	//	Close the IVR (this internally delete the attached line);
		delete (*i);	// Delete IVR/ This S
	}
	m_vecIVR.clear();

	for(j=m_vecConnector.begin();j != m_vecConnector.end();j++)
	{
		//(*j)->Break();
		delete (*j);	//	Delete the connector
	}
	m_vecConnector.clear();
	
	CloseAllLines();
}

int CLineManager::RegisterLine(CLine* pLine)
{
	//	Add the line for tracking
	LOG4CPLUS_INFO(DEFAULT_LOGGER,"registration " << pLine->GetVoiceH() << "=" << pLine);
	m_mapLine[pLine->GetVoiceH()] = pLine;
	m_vecLines[pLine->Channel()] = pLine;
	return 0;
}

int CLineManager::AddUserEvent(int data)
{
	return sr_putevt(SRL_DEVICE, SRL_USER_EVENT,sizeof(int),&data,0);
}

bool CLineManager::IsRunning(void) const
{
	return (m_hThread != NULL);
}

void CLineManager::CloseAllLines(void)
{
	std::map<long,CLine*>::iterator k;
			//	Close and remove all lines
	for(k = m_mapLine.begin();k != m_mapLine.end(); k++)
	{
		k->second->Close();
		delete k->second;
	}

	m_mapLine.clear();
}

int CLineManager::Initialize(void)
{
	GC_START_STRUCT gclib_start;
	CCLIB_START_STRUCT  cclib_start[]={   
        {"GC_SS7_LIB",NULL},   
    };  

    gclib_start.num_cclibs = 1;  
    gclib_start.cclib_list = cclib_start; 

	//Start GC Libraries 
	LOG4CPLUS_INFO(DEFAULT_LOGGER,"Starting GC");

	if ( gc_Start(&gclib_start) == GC_SUCCESS )
	{
		m_fGCStarted = true;
	}
	else
	{
		LOG4CPLUS_FATAL(DEFAULT_LOGGER,"gc_Start() Failed");
	}

	return 0;
}

int CLineManager::Finalize(void)
{
	//	Stop the GC libraries
	LOG4CPLUS_INFO(DEFAULT_LOGGER,"Stopping GC");

	if (m_fGCStarted && gc_Stop() != GC_SUCCESS )
	{
		LOG4CPLUS_FATAL(DEFAULT_LOGGER,"gc_Stop() Failed");
	}
	return 0;
}

void CLineManager::TimerTrigger(void* pVoid)
{
	//	locking is needed to synchronize timer triggers and 
	m_csEventLock.Lock();
	
	try
	{
		LOG4CPLUS_FATAL(DEFAULT_LOGGER,"Timer Trigger for:" << (long)pVoid);
		reinterpret_cast<CLine*>(pVoid)->AddUserEvent(50);
	}
	catch(...)
	{
		LOG4CPLUS_FATAL(DEFAULT_LOGGER,"Exception in time triggering");
	}
	
	m_csEventLock.Unlock();
}

int CLineManager::OnLineOpen(CLine& line)
{
	return 0;
}

bool CLineManager::ShouldOpen(int index)
{
	return (index >= ss7is && index <= ss7oe);
}
