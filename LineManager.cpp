#include "stdafx.h"
#include ".\linemanager.h"
#include "cellcallflow.h"
#include "databasesupport2.h"

CLineManager::CLineManager(void)
{
	m_callhandler[0] = NULL;	//	Tritel uses 1 based index
	m_callhandler[1] = new CCellCallFlow(*this);
}

CLineManager::~CLineManager(void)
{
	CDatabaseSupport2::Cleanup();
}

int CLineManager::GetFreeOutgoingLine(CLine** ppLine)
{
	//	TODO: get the next available outgoing line
	//	Return 0 on success
	//srand( (unsigned)time( NULL ) );
	//do
	//{
	//	i = rand()  % ((OEnd + 1) - OStart) + OStart;																		
	//	if((i>=OStart)&&(i<=OEnd))
	//	{
	//		count++;
	//		if((port[i].state == ST_NULL)&&(port[i].direction== OUTGOING))
	//		{
	//			plineo=&port[i];
	//			plineo->state=ST_DIALING;
	//			pline->state=ST_DIALING;

	//			Congestion=false;
	//			break;
	//		}
	//	}

	//	if(count>2*maxchan)
	//	{
	//		Congestion=true;
	//		break;
	//	}

	//}while(true);
	return 0;
}

CCallFlow* CLineManager::GetCallFlowHandler(int business_model)
{
	//	Remove this for proper functionalyty
	business_model = 1;
	return m_callhandler[business_model];
}
