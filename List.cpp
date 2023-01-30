// List.cpp: implementation of the CList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "List.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CList::CList()
{
	for(int i=0; i<LISTSIZE; i++)
		//strcpy(List[i],'\0');
		List[i][0]='\0';
	index=0;
}

CList::~CList()
{

}
void CList::insert(char *ani)
{
	if(index<LISTSIZE) {
		for(int i=0; i<=index; i++)
			if(!strcmp(List[i],  ani))
				return;
		strcpy(List[index++], ani);
	}
}

bool CList::is_in(char *ani)
{
	for(int i=0; i<=index; i++) 
		if (!strcmp(List[i], ani) )
			return true; 
	return false; 
}

void CList::remove(char *ani)
{
	for(int i=0; i<=index; i++) 
		if (!strcmp(List[i], ani) )
			shift(i);	
}

void CList::shift(int to)
{
 for(int i=to; i<index; i++)
	strcpy(List[i],List[i+1]);
 index--;
}
