/******************************************************************************************/
/*                                                                                        */
/*	Source File		: DataBaseSupport.cpp												  */
/*																						  */	
/*	Version			: 2.0																  */
/*                                                                                        */
/*	Target OS		: Windows 2000	Service Pack 3.0 MDAC 2.6							  */
/*																						  */
/*	Description		: Database Support Implementations									  */
/*																						  */
/*	Last Revision	: <05/12/2002>													      */
/*																						  */
/*	Release Stage	: BETA																  */
/*																						  */
/*	Author			: A.T.Lloyd	(c) 2002,  All rights reserved.							  */
/*					  Tritel Technologies (Pvt.) Ltd.									  */
/******************************************************************************************/

#include "stdafx.h"

#include "DatabaseSupport.h"
#include <stddef.h>
#include <stdio.h>
#include "Registry.h"
#include "atlbase.h"

#include ".\databasesupport.h"
//	v.4 
using namespace Tritel;

CDatabaseSupport::CDatabaseSupport()
{
	m_pIDBInitialize		= NULL;
	m_pIDBCreateSession		= NULL;
	m_pIDBCreateCommand		= NULL;
	m_pICommandText			= NULL;
	m_pICommandWithParams	= NULL;
	m_pIAccessor			= NULL;
	m_pIRowset				= NULL;
	m_hAccessor				= DB_INVALID_HACCESSOR;
	m_pIDataInitialize		= NULL;
}

CDatabaseSupport::~CDatabaseSupport()
{

}

void CDatabaseSupport::Uninitialize()
{
	if(NULL !=m_pIDataInitialize)
	{
		m_pIDataInitialize->Release();
		m_pIDataInitialize=NULL;
	}

	// Uninitialize OLE.
    OleUninitialize();

}

HRESULT CDatabaseSupport::ConnectToDatabase()
{
	
	HRESULT				hResult;
    const ULONG         nInitProps = 4;
    const ULONG         nPropSet = 1;
    DBPROP              InitProperties[nInitProps];
    DBPROPSET           rgInitPropSet[nPropSet];
	CRegistry Registry;
	TCHAR tcDataSource[25],tcCataLog[25],tcUserId[25],tcPassWord[25];

	//Initialize property arrays
    AZERO(InitProperties);
    AZERO(rgInitPropSet);

	/**************Read the Information From The Registry**************/


	//Open Registry Key
	if(Registry.Open(HKEY_LOCAL_MACHINE,TEXT("Software\\Tritel\\Hello\\DataSource"))==FALSE)
	{
		return 0;
	}

	//DataSource

	if(Registry.Read("DataSource",tcDataSource)==TRUE)
	{
	}
	else
		return 0;
	//CataLog

	if(Registry.Read("CataLog",tcCataLog)==TRUE)
	{
	}
	else
		return 0;
	//UserID

	if(Registry.Read("UserId",tcUserId)==TRUE)
	{
	}
	else
		return 0;
	
	//Password
	if(Registry.Read("PassWord",tcPassWord)==TRUE)
	{
	}
	else
		return 0;

	//Now Close Registry
	Registry.Close();


	/******************************************************************/

	// Initialize OLE
    if( FAILED( hResult = OleInitialize( NULL ) ) )
    {
        // Handle errors here.
		return hResult;
    }

    if(m_pIDataInitialize==NULL)
	{

		//Initialize the COM library.
		CoInitialize(NULL);
		hResult = CoCreateInstance(CLSID_MSDAINITIALIZE,
							  NULL,
							  CLSCTX_INPROC_SERVER,
							  IID_IDataInitialize,
							  (void **)&m_pIDataInitialize);
		
		if(FAILED(hResult))
		{
			return hResult;
		}
	}

	char chConnectString[1024];
	WCHAR   pchData[2048];

	sprintf(chConnectString,"PROVIDER=SQLOLEDB;DATA SOURCE=%s;USER ID=%s;PASSWORD=%s;INITIAL CATALOG=%s;",tcDataSource,tcUserId,tcPassWord,tcCataLog);
	
	MultiByteToWideChar(CP_ACP, MB_USEGLYPHCHARS, chConnectString, -1, pchData, 2048);
	
	BSTR bstrConnectString = ::SysAllocString(pchData);

	m_pIDBInitialize=NULL;
	hResult=m_pIDataInitialize->GetDataSource(NULL,
											CLSCTX_INPROC_SERVER,
                                            bstrConnectString,
											IID_IDBInitialize,
                                            (IUnknown**)&m_pIDBInitialize);

	
	::SysFreeString(bstrConnectString);
	

	if(FAILED(hResult))
    {
        return 0;
    }

	// Now establish a connection to the data source.
    hResult = m_pIDBInitialize->Initialize();
	if(FAILED(hResult))
    {
        return hResult;
    }

	return S_OK;
}

HRESULT CDatabaseSupport::CreateDatabaseSession()
{
	HRESULT				hResult;

	hResult = ConnectToDatabase();

    if(FAILED(hResult))
    {
        // can't connect to database for some reason...
    
		return hResult;
    }

	// Create a new session from the data source object (DSO)
    hResult = m_pIDBInitialize->QueryInterface(IID_IDBCreateSession,
                                        (void**) &m_pIDBCreateSession);
    if(FAILED(hResult))
    {
    
		CloseDatabaseSession();

		return hResult;
    }


    // Get the IID_IDBCreateCommand interface to create the command...
    hResult = m_pIDBCreateSession->CreateSession(NULL, 
                                          IID_IDBCreateCommand, 
                                          (IUnknown**) &m_pIDBCreateCommand);

	//	Create smart COM Ptr (Addition)


    if(FAILED(hResult))
    {
        
        CloseDatabaseSession();

		return hResult;
    }

	//	ATL Gimmik
//	m_pDBCreateSession->CreateSession(NULL,IID_IDBCreateCommand,(IUnknown**)&m_pDBCreateCommand);	

	return S_OK;
}

void CDatabaseSupport::CloseDatabaseSession()
{
	if(m_pIRowset != NULL)
    {
        m_pIRowset->Release();
    }

    if(m_pIAccessor != NULL)
    {
        if(m_hAccessor != DB_INVALID_HACCESSOR)
        {
            m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
        }
        m_pIAccessor->Release();
    }
    
    if(m_pICommandWithParams != NULL)
    {
        m_pICommandWithParams->Release();
    }

    if(m_pICommandText != NULL)
    {
        m_pICommandText->Release();
    }
    
    if(m_pIDBCreateCommand != NULL)
    {
        m_pIDBCreateCommand->Release();
    }
    
    if(m_pIDBCreateSession != NULL)
    {
        m_pIDBCreateSession->Release();
    }
    
    if(m_pIDBInitialize != NULL)
    {
		m_pIDBInitialize->Release();
	}
}


BOOL CDatabaseSupport::GetCustomerType(CUSTOMERTYPE* pCUSTOMERTYPE)
{

	HRESULT				hResult;
	const int			nParams = 8;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;

	// Create and initialize database connection
    hResult = CreateDatabaseSession();

    if(FAILED(hResult))
    {
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spCustomerType(?,?,?,?,?,?,?)}");


	// Initialize the stored procedure parameter structure

	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        CloseDatabaseSession();
		return 0;   // failure...!!
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);

    if(FAILED(hResult))
    {
        CloseDatabaseSession();
		return 0;   // failure...!!
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;


	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcCallerID";           
    ParamBindInfo[1].ulParamSize = 16;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[2].pwszName = L"@vcAreaCode";      
    ParamBindInfo[2].ulParamSize = 6;					
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[3].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[3].pwszName = L"@vcLanguage";      
    ParamBindInfo[3].ulParamSize = 21;					
    ParamBindInfo[3].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[4].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[4].pwszName = L"@nPlanID";           
    ParamBindInfo[4].ulParamSize = 4;					
    ParamBindInfo[4].dwFlags = DBPARAMFLAGS_ISOUTPUT;
	
	ParamBindInfo[5].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[5].pwszName = L"@fCurrentBalance";           
    ParamBindInfo[5].ulParamSize = 4;					
    ParamBindInfo[5].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[6].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[6].pwszName = L"@nBusinessCode";           
    ParamBindInfo[6].ulParamSize = 4;					
    ParamBindInfo[6].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[7].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[7].pwszName = L"@nCustomerType";           
    ParamBindInfo[7].ulParamSize = 4;				
    ParamBindInfo[7].dwFlags = DBPARAMFLAGS_ISOUTPUT;




	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        CloseDatabaseSession();
		return 0;   // failure...!!
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }


    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }



	acDBBinding[0].obValue = offsetof(CUSTOMERTYPE,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(CUSTOMERTYPE,chCallerID );
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 11;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(CUSTOMERTYPE,chAreaCode );
    acDBBinding[2].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[2].cbMaxLen = 11;
    acDBBinding[2].wType = DBTYPE_STR;

	acDBBinding[3].obValue = offsetof(CUSTOMERTYPE,chLanguage );
    acDBBinding[3].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[3].cbMaxLen = 11;
    acDBBinding[3].wType = DBTYPE_STR;

	acDBBinding[4].obValue = offsetof(CUSTOMERTYPE,nPlanID );
    acDBBinding[4].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[4].cbMaxLen = 4;
    acDBBinding[4].wType = DBTYPE_I4;

	acDBBinding[5].obValue = offsetof(CUSTOMERTYPE,fCurrentBalance );
    acDBBinding[5].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[5].cbMaxLen = 4;
    acDBBinding[5].wType = DBTYPE_R4;


	acDBBinding[6].obValue = offsetof(CUSTOMERTYPE,nBusinessCode );
    acDBBinding[6].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[6].cbMaxLen = 4;
    acDBBinding[6].wType = DBTYPE_I4;

	acDBBinding[7].obValue = offsetof(CUSTOMERTYPE,nCustomerType );
    acDBBinding[7].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[7].cbMaxLen = 4;
    acDBBinding[7].wType = DBTYPE_I4;

	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!
    }
 
	hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(CUSTOMERTYPE), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        CloseDatabaseSession();
		return 0;  // failure...!!
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pCUSTOMERTYPE;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
    
    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
		CloseDatabaseSession();
		return 0;  // failure...!!
    }
	else
	{
		CloseDatabaseSession();
		return 1; //Sucess
	}

	//	i should release all objects created above

	return 0;
}
BOOL CDatabaseSupport::GetPin(PINCLI *pPINCLI)
{
	HRESULT				hResult;
	const int			nParams = 3;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return -1;
    }


	wsprintfW(wCmdString,L"{?=call hello.spGetPin(?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcCallerId";           
    ParamBindInfo[1].ulParamSize = 16;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;
	
	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[2].pwszName = L"@vcPinNumber";           
    ParamBindInfo[2].ulParamSize = 11;					
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISOUTPUT;

		
	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
   
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }



	acDBBinding[0].obValue = offsetof(PINCLI,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;
	
	acDBBinding[1].obValue = offsetof(PINCLI,chCallerID);
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 11;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(PINCLI,chPinNumber);
    acDBBinding[2].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[2].cbMaxLen = 11;
    acDBBinding[2].wType = DBTYPE_STR;

	
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(PINAUTHENTICATE), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pPINCLI;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
    
    
    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
		 //Success
		
		CloseDatabaseSession();
		return 1;

	}

	return 0;
};

