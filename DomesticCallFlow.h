#pragma once
#include "callflow.h"

class CDomesticCallFlow :
	public CCallFlow
{
public:
	CDomesticCallFlow(CLineManager& manager);
	virtual ~CDomesticCallFlow(void);
	int ProcessCallFlow(CLine& line, int code, int data);
	int GetId(void);
private:
	int PlayMinute(CLine& line, int flag);
};
