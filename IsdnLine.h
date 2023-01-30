#pragma once
#include "line.h"

class CISDNLine :
	public CLine
{
public:
	CISDNLine(CLineManager* pManager);
	virtual ~CISDNLine(void);
};