BOOL CDatabaseSupport::GetPinAuthentication(PINAUTHENTICATE *pPINAUTHENTICATE)
{
	HRESULT				hResult;
	const int			nParams = 11;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return -1;
    }


	wsprintfW(wCmdString,L"{?=call hello.spPinAuthentication(?,?,?,?,?,?,?,?,?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcPinNumber";           
    ParamBindInfo[1].ulParamSize = 11;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[2].pwszName = L"@nCustomerType";      
    ParamBindInfo[2].ulParamSize = 4;					 
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[3].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[3].pwszName = L"@vcBusinessDescription";      
    ParamBindInfo[3].ulParamSize = 51;					
    ParamBindInfo[3].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[4].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[4].pwszName = L"@vcLanguage";      
    ParamBindInfo[4].ulParamSize = 11;					
    ParamBindInfo[4].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[5].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[5].pwszName = L"@vcLangCodeStatus";     
    ParamBindInfo[5].ulParamSize = 8;					 
    ParamBindInfo[5].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[6].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[6].pwszName = L"@fCurrentBalance";      
    ParamBindInfo[6].ulParamSize = 4;					 
    ParamBindInfo[6].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[7].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[7].pwszName = L"@nBusinessCode";      
    ParamBindInfo[7].ulParamSize = 4;					 
    ParamBindInfo[7].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[8].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[8].pwszName = L"@nPlanID";      
    ParamBindInfo[8].ulParamSize = 4;					 
    ParamBindInfo[8].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[9].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[9].pwszName = L"@vcSerialNo";      
    ParamBindInfo[9].ulParamSize = 21;					
    ParamBindInfo[9].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[10].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[10].pwszName = L"@vcWelcome";      
    ParamBindInfo[10].ulParamSize = 51;					 
    ParamBindInfo[10].dwFlags = DBPARAMFLAGS_ISOUTPUT;
	
	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
   
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }



	acDBBinding[0].obValue = offsetof(PINAUTHENTICATE,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(PINAUTHENTICATE,chPinNumber);
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 11;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(PINAUTHENTICATE,nCustomerType);
    acDBBinding[2].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[2].cbMaxLen = 4;
    acDBBinding[2].wType = DBTYPE_I4;

	acDBBinding[3].obValue = offsetof(PINAUTHENTICATE,chBusinessDescription);
    acDBBinding[3].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[3].cbMaxLen = 51;
    acDBBinding[3].wType = DBTYPE_STR;

	acDBBinding[4].obValue = offsetof(PINAUTHENTICATE,chLanguage);
    acDBBinding[4].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[4].cbMaxLen = 11;
    acDBBinding[4].wType = DBTYPE_STR;

	acDBBinding[5].obValue = offsetof(PINAUTHENTICATE,chLangCodeStatus);
    acDBBinding[5].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[5].cbMaxLen = 8;
    acDBBinding[5].wType = DBTYPE_STR;

	acDBBinding[6].obValue = offsetof(PINAUTHENTICATE,fCurrentBalance);
    acDBBinding[6].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[6].cbMaxLen = 4;
    acDBBinding[6].wType = DBTYPE_R4;

	acDBBinding[7].obValue = offsetof(PINAUTHENTICATE,nBusinessCode);
    acDBBinding[7].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[7].cbMaxLen = 4;
    acDBBinding[7].wType = DBTYPE_I4;

	acDBBinding[8].obValue = offsetof(PINAUTHENTICATE,nPlanID);
    acDBBinding[8].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[8].cbMaxLen = 4;
    acDBBinding[8].wType = DBTYPE_I4;

	acDBBinding[9].obValue = offsetof(PINAUTHENTICATE,chSerialNo);
    acDBBinding[9].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[9].cbMaxLen = 21;
    acDBBinding[9].wType = DBTYPE_STR;

	acDBBinding[10].obValue = offsetof(PINAUTHENTICATE,chWelcome);
    acDBBinding[10].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[10].cbMaxLen = 51;
    acDBBinding[10].wType = DBTYPE_STR;



	
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(PINAUTHENTICATE), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pPINAUTHENTICATE;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
    
    
    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
		 //Success
		
		CloseDatabaseSession();
		return 1;

	}

	return 0;
};

