#pragma once
#include "lineconnector.h"
#include "./databasesupport2.h"

namespace Tritel
{

class CTritelCallConnector :
	public CCallConnector
{
public:
	CTritelCallConnector(CLine& line);
	virtual ~CTritelCallConnector(void);

	int Transfer(CCallProcessor& amc,CALLRATE& rate);

private:
	int UpdateComCharges();
	int CommLog(char* msg);
	float fUsedValue;
	CALLRATE m_call_rate;
	time_t start_time,finish_time;
	int nCallDuration;
protected:
	void OnStateChange(int state);
};

};