#pragma once
#include <list>
#include "ilineobserver.h"
#define LINE_LOGGER	log4cplus::Logger::getInstance("default")

class CObservableMixin
{
public:
	CObservableMixin(void);
	virtual ~CObservableMixin(void);
	int AddLineObserver(ILineObserver* pObserver);
	int RemoveLineObserver(ILineObserver* pObserver);
protected:
	void ClearMarkedObservers(void);

	std::list<ILineObserver*>	m_ObserverList;
	std::list<ILineObserver*>::iterator i;

private:
	ILineObserver* m_pObserverLock;
};