BOOL CDatabaseSupport::GetDigits(DIGITS *pDIGITS)
{

	HRESULT				hResult;
	const int			nParams = 4;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spLNDestinationDigits2(?,?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
         ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           //  @ddn_name name seems to have no significance
    ParamBindInfo[0].ulParamSize = 4;					// currently set to 128
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;


	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcPrefixDestNo";           //  @ddn_name name seems to have no significance
    ParamBindInfo[1].ulParamSize = 4;					// currently set to 128
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[2].pwszName = L"@vcAreaCode";      // name must match SP arg name
    ParamBindInfo[2].ulParamSize = 6;					// 
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[3].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[3].pwszName = L"@nDestinationDigits";      // name must match SP arg name
    ParamBindInfo[3].ulParamSize = 4;					// 
    ParamBindInfo[3].dwFlags = DBPARAMFLAGS_ISOUTPUT;


	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }


    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }


	acDBBinding[0].obValue = offsetof(DIGITS,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(DIGITS,chPrefixDestNo);
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 4;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(DIGITS,chAreaCode);
    acDBBinding[2].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[2].cbMaxLen = 7;
    acDBBinding[2].wType = DBTYPE_STR;

	acDBBinding[3].obValue = offsetof(DIGITS,nDestinationDigits);
    acDBBinding[3].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[3].cbMaxLen = 4;
    acDBBinding[3].wType = DBTYPE_I4;

	
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(DIGITS), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pDIGITS;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
      
    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
       		
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
			CloseDatabaseSession();
			return 1; //Sucess
	}

	return 0;
}
/*
BOOL CDatabaseSupport::GetCallRate(CALLRATE *pCALLRATE)
{

	
	HRESULT				hResult;
	const int			nParams = 23;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spCallRate(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
         ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;


	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcAuthorizationKey";           
    ParamBindInfo[1].ulParamSize = 16;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[2].pwszName = L"@vcDestinationNumber";      
    ParamBindInfo[2].ulParamSize = 26;					 
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[3].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[3].pwszName = L"@nBusinessCode";      
    ParamBindInfo[3].ulParamSize = 4;					
    ParamBindInfo[3].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[4].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[4].pwszName = L"@nPlanID";      
    ParamBindInfo[4].ulParamSize = 4;			
    ParamBindInfo[4].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[5].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[5].pwszName = L"@vcAreaCode";      
    ParamBindInfo[5].ulParamSize = 6;					 
    ParamBindInfo[5].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[6].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[6].pwszName = L"@vcIpAddress";      
    ParamBindInfo[6].ulParamSize = 51;					 
    ParamBindInfo[6].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[7].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[7].pwszName = L"@nBlock1";     
    ParamBindInfo[7].ulParamSize = 4;					
    ParamBindInfo[7].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[8].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[8].pwszName = L"@nBlock2";      
    ParamBindInfo[8].ulParamSize = 4;					 
    ParamBindInfo[8].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[9].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[9].pwszName = L"@nBlock3";      
    ParamBindInfo[9].ulParamSize = 4;					
    ParamBindInfo[9].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[10].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[10].pwszName = L"@fCost1";           
    ParamBindInfo[10].ulParamSize = 4;					
    ParamBindInfo[10].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[11].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[11].pwszName = L"@fCost2";           
    ParamBindInfo[11].ulParamSize = 4;					
    ParamBindInfo[11].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[12].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[12].pwszName = L"@fCost3";           
    ParamBindInfo[12].ulParamSize = 4;					
    ParamBindInfo[12].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[13].pwszDataSourceType = L"DBTYPE_I4";
    ParamBindInfo[13].pwszName = L"@nAvailableTime";      
    ParamBindInfo[13].ulParamSize = 8;					 
    ParamBindInfo[13].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[14].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[14].pwszName = L"@vcDestinationCode";      
    ParamBindInfo[14].ulParamSize = 16;					 
    ParamBindInfo[14].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[15].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[15].pwszName = L"@vcCallType";      
    ParamBindInfo[15].ulParamSize = 51;					
    ParamBindInfo[15].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[16].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[16].pwszName = L"@nRateID";      
    ParamBindInfo[16].ulParamSize = 4;					
    ParamBindInfo[16].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[17].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[17].pwszName = L"@vcDialString";      
    ParamBindInfo[17].ulParamSize = 26;					
    ParamBindInfo[17].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[18].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[18].pwszName = L"@vcSpCode";      
    ParamBindInfo[18].ulParamSize = 16;					
    ParamBindInfo[18].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[19].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[19].pwszName = L"@vcRoutingMethod";      
    ParamBindInfo[19].ulParamSize = 16;					
    ParamBindInfo[19].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[20].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[20].pwszName = L"@nOStart";      
    ParamBindInfo[20].ulParamSize = 4;					
    ParamBindInfo[20].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[21].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[21].pwszName = L"@nOEnd";      
    ParamBindInfo[21].ulParamSize = 4;					
    ParamBindInfo[21].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[22].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[22].pwszName = L"@fAUCharge";           
    ParamBindInfo[22].ulParamSize = 4;					
    ParamBindInfo[22].dwFlags = DBPARAMFLAGS_ISOUTPUT;


	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
   		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }

	acDBBinding[0].obValue = offsetof(CALLRATE,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(CALLRATE,chAuthorizationKey);
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 16;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(CALLRATE,chDestinationNumber);
    acDBBinding[2].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[2].cbMaxLen = 26;
    acDBBinding[2].wType = DBTYPE_STR;

	acDBBinding[3].obValue = offsetof(CALLRATE,nBusinessCode);
    acDBBinding[3].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[3].cbMaxLen = 4;
    acDBBinding[3].wType = DBTYPE_I4;

	acDBBinding[4].obValue = offsetof(CALLRATE,nPlanID);
    acDBBinding[4].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[4].cbMaxLen = 4;
    acDBBinding[4].wType = DBTYPE_I4;

	acDBBinding[5].obValue = offsetof(CALLRATE,chAreaCode);
    acDBBinding[5].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[5].cbMaxLen = 6;
    acDBBinding[5].wType = DBTYPE_STR;

	acDBBinding[6].obValue = offsetof(CALLRATE,chIPAddress);
    acDBBinding[6].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[6].cbMaxLen = 51;
    acDBBinding[6].wType = DBTYPE_STR;

	acDBBinding[7].obValue = offsetof(CALLRATE,nBlock1);
    acDBBinding[7].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[7].cbMaxLen = 4;
    acDBBinding[7].wType = DBTYPE_I4;

	acDBBinding[8].obValue = offsetof(CALLRATE,nBlock2);
    acDBBinding[8].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[8].cbMaxLen = 4;
    acDBBinding[8].wType = DBTYPE_I4;

	acDBBinding[9].obValue = offsetof(CALLRATE,nBlock3);
    acDBBinding[9].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[9].cbMaxLen = 4;
    acDBBinding[9].wType = DBTYPE_I4;

	acDBBinding[10].obValue = offsetof(CALLRATE,fCost1);
    acDBBinding[10].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[10].cbMaxLen = 4;
    acDBBinding[10].wType = DBTYPE_R4;

	acDBBinding[11].obValue = offsetof(CALLRATE,fCost2);
    acDBBinding[11].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[11].cbMaxLen = 4;
    acDBBinding[11].wType = DBTYPE_R4;

	acDBBinding[12].obValue = offsetof(CALLRATE,fCost3);
    acDBBinding[12].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[12].cbMaxLen = 4;
    acDBBinding[12].wType = DBTYPE_R4;

	acDBBinding[13].obValue = offsetof(CALLRATE,nAvailableTime);
    acDBBinding[13].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[13].cbMaxLen = 8;
    acDBBinding[13].wType = DBTYPE_I4;

	acDBBinding[14].obValue = offsetof(CALLRATE,chDestinationCode);
    acDBBinding[14].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[14].cbMaxLen = 16;
    acDBBinding[14].wType = DBTYPE_STR;

	acDBBinding[15].obValue = offsetof(CALLRATE,chCallType);
    acDBBinding[15].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[15].cbMaxLen = 51;
    acDBBinding[15].wType = DBTYPE_STR;

	acDBBinding[16].obValue = offsetof(CALLRATE,nRateID);
    acDBBinding[16].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[16].cbMaxLen = 4;
    acDBBinding[16].wType = DBTYPE_I4;

	acDBBinding[17].obValue = offsetof(CALLRATE,chDialString);
    acDBBinding[17].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[17].cbMaxLen = 26;
    acDBBinding[17].wType = DBTYPE_STR;

	acDBBinding[18].obValue = offsetof(CALLRATE,chSpCode);
    acDBBinding[18].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[18].cbMaxLen = 16;
    acDBBinding[18].wType = DBTYPE_STR;

	acDBBinding[19].obValue = offsetof(CALLRATE,chRoutingMethod);
    acDBBinding[19].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[19].cbMaxLen = 16;
    acDBBinding[19].wType = DBTYPE_STR;

	acDBBinding[20].obValue = offsetof(CALLRATE,nOStart);
    acDBBinding[20].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[20].cbMaxLen = 4;
    acDBBinding[20].wType = DBTYPE_I4;

	acDBBinding[21].obValue = offsetof(CALLRATE,nOEnd);
    acDBBinding[21].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[21].cbMaxLen = 4;
    acDBBinding[21].wType = DBTYPE_I4;

	acDBBinding[22].obValue = offsetof(CALLRATE,fAUCharge);
    acDBBinding[22].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[22].cbMaxLen = 4;
    acDBBinding[22].wType = DBTYPE_R4;

	
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(CALLRATE), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pCALLRATE;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
    
    
    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
		CloseDatabaseSession();
		return 1; //Sucess
	}
	
	return 0;

}

*/
BOOL CDatabaseSupport::SetComCallLog(COMCALLLOG *pCOMCALLLOG)
{

	HRESULT				hResult;
	const int			nParams = 20;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spCommunicationCallLog(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;


	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcCallerID";           
    ParamBindInfo[1].ulParamSize = 16;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[2].pwszName = L"@vcDNI";      
    ParamBindInfo[2].ulParamSize = 26;					
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[3].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[3].pwszName = L"@vcDate";      
    ParamBindInfo[3].ulParamSize = 51;					 
    ParamBindInfo[3].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[4].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[4].pwszName = L"@vcTime";      
    ParamBindInfo[4].ulParamSize = 51;					
    ParamBindInfo[4].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[5].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[5].pwszName = L"@nCallDuration";      
    ParamBindInfo[5].ulParamSize = 4;					
    ParamBindInfo[5].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[6].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[6].pwszName = L"@nUnits";      
    ParamBindInfo[6].ulParamSize = 4;					 
    ParamBindInfo[6].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[7].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[7].pwszName = L"@fCallCharge";      
    ParamBindInfo[7].ulParamSize = 4;					
    ParamBindInfo[7].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[8].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[8].pwszName = L"@fActualCost";           
    ParamBindInfo[8].ulParamSize = 4;					
    ParamBindInfo[8].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[9].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[9].pwszName = L"@vcCallType";           
    ParamBindInfo[9].ulParamSize = 51;					
    ParamBindInfo[9].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[10].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[10].pwszName = L"@vcDestinationCode";           
    ParamBindInfo[10].ulParamSize = 11;					
    ParamBindInfo[10].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[11].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[11].pwszName = L"@nPlanID";      
    ParamBindInfo[11].ulParamSize = 4;					 
    ParamBindInfo[11].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[12].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[12].pwszName = L"@nRateID";      
    ParamBindInfo[12].ulParamSize = 4;					 
    ParamBindInfo[12].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[13].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[13].pwszName = L"@vcSpCode";      
    ParamBindInfo[13].ulParamSize = 11;					 
    ParamBindInfo[13].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[14].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[14].pwszName = L"@nOutgoingLineNumber";      
    ParamBindInfo[14].ulParamSize = 4;					 
    ParamBindInfo[14].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[15].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[15].pwszName = L"@vcIPAddress";      
    ParamBindInfo[15].ulParamSize = 51;					 
    ParamBindInfo[15].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[16].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[16].pwszName = L"@vcCallStatus";      
    ParamBindInfo[16].ulParamSize = 26;					 
    ParamBindInfo[16].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[17].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[17].pwszName = L"@vcRoutingMethod";      
    ParamBindInfo[17].ulParamSize =26;					 
    ParamBindInfo[17].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[18].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[18].pwszName = L"@nBusinessCode";      
    ParamBindInfo[18].ulParamSize =4;					 
    ParamBindInfo[18].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[19].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[19].pwszName = L"@fUnitCharge";      
    ParamBindInfo[19].ulParamSize =26;					
    ParamBindInfo[19].dwFlags = DBPARAMFLAGS_ISINPUT;


	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }


	acDBBinding[0].obValue = offsetof(COMCALLLOG,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(COMCALLLOG,chCallerID);
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 16;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(COMCALLLOG,chDNI);
    acDBBinding[2].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[2].cbMaxLen = 26;
    acDBBinding[2].wType = DBTYPE_STR;

	acDBBinding[3].obValue = offsetof(COMCALLLOG,chDate);
    acDBBinding[3].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[3].cbMaxLen = 51;
    acDBBinding[3].wType = DBTYPE_STR;

	acDBBinding[4].obValue = offsetof(COMCALLLOG,chTime);
    acDBBinding[4].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[4].cbMaxLen = 51;
    acDBBinding[4].wType = DBTYPE_STR;

	acDBBinding[5].obValue = offsetof(COMCALLLOG,nCallDuration);
    acDBBinding[5].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[5].cbMaxLen = 4;
    acDBBinding[5].wType = DBTYPE_I4;

	acDBBinding[6].obValue = offsetof(COMCALLLOG,nUnits);
    acDBBinding[6].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[6].cbMaxLen = 4;
    acDBBinding[6].wType = DBTYPE_I4;

	acDBBinding[7].obValue = offsetof(COMCALLLOG, fCallCharge);
    acDBBinding[7].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[7].cbMaxLen = 4;
    acDBBinding[7].wType = DBTYPE_R4;

	acDBBinding[8].obValue = offsetof(COMCALLLOG,fActualCost);
    acDBBinding[8].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[8].cbMaxLen = 4;
    acDBBinding[8].wType = DBTYPE_R4;

	acDBBinding[9].obValue = offsetof(COMCALLLOG,chCallType);
    acDBBinding[9].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[9].cbMaxLen = 51;
    acDBBinding[9].wType = DBTYPE_STR;

	acDBBinding[10].obValue = offsetof(COMCALLLOG,chDestinationCode);
    acDBBinding[10].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[10].cbMaxLen = 11;
    acDBBinding[10].wType = DBTYPE_STR;

	acDBBinding[11].obValue = offsetof(COMCALLLOG,nPlanID);
    acDBBinding[11].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[11].cbMaxLen = 4;
    acDBBinding[11].wType = DBTYPE_I4;

	acDBBinding[12].obValue = offsetof(COMCALLLOG,nRateID);
    acDBBinding[12].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[12].cbMaxLen = 16;
    acDBBinding[12].wType = DBTYPE_I4;

	acDBBinding[13].obValue = offsetof(COMCALLLOG,chSpCode);
    acDBBinding[13].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[13].cbMaxLen = 11;
    acDBBinding[13].wType = DBTYPE_STR;

	acDBBinding[14].obValue = offsetof(COMCALLLOG,nOutgoingLineNumber);
    acDBBinding[14].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[14].cbMaxLen = 4;
    acDBBinding[14].wType = DBTYPE_I4;

	acDBBinding[15].obValue = offsetof(COMCALLLOG,chIPAddress);
    acDBBinding[15].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[15].cbMaxLen = 51;
    acDBBinding[15].wType = DBTYPE_STR;

	acDBBinding[16].obValue = offsetof(COMCALLLOG,chCallStatus);
    acDBBinding[16].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[16].cbMaxLen = 26;
    acDBBinding[16].wType = DBTYPE_STR;

	acDBBinding[17].obValue = offsetof(COMCALLLOG,chRoutingMethod);
    acDBBinding[17].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[17].cbMaxLen = 26;
    acDBBinding[17].wType = DBTYPE_STR;

	acDBBinding[18].obValue = offsetof(COMCALLLOG,nBusinessCode);
    acDBBinding[18].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[18].cbMaxLen = 4;
    acDBBinding[18].wType = DBTYPE_I4;

	acDBBinding[19].obValue = offsetof(COMCALLLOG,fUnitCharge);
    acDBBinding[19].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[19].cbMaxLen = 4;
    acDBBinding[19].wType = DBTYPE_R4;

		
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(COMCALLLOG), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pCOMCALLLOG;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
    
    
    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
			CloseDatabaseSession();
			return 1; //Sucess
	}

	return 0;
}

