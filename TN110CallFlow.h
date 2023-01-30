#pragma once
#include "callflow.h"

class CTN110CallFlow :
	public CCallFlow
{
public:
	CTN110CallFlow(CLineManager& manager);
	virtual ~CTN110CallFlow(void);
	int ProcessCallFlow(CLine& line, int code, int data);
private:
	void StoreErrCode(CLine& line, int errorCode);
	int PlayBalance(CLine& line);
	void PlayErrTones(CLine& line);
public:
	int StoreBalance(CLine& line);
	int StoreTime(CLine& line);
	int PlayTime(CLine& line);
};
