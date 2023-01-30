#include "stdafx.h"
#include ".\tritellinemanager.h"
#include ".\tritelcallconnector.h"
#include ".\tritelivr.h"

Tritel::CTritelLineManager::CTritelLineManager(void)
: m_ss7_in_start(0)
, m_ss7_in_end(0)
, m_ss7_out_start(0)
, m_ss7_out_end(0)
{
}

Tritel::CTritelLineManager::~CTritelLineManager(void)
{
}

int Tritel::CTritelLineManager::OnLineOpen(CLine& line)
{
	//	depending on in/out mode we choose IVR/Connector embedding
	if(line.Channel() >= m_ss7_out_start)
	{
		AddToConnectorPool(line);
	}
	else
	{
		AddToIVRPool(line);
	}
	return 0;
}

void Tritel::CTritelLineManager::LoadSettings(void)
{
	ATL::CRegKey rk;
	if(!rk.Open(HKEY_LOCAL_MACHINE,"Software\\Tritel\\Hello\\Board\\SS7"))
	{
		rk.QueryDWORDValue("ISTART",m_ss7_in_start);
		rk.QueryDWORDValue("IEND",m_ss7_in_end);
		rk.QueryDWORDValue("OSTART",m_ss7_out_start);
		rk.QueryDWORDValue("OEND",m_ss7_out_end);
	}
}

int Tritel::CTritelLineManager::AddToConnectorPool(CLine& line)
{
		//	Create CallConnector which handles this outgoing line
	CCallConnector* new_connector = new CTritelCallConnector(line);

	//	Add this line to outgoing vector.
	if(new_connector != NULL)
	{
		//	add this to my connector pool
		m_vecConnector2.push_back(new_connector);
		return 0;
	}

	return -1;
}

int Tritel::CTritelLineManager::AddToIVRPool(CLine& line)
{
	CAnswerMachine* new_ivr = NULL;

	//	Create a IVR to handle in call functions
	try
	{
		new_ivr = new CTritelIVR(line);

		if(new_ivr != NULL)
		{	
			m_vecIVR2.push_back(new_ivr);
			return 0;
		}
	}
	catch(...)
	{
		LOG4CPLUS_DEBUG(DEFAULT_LOGGER,"Exception:" << __FUNCTION__);
	}
	return -1;
}

int Tritel::CTritelLineManager::PickLineConnector(CCallConnector** ppOutLine, int Start, int End)
{
	return 0;
}