BOOL CDatabaseSupport::GetComTotalBusiness(COMTOTALBUSINESS *pCOMTOTALBUSINESS)
{
	HRESULT				hResult;
	const int			nParams = 4;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spCommunicationTotalBusiness(?,?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;


	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcCallerID";           
    ParamBindInfo[1].ulParamSize = 31;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[2].pwszName = L"@vcPassWord";      
    ParamBindInfo[2].ulParamSize = 6;					
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[3].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[3].pwszName = L"@fTotalAmount";      
    ParamBindInfo[3].ulParamSize = 21;					
    ParamBindInfo[3].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	
	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }



	acDBBinding[0].obValue = offsetof(COMTOTALBUSINESS,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(COMTOTALBUSINESS,chCallerID );
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 31;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(COMTOTALBUSINESS,chPassWord );
    acDBBinding[2].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[2].cbMaxLen = 6;
    acDBBinding[2].wType = DBTYPE_STR;

	acDBBinding[3].obValue = offsetof(COMTOTALBUSINESS,fTotalAmount );
    acDBBinding[3].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[3].cbMaxLen = 4;
    acDBBinding[3].wType = DBTYPE_R4;

	
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(COMTOTALBUSINESS), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pCOMTOTALBUSINESS;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
        

    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
		 //Success
	
		CloseDatabaseSession();
		return 1;

	}


	return 0;
}

