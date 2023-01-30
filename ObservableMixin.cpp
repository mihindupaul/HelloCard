#include "stdafx.h"
#include "log4cplus/logger.h"
#include ".\observablemixin.h"

CObservableMixin::CObservableMixin(void)
:m_pObserverLock(NULL)
{
}

CObservableMixin::~CObservableMixin(void)
{
}

int CObservableMixin::AddLineObserver(ILineObserver* pObserver)
{
	//	Add the Observer to my observer list
	assert(m_ObserverList.size()<=3);	// it is unlike to have more than 2 observer per line

	m_ObserverList.push_front(pObserver);

	LOG4CPLUS_INFO(LINE_LOGGER,"Observer Added n=" << m_ObserverList.size() << ".."  << ":" <<pObserver); 
	return 0;
}

int CObservableMixin::RemoveLineObserver(ILineObserver* pObserver)
{
	LOG4CPLUS_INFO(LINE_LOGGER,__FUNCTION__ << pObserver);

	//	Observer Self Removal
	if(m_pObserverLock == NULL || m_pObserverLock == pObserver)
	{
		LOG4CPLUS_INFO(LINE_LOGGER,"Observer Marked for Removing..." << pObserver );
		m_pObserverLock = pObserver;
	}
	else
	{
		//	This should Never been called
		LOG4CPLUS_FATAL(LINE_LOGGER,"NEVER: Recursive Observer removal Old:" << m_pObserverLock << "New:" << pObserver );
	}
	return 0;
}

void CObservableMixin::ClearMarkedObservers(void)
{
	if(m_pObserverLock != NULL)
	{
		//	observer has been modified in above iteration
		//	we can safely remove that 
		LOG4CPLUS_INFO(LINE_LOGGER,"OBSERVER REMOVAL DETECTD(Safe to Delete)" << m_pObserverLock <<"n=" <<m_ObserverList.size());
		
		//	Safely Removing the requested Observer
		m_ObserverList.remove_if(std::bind2nd(std::equal_to<ILineObserver*>(), m_pObserverLock));

		LOG4CPLUS_INFO(LINE_LOGGER,"Observer Stat = " << m_ObserverList.size() );
		
		if(m_ObserverList.size() > 2)
		{
			for(i=m_ObserverList.begin();i!=m_ObserverList.end();i++)
			{
				//	notify all observers about the event
				LOG4CPLUS_INFO(LINE_LOGGER,"Multi observers " << *i);
			}
		}

		m_pObserverLock = NULL;	//	reset this flag
	}
}
