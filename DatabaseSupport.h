/******************************************************************************************/
/*                                                                                        */
/*	Source File		: DataBaseSupport.h													  */
/*																						  */	
/*	Version			: 2.0																  */
/*                                                                                        */
/*	Target OS		: Windows 2000	Service Pack 3.0 MDAC 2.6							  */
/*																						  */
/*	Description		: Database Support Prototypes Definitions							  */
/*																						  */
/*	Last Revision	: <05/12/2002>													      */
/*																						  */
/*	Release Stage	: BETA																  */
/*																						  */
/*	Author			: A.T.Lloyd	(c) 2002,  All rights reserved.							  */
/*					  Tritel Technologies (Pvt.) Ltd.									  */
/*  Revisions		: 
/*		08/15/008	Includes updated	mihindu	
/******************************************************************************************/
#pragma once

#define AZERO(array) do{memset(array,0,sizeof(array));}while(0)
#define SZERO(strvct) do{memset(&strvct,0,sizeof(strvct));}while(0)

namespace Tritel
{

/*typedef*/ struct CUSTOMERTYPE//tagCUSTOMERTYPE
{
	char	chCallerID[16];
	char	chAreaCode[6];
	char	chLanguage[21];
	int		nPlanID;
	float	fCurrentBalance;
	int		nBusinessCode;
	int		nCustomerType;
	int		nResult;
	

};

/*
typedef struct tagPINCLI
{
	
	char chCallerID[16];
	char chPinNumber[11];
	int  nResult;


}PINCLI;

typedef struct tagDIGITS
{

	char	chPrefixDestNo[6];
	char	chAreaCode[6];
	int		nDestinationDigits;
	int		nResult;
}DIGITS;
*/

struct CALLRATE
{
	char	chAuthorizationKey[16];
	char	chDestinationNumber[26];
	int		nBusinessCode;
	int		nPlanID;
	char	chAreaCode[6];
	char	chIPAddress[51];
	int		nBlock1;
	int		nBlock2;
	int		nBlock3;
	float	fCost1;
	float	fCost2;
	float	fCost3;
	int		nAvailableTime;
	char	chDestinationCode[16];
	char	chCallType[51];
	int		nRateID;
	char	chDialString[26];
	char	chSpCode[16];
	char	chRoutingMethod[16];
	int		nOStart;
	int		nOEnd;
	float	fAUCharge;
	int		nResult;
	

};

/*typedef*/ struct COMCALLLOG//tagCOMCALLLOG
{
	char	chCallerID[16];
	char	chDNI[26];	
	char	chDate[51];
	char	chTime[51];
	int		nCallDuration;
	int		nUnits;
	float	fCallCharge;
	float	fActualCost;
	char	chCallType[51];
	char	chDestinationCode[11];
	int		nPlanID;
	int		nRateID;
	char	chSpCode[11];
	int		nOutgoingLineNumber;
	char	chIPAddress[51];
	char	chCallStatus[26];
	char	chRoutingMethod[26];
	int		nBusinessCode;
	float	fUnitCharge;
	int		nResult;

}; 


/*typedef*/ struct COMTOTALBUSINESS //tagCOMTOTALBUSINESS
{
	char	chCallerID[16];
	char	chPassWord[6];
	float	fTotalAmount;
	int		nResult;

};


/*typedef*/ struct COMRECHARGE //tagCOMRECHARGE
{
	char	chCallerID[16];
	int		nBusinessCode;
	char	chPinNumber[11];
	float	fCurrentValue;
	int		nResult;

};

/*
typedef struct tagCOMCALLHISTORY
{
	char	chCallerID[15];
	char	chPassWord[4];
	int		nResult;
	
}COMCALLHISTORY;
*/

/*typedef*/ struct UPDATECALLCHARGE//tagUPDATECALLCHARGE
{
	char	chAuthorizationKey[16];
	int		nNumberOfSecons;
	int		nBusinessCode;
	int		nBlock1;
	int		nBlock2;
	int		nBlock3;
	float	fCost1;
	float	fCost2;
	float	fCost3;
	float	fUsedValue;
	int		nResult;
	
};
/*
typedef struct tagPINAUTHENTICATE
{
	char chPinNumber[11];
	int nCustomerType;
	char chBusinessDescription[51];
	char chLanguage[11];
	char chLangCodeStatus[8];
	float fCurrentBalance;
	int nBusinessCode;
	int nPlanID;
	char chSerialNo[21];
	char chWelcome[51];
	int nResult;


}PINAUTHENTICATE;

typedef struct tagPINLOCK
{
	char	chPinNumber[11];
	int		nResult;

}PINLOCK;

typedef struct tagPINUNLOCK
{
	char	chPinNumber[11];
	int		nResult;

}PINUNLOCK;

typedef struct tagTest
{
	int nResult;
	char Name[11];
}TESTSTUCT;

typedef struct tagPINCALLLOG
{
	char	chSerialNo[21];
	char	chPinNumber[11];
	char	chANI[16];
	char	chDNI[26];
	char	chDate[51];
	char	chTime[51];
	int		nUnits;
	int		nCallDuration;
	float	fCallCharge;
	float	fActualCost;
	char	chCallType[51];
	char	chDestinationCode[11];
	int		nPlanID;
	int		chRateID;
	char	chSpCode[11];
	int		nOutLine;
	char	chIPAddress[51];
	char	chRoutingMethod[26];
	int		nBusinessCode;
	char	chBusinessDescription[51];
	int		nCustomerType;
	char	chCallStatus[26];
	int		nAvailableTime;
	float	fUnitCharge;
	int		nResult;

}PINCALLLOG;

typedef struct tagHELLOCALLLOG
{
	char	chSerialNo[21];
	char	chPinNumber[11];
	char	chHelloANI[16];
	char	chDNI[26];
	char	chDate[51];
	char	chTime[51];
	int		nUnits;
	int		nCallDuration;
	float	fCallCharge;
	float	fActualCost;
	char	chCallType[51];
	char	chDestinationCode[11];
	int		nPlanID;
	int		chRateID;
	char	chServiceProviderCode[11];
	int		nOutLine;
	char	chIPAddress[51];
	char	chRoutingMethod[26];
	int		nBusinessCode;
	char	chBusinessDescription[51];
	int		nCustomerType;
	char	chCallStatus[26];
	int		nAvailableTime;
	float	fUnitCharge;
	int		nResult;

}HELLOCALLLOG;
*/
};