BOOL CDatabaseSupport::SetComRecharge(COMRECHARGE *pCOMRECHARGE)
{
	HRESULT				hResult;
	const int			nParams = 5;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spCommunicationRecharge(?,?,?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;


	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcCallerID";          
    ParamBindInfo[1].ulParamSize = 16;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[2].pwszName = L"@nBusinessCode";      
    ParamBindInfo[2].ulParamSize = 4;					
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[3].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[3].pwszName = L"@vcPinNumber";      
    ParamBindInfo[3].ulParamSize = 17;					
    ParamBindInfo[3].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[4].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[4].pwszName = L"@fCurrentValue";      
    ParamBindInfo[4].ulParamSize = 4;					
    ParamBindInfo[4].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	
	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }



	acDBBinding[0].obValue = offsetof(COMRECHARGE,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(COMRECHARGE,chCallerID );
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 16;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(COMRECHARGE,nBusinessCode);
    acDBBinding[2].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[2].cbMaxLen = 4;
    acDBBinding[2].wType = DBTYPE_I4;

	acDBBinding[3].obValue = offsetof(COMRECHARGE,chPinNumber );
    acDBBinding[3].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[3].cbMaxLen = 17;
    acDBBinding[3].wType = DBTYPE_STR;

	acDBBinding[4].obValue = offsetof(COMRECHARGE,fCurrentValue);
    acDBBinding[4].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[4].cbMaxLen = 4;
    acDBBinding[4].wType = DBTYPE_R4;

	
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(COMRECHARGE), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pCOMRECHARGE;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
        

    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{	 //Success
		
		CloseDatabaseSession();
		return 1;

	}

	return 0;
	
}

BOOL CDatabaseSupport::GetComCallHistory(COMCALLHISTORY *pCOMCALLHISTORY)
{
	
	HRESULT				hResult;
	const int			nParams = 3;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spCommunicationCallHistory(?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;


	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcCallerID";           
    ParamBindInfo[1].ulParamSize = 31;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[2].pwszName = L"@vcPassWord";      
    ParamBindInfo[2].ulParamSize = 5;					
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISINPUT;

	
	
	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }


    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }



	acDBBinding[0].obValue = offsetof(COMCALLHISTORY,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(COMCALLHISTORY,chCallerID );
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 31;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(COMCALLHISTORY,chPassWord);
    acDBBinding[2].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[2].cbMaxLen = 5;
    acDBBinding[2].wType = DBTYPE_STR;

	
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(COMCALLHISTORY), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pCOMCALLHISTORY;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
      

    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
		 //Success
		
		CloseDatabaseSession();
		return 1;

	}

	return 0;
}
/*
BOOL CDatabaseSupport::SetUpdateCallCharge(UPDATECALLCHARGE *pUPDATECALLCHARGE)
{
	HRESULT				hResult;
	const int			nParams = 11;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spUpdateCallCharge(?,?,?,?,?,?,?,?,?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcAuthorizationKey";           
    ParamBindInfo[1].ulParamSize = 16;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[2].pwszName = L"@nNumberOfSecons";      
    ParamBindInfo[2].ulParamSize = 4;					 
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[3].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[3].pwszName = L"@nBusinessCode";      
    ParamBindInfo[3].ulParamSize = 4;					
    ParamBindInfo[3].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[4].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[4].pwszName = L"@nBlock1";      
    ParamBindInfo[4].ulParamSize = 4;					 
    ParamBindInfo[4].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[5].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[5].pwszName = L"@nBlock2";      
    ParamBindInfo[5].ulParamSize = 4;					 
    ParamBindInfo[5].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[6].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[6].pwszName = L"@nBlock3";      
    ParamBindInfo[6].ulParamSize = 4;					 
    ParamBindInfo[6].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[7].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[7].pwszName = L"@fCost1";      
    ParamBindInfo[7].ulParamSize = 4;					 
    ParamBindInfo[7].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[8].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[8].pwszName = L"@fCost2";      
    ParamBindInfo[8].ulParamSize = 4;					 
    ParamBindInfo[8].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[9].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[9].pwszName = L"@fCost3";      
    ParamBindInfo[9].ulParamSize = 4;					
    ParamBindInfo[9].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[10].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[10].pwszName = L"@fUsedValue";      
    ParamBindInfo[10].ulParamSize = 4;					 
    ParamBindInfo[10].dwFlags = DBPARAMFLAGS_ISOUTPUT;
	
	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }



	acDBBinding[0].obValue = offsetof(UPDATECALLCHARGE,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(UPDATECALLCHARGE,chAuthorizationKey );
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 21;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(UPDATECALLCHARGE,nNumberOfSecons);
    acDBBinding[2].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[2].cbMaxLen = 4;
    acDBBinding[2].wType = DBTYPE_I4;

	acDBBinding[3].obValue = offsetof(UPDATECALLCHARGE,nBusinessCode);
    acDBBinding[3].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[3].cbMaxLen = 4;
    acDBBinding[3].wType = DBTYPE_I4;

	acDBBinding[4].obValue = offsetof(UPDATECALLCHARGE,nBlock1);
    acDBBinding[4].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[4].cbMaxLen = 4;
    acDBBinding[4].wType = DBTYPE_I4;

	acDBBinding[5].obValue = offsetof(UPDATECALLCHARGE,nBlock2);
    acDBBinding[5].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[5].cbMaxLen = 4;
    acDBBinding[5].wType = DBTYPE_I4;

	acDBBinding[6].obValue = offsetof(UPDATECALLCHARGE,nBlock3);
    acDBBinding[6].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[6].cbMaxLen = 4;
    acDBBinding[6].wType = DBTYPE_I4;

	acDBBinding[7].obValue = offsetof(UPDATECALLCHARGE,fCost1);
    acDBBinding[7].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[7].cbMaxLen = 4;
    acDBBinding[7].wType = DBTYPE_R4;

	acDBBinding[8].obValue = offsetof(UPDATECALLCHARGE,fCost2);
    acDBBinding[8].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[8].cbMaxLen = 4;
    acDBBinding[8].wType = DBTYPE_R4;

	acDBBinding[9].obValue = offsetof(UPDATECALLCHARGE,fCost3);
    acDBBinding[9].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[9].cbMaxLen = 4;
    acDBBinding[9].wType = DBTYPE_R4;

	acDBBinding[10].obValue = offsetof(UPDATECALLCHARGE,fUsedValue);
    acDBBinding[10].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[10].cbMaxLen = 4;
    acDBBinding[10].wType = DBTYPE_R4;


	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
		CloseDatabaseSession();
		return 0;   // failure...!!
    }
 
	hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(UPDATECALLCHARGE), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pUPDATECALLCHARGE;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
    
    
    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
			CloseDatabaseSession();
			return 1; //Sucess
	}
	
	
	return 0;
};


*/
BOOL CDatabaseSupport::SetPinLock(PINLOCK *pPINLOCK)
{
	HRESULT				hResult;
	const int			nParams = 2;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spPinLock(?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;


	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcPinNumber";           
    ParamBindInfo[1].ulParamSize = 11;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;
		
	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }



	acDBBinding[0].obValue = offsetof(PINLOCK,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(PINLOCK,chPinNumber );
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 11;
    acDBBinding[1].wType = DBTYPE_STR;

		
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(PINLOCK), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pPINLOCK;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
       

    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
  		
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
		 //Success
	
		CloseDatabaseSession();
		return 1;

	}

	return 0;
};

BOOL CDatabaseSupport::SetPinUnlock(PINUNLOCK *pPINUNLOCK)
{
	HRESULT				hResult;
	const int			nParams = 2;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spPinUnLock(?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;


	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcPinNumber";          
    ParamBindInfo[1].ulParamSize = 11;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;
		
	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }



	acDBBinding[0].obValue = offsetof(PINUNLOCK,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(PINUNLOCK,chPinNumber );
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 11;
    acDBBinding[1].wType = DBTYPE_STR;

		
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(PINUNLOCK), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pPINUNLOCK;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
       

    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
		 //Success
	
		CloseDatabaseSession();
		return 1;

	}

	return 0;
};

