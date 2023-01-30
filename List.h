// List.h: interface for the CList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIST_H__B3DB2DD2_D4B1_42C9_905A_F59C761B0872__INCLUDED_)
#define AFX_LIST_H__B3DB2DD2_D4B1_42C9_905A_F59C761B0872__INCLUDED_

#include <string.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define LISTSIZE 100

class CList  
{
public:
	void shift(int indx);
	void remove(char* ani);
	bool is_in(char* ani);
	void insert(char* ani);
	CList();
	virtual ~CList();
private:
	char List[LISTSIZE][10];
	int index;
};

#endif // !defined(AFX_LIST_H__B3DB2DD2_D4B1_42C9_905A_F59C761B0872__INCLUDED_)
