/******************************************************************************************/
/*                                                                                        */
/*	Source File		: Registry.h														  */
/*																						  */	
/*	Version			: 2.0																  */
/*                                                                                        */
/*	Target OS		: Windows 2000	Service Pack 3.0									  */
/*																						  */
/*	Description		: Registry Operations ProtoTypes: Create/Open/Verify/Write/Read		  */
/*																						  */
/*	Last Revision	: <22/19/2002>													      */
/*																						  */
/*	Release Stage	: BETA																  */
/*																						  */
/*	Author			: A.T.Lloyd	(c) 2002,  All rights reserved.							  */
/*					  Tritel Technologies (Pvt.) Ltd.									  */
/******************************************************************************************/
#pragma once
#include "stdafx.h"

class CRegistry  
{
	protected:
		HKEY m_hKey;
		TCHAR m_sPath[100];

	public:
		//CRegistry();
		//Constructor

		CRegistry(HKEY hKeyRoot = HKEY_LOCAL_MACHINE);

		//Destructor
		virtual ~CRegistry();

		struct REGINFO
		{

			LONG lMessage;
			DWORD dwType;
			DWORD dwSize;

		}m_Info;
		
	//Operations
	public:

		//Open The Registry
		BOOL Open (HKEY hKeyRoot,LPCTSTR pszPath);

		//Close The Registry
		void Close();

		//Create a Key
		BOOL CreateKey(HKEY hKeyRoot,LPCTSTR pszPath);

		//Verify weather a key/path is valid
		BOOL VerifyKey(HKEY hKeyRoot,LPCTSTR pszPath);

		//Write a Key (Overloading Prototypes)

		BOOL Write(LPCTSTR pszKey,DWORD dwVal);
		BOOL Write(LPCTSTR pszKey,LPCTSTR pszVal);

		//Read a Key (Overloading Prototypes)

		BOOL Read(LPCTSTR pszKey,DWORD& dwVal);
		BOOL Read(LPCTSTR pszKey,TCHAR tcVal[255]);


};