BOOL CDatabaseSupport::SetHelloCallLog(HELLOCALLLOG *pHELLOCALLLOG)
{
	HRESULT				hResult;
	const int			nParams = 25;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spHelloCallLog(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcSerialNo";           
    ParamBindInfo[1].ulParamSize = 17;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[2].pwszName = L"@vcPinNumber";      
    ParamBindInfo[2].ulParamSize = 17;					 
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[3].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[3].pwszName = L"@vcHelloANI";      
    ParamBindInfo[3].ulParamSize = 16;			
    ParamBindInfo[3].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[4].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[4].pwszName = L"@vcDNI";      
    ParamBindInfo[4].ulParamSize = 26;			
    ParamBindInfo[4].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[5].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[5].pwszName = L"@vcDate";      
    ParamBindInfo[5].ulParamSize = 51;					
    ParamBindInfo[5].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[6].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[6].pwszName = L"@vcTime";      
    ParamBindInfo[6].ulParamSize = 51;					
    ParamBindInfo[6].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[7].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[7].pwszName = L"@nUnits";      
    ParamBindInfo[7].ulParamSize = 4;					
    ParamBindInfo[7].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[8].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[8].pwszName = L"@nCallDuration";      
    ParamBindInfo[8].ulParamSize = 4;					
    ParamBindInfo[8].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[9].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[9].pwszName = L"@fCallCharge";      
    ParamBindInfo[9].ulParamSize = 4;					 
    ParamBindInfo[9].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[10].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[10].pwszName = L"@fActualCost";          
    ParamBindInfo[10].ulParamSize = 4;					
    ParamBindInfo[10].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[11].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[11].pwszName = L"@vcCallType";           
    ParamBindInfo[11].ulParamSize = 51;					
    ParamBindInfo[11].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[12].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[12].pwszName = L"@vcDestinationCode";           
    ParamBindInfo[12].ulParamSize = 11;					
    ParamBindInfo[12].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[13].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[13].pwszName = L"@nPlanID";      
    ParamBindInfo[13].ulParamSize = 4;					 
    ParamBindInfo[13].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[14].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[14].pwszName = L"@vcRateID";      
    ParamBindInfo[14].ulParamSize = 4;					 
    ParamBindInfo[14].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[15].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[15].pwszName = L"@vcSpCode";      
    ParamBindInfo[15].ulParamSize = 11;					 
    ParamBindInfo[15].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[16].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[16].pwszName = L"@nOutLine";      
    ParamBindInfo[16].ulParamSize = 2;					
    ParamBindInfo[16].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[17].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[17].pwszName = L"@vcIPAddress";      
    ParamBindInfo[17].ulParamSize = 51;					 
    ParamBindInfo[17].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[18].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[18].pwszName = L"@vcRoutingMethod";      
    ParamBindInfo[18].ulParamSize =26;					 
    ParamBindInfo[18].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[19].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[19].pwszName = L"@nBusinessCode";      
    ParamBindInfo[19].ulParamSize =4;					 
    ParamBindInfo[19].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[20].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[20].pwszName = L"@vcBusinessDescription";      
    ParamBindInfo[20].ulParamSize = 51;					 
    ParamBindInfo[20].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[21].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[21].pwszName = L"@nCustomerType";      
    ParamBindInfo[21].ulParamSize = 4;					 
    ParamBindInfo[21].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[22].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[22].pwszName = L"@vcCallStatus";      
    ParamBindInfo[22].ulParamSize = 26;					 
    ParamBindInfo[22].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[23].pwszDataSourceType = L"DBTYPE_I4";
    ParamBindInfo[23].pwszName = L"@nAvailableTime";      
    ParamBindInfo[23].ulParamSize = 8;					 
    ParamBindInfo[23].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[24].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[24].pwszName = L"@fUnitCharge";      
    ParamBindInfo[24].ulParamSize = 4;					 
    ParamBindInfo[24].dwFlags = DBPARAMFLAGS_ISINPUT;


	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }


	acDBBinding[0].obValue = offsetof(HELLOCALLLOG,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(HELLOCALLLOG,chSerialNo);
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 17;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(HELLOCALLLOG,chPinNumber);
    acDBBinding[2].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[2].cbMaxLen = 17;
    acDBBinding[2].wType = DBTYPE_STR;

	acDBBinding[3].obValue = offsetof(HELLOCALLLOG,chHelloANI);
    acDBBinding[3].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[3].cbMaxLen = 16;
    acDBBinding[3].wType = DBTYPE_STR;

	acDBBinding[4].obValue = offsetof(HELLOCALLLOG,chDNI);
    acDBBinding[4].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[4].cbMaxLen = 51;
    acDBBinding[4].wType = DBTYPE_STR;

	acDBBinding[5].obValue = offsetof(HELLOCALLLOG,chDate);
    acDBBinding[5].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[5].cbMaxLen = 51;
    acDBBinding[5].wType = DBTYPE_STR;

	acDBBinding[6].obValue = offsetof(HELLOCALLLOG,chTime);
    acDBBinding[6].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[6].cbMaxLen = 51;
    acDBBinding[6].wType = DBTYPE_STR;

	acDBBinding[7].obValue = offsetof(HELLOCALLLOG,nUnits);
    acDBBinding[7].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[7].cbMaxLen = 4;
    acDBBinding[7].wType = DBTYPE_I4;

	acDBBinding[8].obValue = offsetof(HELLOCALLLOG,nCallDuration);
    acDBBinding[8].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[8].cbMaxLen = 4;
    acDBBinding[8].wType = DBTYPE_I4;

	acDBBinding[9].obValue = offsetof(HELLOCALLLOG, fCallCharge);
    acDBBinding[9].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[9].cbMaxLen = 4;
    acDBBinding[9].wType = DBTYPE_R4;

	acDBBinding[10].obValue = offsetof(HELLOCALLLOG,fActualCost);
    acDBBinding[10].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[10].cbMaxLen = 4;
    acDBBinding[10].wType = DBTYPE_R4;

	acDBBinding[11].obValue = offsetof(HELLOCALLLOG,chCallType);
    acDBBinding[11].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[11].cbMaxLen = 51;
    acDBBinding[11].wType = DBTYPE_STR;

	acDBBinding[12].obValue = offsetof(HELLOCALLLOG,chDestinationCode);
    acDBBinding[12].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[12].cbMaxLen = 11;
    acDBBinding[12].wType = DBTYPE_STR;

	acDBBinding[13].obValue = offsetof(HELLOCALLLOG,nPlanID);
    acDBBinding[13].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[13].cbMaxLen = 4;
    acDBBinding[13].wType = DBTYPE_I4;

	acDBBinding[14].obValue = offsetof(HELLOCALLLOG,chRateID);
    acDBBinding[14].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[14].cbMaxLen = 16;
    acDBBinding[14].wType = DBTYPE_I4;

	acDBBinding[15].obValue = offsetof(HELLOCALLLOG,chServiceProviderCode);
    acDBBinding[15].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[15].cbMaxLen = 11;
    acDBBinding[15].wType = DBTYPE_STR;

	acDBBinding[16].obValue = offsetof(HELLOCALLLOG,nOutLine);
    acDBBinding[16].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[16].cbMaxLen = 4;
    acDBBinding[16].wType = DBTYPE_I4;

	acDBBinding[17].obValue = offsetof(HELLOCALLLOG,chIPAddress);
    acDBBinding[17].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[17].cbMaxLen = 51;
    acDBBinding[17].wType = DBTYPE_STR;

	acDBBinding[18].obValue = offsetof(HELLOCALLLOG,chRoutingMethod);
    acDBBinding[18].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[18].cbMaxLen = 26;
    acDBBinding[18].wType = DBTYPE_STR;

	acDBBinding[19].obValue = offsetof(HELLOCALLLOG,nBusinessCode);
    acDBBinding[19].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[19].cbMaxLen = 4;
    acDBBinding[19].wType = DBTYPE_I4;

	acDBBinding[20].obValue = offsetof(HELLOCALLLOG,chBusinessDescription);
    acDBBinding[20].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[20].cbMaxLen = 51;
    acDBBinding[20].wType = DBTYPE_STR;

	acDBBinding[21].obValue = offsetof(HELLOCALLLOG,nCustomerType);
    acDBBinding[21].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[21].cbMaxLen = 4;
    acDBBinding[21].wType = DBTYPE_I4;

	acDBBinding[22].obValue = offsetof(HELLOCALLLOG,chCallStatus);
    acDBBinding[22].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[22].cbMaxLen = 26;
    acDBBinding[22].wType = DBTYPE_STR;

	acDBBinding[23].obValue = offsetof(HELLOCALLLOG,nAvailableTime);
    acDBBinding[23].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[23].cbMaxLen = 4;
    acDBBinding[23].wType = DBTYPE_I4;

	acDBBinding[24].obValue = offsetof(HELLOCALLLOG,fUnitCharge);
    acDBBinding[24].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[24].cbMaxLen = 4;
    acDBBinding[24].wType = DBTYPE_R4;

		
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(PINCALLLOG), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pHELLOCALLLOG;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
    
    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
		 //Success
		CloseDatabaseSession();

		return 1;
	}

	return 0;
};

