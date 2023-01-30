#pragma once
#include <string>
#include "line.h"

class CSS7Line :
	public CLine
{
public:
	CSS7Line(CLineManager* manager);
	virtual ~CSS7Line();

	int MakeCall(std::string number,std::string caller_number);
	int Open(int index,int board,int time_slot,int vb,int vch);
	std::string GetANI() const;
	std::string GetDNIS() const;

private:

};
