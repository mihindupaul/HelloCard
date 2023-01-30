#include "stdafx.h"
#include ".\linehandler.h"

CLineHandler::CLineHandler(CLine& line)
:m_Line(line)
{
	//	this always become a permanent observer of given line
}

CLineHandler::~CLineHandler(void)
{
}

CLine& CLineHandler::GetLine(void)
{
	return m_Line;
}

bool CLineHandler::IsMyLine(CLine& line)
{
	return (&line == &m_Line);
}

// Close the handler and underlaying line
int CLineHandler::Close(void)
{
	return 0;
//	return m_Line.Close();
}