BOOL CDatabaseSupport::SetTN110CallLog(HELLOCALLLOG *pHELLOCALLLOG)
{
	HRESULT				hResult;
	const int			nParams = 25;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }

	wsprintfW(wCmdString,L"{?=call hello.spTN110CallLog(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcSerialNo";           
    ParamBindInfo[1].ulParamSize = 17;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[2].pwszName = L"@vcPinNumber";      
    ParamBindInfo[2].ulParamSize = 17;					 
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[3].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[3].pwszName = L"@vcHelloANI";      
    ParamBindInfo[3].ulParamSize = 16;			
    ParamBindInfo[3].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[4].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[4].pwszName = L"@vcDNI";      
    ParamBindInfo[4].ulParamSize = 26;			
    ParamBindInfo[4].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[5].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[5].pwszName = L"@vcDate";      
    ParamBindInfo[5].ulParamSize = 51;					
    ParamBindInfo[5].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[6].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[6].pwszName = L"@vcTime";      
    ParamBindInfo[6].ulParamSize = 51;					
    ParamBindInfo[6].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[7].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[7].pwszName = L"@nUnits";      
    ParamBindInfo[7].ulParamSize = 4;					
    ParamBindInfo[7].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[8].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[8].pwszName = L"@nCallDuration";      
    ParamBindInfo[8].ulParamSize = 4;					
    ParamBindInfo[8].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[9].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[9].pwszName = L"@fCallCharge";      
    ParamBindInfo[9].ulParamSize = 4;					 
    ParamBindInfo[9].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[10].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[10].pwszName = L"@fActualCost";          
    ParamBindInfo[10].ulParamSize = 4;					
    ParamBindInfo[10].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[11].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[11].pwszName = L"@vcCallType";           
    ParamBindInfo[11].ulParamSize = 51;					
    ParamBindInfo[11].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[12].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[12].pwszName = L"@vcDestinationCode";           
    ParamBindInfo[12].ulParamSize = 11;					
    ParamBindInfo[12].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[13].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[13].pwszName = L"@nPlanID";      
    ParamBindInfo[13].ulParamSize = 4;					 
    ParamBindInfo[13].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[14].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[14].pwszName = L"@vcRateID";      
    ParamBindInfo[14].ulParamSize = 4;					 
    ParamBindInfo[14].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[15].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[15].pwszName = L"@vcSpCode";      
    ParamBindInfo[15].ulParamSize = 11;					 
    ParamBindInfo[15].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[16].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[16].pwszName = L"@nOutLine";      
    ParamBindInfo[16].ulParamSize = 2;					
    ParamBindInfo[16].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[17].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[17].pwszName = L"@vcIPAddress";      
    ParamBindInfo[17].ulParamSize = 51;					 
    ParamBindInfo[17].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[18].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[18].pwszName = L"@vcRoutingMethod";      
    ParamBindInfo[18].ulParamSize =26;					 
    ParamBindInfo[18].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[19].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[19].pwszName = L"@nBusinessCode";      
    ParamBindInfo[19].ulParamSize =4;					 
    ParamBindInfo[19].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[20].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[20].pwszName = L"@vcBusinessDescription";      
    ParamBindInfo[20].ulParamSize = 51;					 
    ParamBindInfo[20].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[21].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[21].pwszName = L"@nCustomerType";      
    ParamBindInfo[21].ulParamSize = 4;					 
    ParamBindInfo[21].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[22].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[22].pwszName = L"@vcCallStatus";      
    ParamBindInfo[22].ulParamSize = 26;					 
    ParamBindInfo[22].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[23].pwszDataSourceType = L"DBTYPE_I4";
    ParamBindInfo[23].pwszName = L"@nAvailableTime";      
    ParamBindInfo[23].ulParamSize = 8;					 
    ParamBindInfo[23].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[24].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[24].pwszName = L"@fUnitCharge";      
    ParamBindInfo[24].ulParamSize = 4;					 
    ParamBindInfo[24].dwFlags = DBPARAMFLAGS_ISINPUT;


	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }


	acDBBinding[0].obValue = offsetof(HELLOCALLLOG,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(HELLOCALLLOG,chSerialNo);
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 17;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(HELLOCALLLOG,chPinNumber);
    acDBBinding[2].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[2].cbMaxLen = 17;
    acDBBinding[2].wType = DBTYPE_STR;

	acDBBinding[3].obValue = offsetof(HELLOCALLLOG,chHelloANI);
    acDBBinding[3].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[3].cbMaxLen = 16;
    acDBBinding[3].wType = DBTYPE_STR;

	acDBBinding[4].obValue = offsetof(HELLOCALLLOG,chDNI);
    acDBBinding[4].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[4].cbMaxLen = 51;
    acDBBinding[4].wType = DBTYPE_STR;

	acDBBinding[5].obValue = offsetof(HELLOCALLLOG,chDate);
    acDBBinding[5].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[5].cbMaxLen = 51;
    acDBBinding[5].wType = DBTYPE_STR;

	acDBBinding[6].obValue = offsetof(HELLOCALLLOG,chTime);
    acDBBinding[6].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[6].cbMaxLen = 51;
    acDBBinding[6].wType = DBTYPE_STR;

	acDBBinding[7].obValue = offsetof(HELLOCALLLOG,nUnits);
    acDBBinding[7].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[7].cbMaxLen = 4;
    acDBBinding[7].wType = DBTYPE_I4;

	acDBBinding[8].obValue = offsetof(HELLOCALLLOG,nCallDuration);
    acDBBinding[8].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[8].cbMaxLen = 4;
    acDBBinding[8].wType = DBTYPE_I4;

	acDBBinding[9].obValue = offsetof(HELLOCALLLOG, fCallCharge);
    acDBBinding[9].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[9].cbMaxLen = 4;
    acDBBinding[9].wType = DBTYPE_R4;

	acDBBinding[10].obValue = offsetof(HELLOCALLLOG,fActualCost);
    acDBBinding[10].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[10].cbMaxLen = 4;
    acDBBinding[10].wType = DBTYPE_R4;

	acDBBinding[11].obValue = offsetof(HELLOCALLLOG,chCallType);
    acDBBinding[11].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[11].cbMaxLen = 51;
    acDBBinding[11].wType = DBTYPE_STR;

	acDBBinding[12].obValue = offsetof(HELLOCALLLOG,chDestinationCode);
    acDBBinding[12].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[12].cbMaxLen = 11;
    acDBBinding[12].wType = DBTYPE_STR;

	acDBBinding[13].obValue = offsetof(HELLOCALLLOG,nPlanID);
    acDBBinding[13].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[13].cbMaxLen = 4;
    acDBBinding[13].wType = DBTYPE_I4;

	acDBBinding[14].obValue = offsetof(HELLOCALLLOG,chRateID);
    acDBBinding[14].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[14].cbMaxLen = 16;
    acDBBinding[14].wType = DBTYPE_I4;

	acDBBinding[15].obValue = offsetof(HELLOCALLLOG,chServiceProviderCode);
    acDBBinding[15].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[15].cbMaxLen = 11;
    acDBBinding[15].wType = DBTYPE_STR;

	acDBBinding[16].obValue = offsetof(HELLOCALLLOG,nOutLine);
    acDBBinding[16].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[16].cbMaxLen = 4;
    acDBBinding[16].wType = DBTYPE_I4;

	acDBBinding[17].obValue = offsetof(HELLOCALLLOG,chIPAddress);
    acDBBinding[17].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[17].cbMaxLen = 51;
    acDBBinding[17].wType = DBTYPE_STR;

	acDBBinding[18].obValue = offsetof(HELLOCALLLOG,chRoutingMethod);
    acDBBinding[18].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[18].cbMaxLen = 26;
    acDBBinding[18].wType = DBTYPE_STR;

	acDBBinding[19].obValue = offsetof(HELLOCALLLOG,nBusinessCode);
    acDBBinding[19].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[19].cbMaxLen = 4;
    acDBBinding[19].wType = DBTYPE_I4;

	acDBBinding[20].obValue = offsetof(HELLOCALLLOG,chBusinessDescription);
    acDBBinding[20].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[20].cbMaxLen = 51;
    acDBBinding[20].wType = DBTYPE_STR;

	acDBBinding[21].obValue = offsetof(HELLOCALLLOG,nCustomerType);
    acDBBinding[21].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[21].cbMaxLen = 4;
    acDBBinding[21].wType = DBTYPE_I4;

	acDBBinding[22].obValue = offsetof(HELLOCALLLOG,chCallStatus);
    acDBBinding[22].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[22].cbMaxLen = 26;
    acDBBinding[22].wType = DBTYPE_STR;

	acDBBinding[23].obValue = offsetof(HELLOCALLLOG,nAvailableTime);
    acDBBinding[23].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[23].cbMaxLen = 4;
    acDBBinding[23].wType = DBTYPE_I4;

	acDBBinding[24].obValue = offsetof(HELLOCALLLOG,fUnitCharge);
    acDBBinding[24].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[24].cbMaxLen = 4;
    acDBBinding[24].wType = DBTYPE_R4;

		
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(PINCALLLOG), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pHELLOCALLLOG;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
    
    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
		 //Success
		CloseDatabaseSession();

		return 1;
	}

	return 0;
};


