/******************************************************************************************/
/*                                                                                        */
/*	Source File		: Registry.cpp														  */
/*																						  */	
/*	Version			: 2.0																  */
/*                                                                                        */
/*	Target OS		: Windows 2000	Service Pack 3.0									  */
/*																						  */
/*	Description		: Registry Operations: Create/Open/Verify/Write/Read				  */
/*																						  */
/*	Last Revision	: <02/12/2002>													      */
/*																						  */
/*	Release Stage	: BETA																  */
/*																						  */
/*	Author			: A.T.Lloyd	(c) 2002,  All rights reserved.							  */
/*					  Tritel Technologies (Pvt.) Ltd.									  */
/******************************************************************************************/

#include "stdafx.h"
#include "Registry.h"
//#include <windows.h>
#include <Winuser.h>

CRegistry ::CRegistry(HKEY hKeyRoot)
{
	m_hKey=hKeyRoot;
}

CRegistry ::~CRegistry()
{
	if(m_hKey)
		Close();
}

BOOL CRegistry ::Open (HKEY hKeyRoot,LPCTSTR pszPath)
{
	lstrcpy(m_sPath,pszPath);

	LONG ReturnValue=RegOpenKeyEx(hKeyRoot,pszPath,0L,KEY_ALL_ACCESS,&m_hKey);

	m_Info.lMessage=ReturnValue;
	m_Info.dwSize=0L;
	m_Info.dwType=0L;

	if(ReturnValue==ERROR_SUCCESS)
		return TRUE;
	
	return FALSE;

}
void CRegistry::Close()
{
	if(m_hKey)
	{
		RegCloseKey(m_hKey);
		m_hKey=NULL;

	}

}

BOOL CRegistry::CreateKey(HKEY hKeyRoot,LPCTSTR pszPath)
{
	DWORD dw;

	LONG ReturnValue=RegCreateKeyEx(hKeyRoot,pszPath,0L,NULL,REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,NULL,&m_hKey,&dw);

	m_Info.lMessage=ReturnValue;
	m_Info.dwSize=0L;
	m_Info.dwType=0L;

	if(ReturnValue==ERROR_SUCCESS)
		return TRUE;

	return FALSE;
}

BOOL CRegistry::VerifyKey(HKEY hKeyRoot,LPCTSTR pszPath)
{

	LONG ReturnValue=RegOpenKeyEx(hKeyRoot,pszPath,0L,KEY_ALL_ACCESS,&m_hKey);

	
	m_Info.lMessage=ReturnValue;
	m_Info.dwSize=0L;
	m_Info.dwType=0L;

	if(ReturnValue==ERROR_SUCCESS)
	return TRUE;
	
	return FALSE;

}

BOOL CRegistry::Write(LPCTSTR pszKey,DWORD dwVal)
{
	
	LONG ReturnValue=RegSetValueEx(m_hKey,pszKey,0L,REG_DWORD,(CONST BYTE*)&dwVal,sizeof(DWORD));
	
	if(ReturnValue==ERROR_SUCCESS)
		return TRUE;
	
	return FALSE;
}

BOOL CRegistry::Write(LPCTSTR pszKey,LPCTSTR pszVal)
{
	
	LONG ReturnValue=RegSetValueEx(m_hKey,pszKey,0L,REG_SZ,(CONST BYTE*)pszVal,strlen(pszVal)+1);

	if(ReturnValue==ERROR_SUCCESS)
		return TRUE;
	
	return FALSE;
	
}

BOOL CRegistry::Read(LPCTSTR pszKey,DWORD& dwVal)
{
	DWORD dwType;
	DWORD dwSize=sizeof(DWORD);
	DWORD dwDest;

	LONG ReturnValue=RegQueryValueEx(m_hKey,pszKey,NULL,&dwType,(BYTE*)&dwDest,&dwSize);

	m_Info.lMessage=ReturnValue;
	m_Info.dwSize=dwSize;
	m_Info.dwType=dwType;

	if(ReturnValue==ERROR_SUCCESS)
	{
		dwVal=dwDest;
		return TRUE;
	}
	
		
	return FALSE;

}

BOOL CRegistry::Read(LPCTSTR pszKey,TCHAR tcVal[255])
{
	DWORD dwType;
	DWORD dwSize=200;
	char szString[255];

	LONG ReturnValue=RegQueryValueEx(m_hKey,pszKey,NULL,&dwType,(BYTE*)&szString,&dwSize);

	m_Info.lMessage=ReturnValue;
	m_Info.dwSize=dwSize;
	m_Info.dwType=dwType;

	if(ReturnValue==ERROR_SUCCESS)
	{
		lstrcpy(tcVal,szString);
		return TRUE;
	}
	
	return FALSE;

}