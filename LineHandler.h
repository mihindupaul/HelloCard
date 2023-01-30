#pragma once
#include "./line.h"
#include "./ilineobserver.h"
#include "./configuration.h"

class CLineHandler 
{
//	prevent creation of objects
protected:
	CLineHandler(CLine& line);
	virtual ~CLineHandler(void);

public:
	CLine& GetLine(void);
	bool IsMyLine(CLine& line);
	// Close the handler and underlaying line
	virtual int Close(void);

private:
	CLine& m_Line;
};