BOOL CDatabaseSupport::SetPinCallLog(PINCALLLOG *pPINCALLLOG)
{
	HRESULT				hResult;
	const int			nParams = 25;			
    WCHAR				wCmdString[128] = {0};  
	DBPARAMBINDINFO     ParamBindInfo[nParams]; 
    ULONG               ParamOrdinals[nParams]; 
	DBBINDING           acDBBinding[nParams];   
    DBBINDSTATUS        acDBBindStatus[nParams];
	DBPARAMS            Params;			
	LONG                lNumRows = 0;   
	int					i;


	// Create and initialize database connection
    hResult = CreateDatabaseSession();
    if(FAILED(hResult))
    {
        
		// can't connect to database for some reason...
        return 0;
    }


	wsprintfW(wCmdString,L"{?=call hello.spPinCallLog(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}");


	// Initialize the stored procedure parameter structure
    
	// Create a CommandText object.
    hResult = m_pIDBCreateCommand->CreateCommand(NULL, 
                                          IID_ICommandText, 
                                          (IUnknown**) &m_pICommandText);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Set the command text.
    hResult = m_pICommandText->SetCommandText(DBGUID_DBSQL, wCmdString);
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	// Initialize the parameter bind info structure array
    AZERO(ParamBindInfo);

    // now, populate with exactly what we want
    for(i=0;i<nParams;i++)
    {
        ParamOrdinals[i] = i+1;
        ParamBindInfo[i].bPrecision = 0;
        ParamBindInfo[i].bScale = 0;
        ParamBindInfo[i].bPrecision = 11;
    }


	ParamBindInfo[0].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[0].pwszName = L"ReturnVal";           
    ParamBindInfo[0].ulParamSize = 4;					
    ParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISOUTPUT;

	ParamBindInfo[1].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[1].pwszName = L"@vcSerialNo";           
    ParamBindInfo[1].ulParamSize = 21;					
    ParamBindInfo[1].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[2].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[2].pwszName = L"@vcPinNumber";      
    ParamBindInfo[2].ulParamSize = 11;					 
    ParamBindInfo[2].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[3].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[3].pwszName = L"@vcANI";      
    ParamBindInfo[3].ulParamSize = 16;			
    ParamBindInfo[3].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[4].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[4].pwszName = L"@vcDNI";      
    ParamBindInfo[4].ulParamSize = 26;			
    ParamBindInfo[4].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[5].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[5].pwszName = L"@vcDate";      
    ParamBindInfo[5].ulParamSize = 51;					
    ParamBindInfo[5].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[6].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[6].pwszName = L"@vcTime";      
    ParamBindInfo[6].ulParamSize = 51;					
    ParamBindInfo[6].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[7].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[7].pwszName = L"@nUnits";      
    ParamBindInfo[7].ulParamSize = 4;					
    ParamBindInfo[7].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[8].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[8].pwszName = L"@nCallDuration";      
    ParamBindInfo[8].ulParamSize = 4;					
    ParamBindInfo[8].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[9].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[9].pwszName = L"@fCallCharge";      
    ParamBindInfo[9].ulParamSize = 4;					 
    ParamBindInfo[9].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[10].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[10].pwszName = L"@fActualCost";          
    ParamBindInfo[10].ulParamSize = 4;					
    ParamBindInfo[10].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[11].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[11].pwszName = L"@vcCallType";           
    ParamBindInfo[11].ulParamSize = 51;					
    ParamBindInfo[11].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[12].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[12].pwszName = L"@vcDestinationCode";           
    ParamBindInfo[12].ulParamSize = 11;					
    ParamBindInfo[12].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[13].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[13].pwszName = L"@nPlanID";      
    ParamBindInfo[13].ulParamSize = 4;					 
    ParamBindInfo[13].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[14].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[14].pwszName = L"@vcRateID";      
    ParamBindInfo[14].ulParamSize = 4;					 
    ParamBindInfo[14].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[15].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[15].pwszName = L"@vcSpCode";      
    ParamBindInfo[15].ulParamSize = 11;					 
    ParamBindInfo[15].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[16].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[16].pwszName = L"@nOutLine";      
    ParamBindInfo[16].ulParamSize = 2;					
    ParamBindInfo[16].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[17].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[17].pwszName = L"@vcIPAddress";      
    ParamBindInfo[17].ulParamSize = 51;					 
    ParamBindInfo[17].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[18].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[18].pwszName = L"@vcRoutingMethod";      
    ParamBindInfo[18].ulParamSize =26;					 
    ParamBindInfo[18].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[19].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[19].pwszName = L"@nBusinessCode";      
    ParamBindInfo[19].ulParamSize =4;					 
    ParamBindInfo[19].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[20].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[20].pwszName = L"@vcBusinessDescription";      
    ParamBindInfo[20].ulParamSize = 51;					 
    ParamBindInfo[20].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[21].pwszDataSourceType = L"DBTYPE_I2";
    ParamBindInfo[21].pwszName = L"@nCustomerType";      
    ParamBindInfo[21].ulParamSize = 4;					 
    ParamBindInfo[21].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[22].pwszDataSourceType = L"DBTYPE_VARCHAR";
    ParamBindInfo[22].pwszName = L"@vcCallStatus";      
    ParamBindInfo[22].ulParamSize = 26;					 
    ParamBindInfo[22].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[23].pwszDataSourceType = L"DBTYPE_I4";
    ParamBindInfo[23].pwszName = L"@nAvailableTime";      
    ParamBindInfo[23].ulParamSize = 8;					 
    ParamBindInfo[23].dwFlags = DBPARAMFLAGS_ISINPUT;

	ParamBindInfo[24].pwszDataSourceType = L"DBTYPE_R4";
    ParamBindInfo[24].pwszName = L"@fUnitCharge";      
    ParamBindInfo[24].ulParamSize = 4;					 
    ParamBindInfo[24].dwFlags = DBPARAMFLAGS_ISINPUT;


	// Set the parameter information
    hResult = m_pICommandText->QueryInterface(IID_ICommandWithParameters,
                                       (void**)&m_pICommandWithParams);
	
    if(FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }
    
    hResult = m_pICommandWithParams->SetParameterInfo(nParams, 
                                               ParamOrdinals, 
                                               ParamBindInfo);
	
    if(FAILED(hResult))
    {
    		
        CloseDatabaseSession();
		return 0;   // failure...!!
		
    }

	
    for(i = 0; i < nParams; i++)
    {
        acDBBinding[i].iOrdinal = i+1;
        
        acDBBinding[i].obLength = 0;
        acDBBinding[i].obStatus = 0;
        acDBBinding[i].pTypeInfo = NULL;
        acDBBinding[i].pObject = NULL;
        acDBBinding[i].pBindExt = NULL;
        acDBBinding[i].dwPart = DBPART_VALUE;
        acDBBinding[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        acDBBinding[i].dwFlags = 0;
        acDBBinding[i].bScale = 0;
        acDBBinding[i].bPrecision = 11;
    }


	acDBBinding[0].obValue = offsetof(PINCALLLOG,nResult);
    acDBBinding[0].eParamIO = DBPARAMIO_OUTPUT;
    acDBBinding[0].cbMaxLen = 4;
    acDBBinding[0].wType = DBTYPE_I4;

	acDBBinding[1].obValue = offsetof(PINCALLLOG,chSerialNo);
    acDBBinding[1].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[1].cbMaxLen = 21;
    acDBBinding[1].wType = DBTYPE_STR;

	acDBBinding[2].obValue = offsetof(PINCALLLOG,chPinNumber);
    acDBBinding[2].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[2].cbMaxLen = 11;
    acDBBinding[2].wType = DBTYPE_STR;

	acDBBinding[3].obValue = offsetof(PINCALLLOG,chANI);
    acDBBinding[3].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[3].cbMaxLen = 26;
    acDBBinding[3].wType = DBTYPE_STR;

	acDBBinding[4].obValue = offsetof(PINCALLLOG,chDNI);
    acDBBinding[4].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[4].cbMaxLen = 51;
    acDBBinding[4].wType = DBTYPE_STR;

	acDBBinding[5].obValue = offsetof(PINCALLLOG,chDate);
    acDBBinding[5].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[5].cbMaxLen = 51;
    acDBBinding[5].wType = DBTYPE_STR;

	acDBBinding[6].obValue = offsetof(PINCALLLOG,chTime);
    acDBBinding[6].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[6].cbMaxLen = 51;
    acDBBinding[6].wType = DBTYPE_STR;

	acDBBinding[7].obValue = offsetof(PINCALLLOG,nUnits);
    acDBBinding[7].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[7].cbMaxLen = 4;
    acDBBinding[7].wType = DBTYPE_I4;

	acDBBinding[8].obValue = offsetof(PINCALLLOG,nCallDuration);
    acDBBinding[8].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[8].cbMaxLen = 4;
    acDBBinding[8].wType = DBTYPE_I4;

	acDBBinding[9].obValue = offsetof(PINCALLLOG, fCallCharge);
    acDBBinding[9].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[9].cbMaxLen = 4;
    acDBBinding[9].wType = DBTYPE_R4;

	acDBBinding[10].obValue = offsetof(PINCALLLOG,fActualCost);
    acDBBinding[10].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[10].cbMaxLen = 4;
    acDBBinding[10].wType = DBTYPE_R4;

	acDBBinding[11].obValue = offsetof(PINCALLLOG,chCallType);
    acDBBinding[11].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[11].cbMaxLen = 51;
    acDBBinding[11].wType = DBTYPE_STR;

	acDBBinding[12].obValue = offsetof(PINCALLLOG,chDestinationCode);
    acDBBinding[12].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[12].cbMaxLen = 11;
    acDBBinding[12].wType = DBTYPE_STR;

	acDBBinding[13].obValue = offsetof(PINCALLLOG,nPlanID);
    acDBBinding[13].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[13].cbMaxLen = 4;
    acDBBinding[13].wType = DBTYPE_I4;

	acDBBinding[14].obValue = offsetof(PINCALLLOG,chRateID);
    acDBBinding[14].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[14].cbMaxLen = 16;
    acDBBinding[14].wType = DBTYPE_I4;

	acDBBinding[15].obValue = offsetof(PINCALLLOG,chSpCode);
    acDBBinding[15].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[15].cbMaxLen = 11;
    acDBBinding[15].wType = DBTYPE_STR;

	acDBBinding[16].obValue = offsetof(PINCALLLOG,nOutLine);
    acDBBinding[16].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[16].cbMaxLen = 4;
    acDBBinding[16].wType = DBTYPE_I4;

	acDBBinding[17].obValue = offsetof(PINCALLLOG,chIPAddress);
    acDBBinding[17].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[17].cbMaxLen = 51;
    acDBBinding[17].wType = DBTYPE_STR;

	acDBBinding[18].obValue = offsetof(PINCALLLOG,chRoutingMethod);
    acDBBinding[18].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[18].cbMaxLen = 26;
    acDBBinding[18].wType = DBTYPE_STR;

	acDBBinding[19].obValue = offsetof(PINCALLLOG,nBusinessCode);
    acDBBinding[19].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[19].cbMaxLen = 4;
    acDBBinding[19].wType = DBTYPE_I4;

	acDBBinding[20].obValue = offsetof(PINCALLLOG,chBusinessDescription);
    acDBBinding[20].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[20].cbMaxLen = 51;
    acDBBinding[20].wType = DBTYPE_STR;

	acDBBinding[21].obValue = offsetof(PINCALLLOG,nCustomerType);
    acDBBinding[21].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[21].cbMaxLen = 4;
    acDBBinding[21].wType = DBTYPE_I4;

	acDBBinding[22].obValue = offsetof(PINCALLLOG,chCallStatus);
    acDBBinding[22].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[22].cbMaxLen = 26;
    acDBBinding[22].wType = DBTYPE_STR;

	acDBBinding[23].obValue = offsetof(PINCALLLOG,nAvailableTime);
    acDBBinding[23].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[23].cbMaxLen = 4;
    acDBBinding[23].wType = DBTYPE_I4;

	acDBBinding[24].obValue = offsetof(PINCALLLOG,fUnitCharge);
    acDBBinding[24].eParamIO = DBPARAMIO_INPUT;
    acDBBinding[24].cbMaxLen = 4;
    acDBBinding[24].wType = DBTYPE_R4;

		
	// Create an accessor from the above set of bindings.
    hResult = m_pICommandWithParams->QueryInterface(IID_IAccessor, 
                                             (void**)&m_pIAccessor);
    if (FAILED(hResult))
    {
       
		CloseDatabaseSession();
		return 0;   // failure...!!

		
    }
 
	  hResult = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
                                    nParams, 
                                    acDBBinding, 
                                    sizeof(PINCALLLOG), 
                                    &m_hAccessor,
                                    acDBBindStatus);
    if (FAILED(hResult))
    {
        
		
        CloseDatabaseSession();
		return 0;  // failure...!!
		
    }

	// Initialize the structure first
    SZERO(Params);

    Params.pData = pPINCALLLOG;
    Params.cParamSets = 1;
    Params.hAccessor = m_hAccessor;
    
    // Execute the command
    
    hResult = m_pICommandText->Execute(NULL, 
                                IID_IRowset, 
                                &Params, 
                                &lNumRows, 
                                (IUnknown **) &m_pIRowset);
    if(FAILED(hResult))
    {
        
		CloseDatabaseSession();
		return 0;  // failure...!!
		
    }
	else
	{
		 //Success
		CloseDatabaseSession();

		return 1;
	}

	return 0;
};


