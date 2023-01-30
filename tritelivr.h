#pragma once
#include "answermachine.h"
#include "voiceheader.h"
#include "./databasesupport2.h"

/******************* CallFlow state definitions ***********/
#define START							0x2001 
#define DIALTONE						0x2002 
#define BUSYTONE						0x2003
#define GETDIGIT						0x2004
#define GETDIGIT2						0x2005
#define GETDIGIT3						0x2006
#define CALLHISTORY						0x2007
#define TOTALBUSINESS					0x2008
#define RECHARGE						0x2009
#define MAKECALL						0x2010
#define STOP							0x2011
#define CURRENTBALANCE					0x2012
#define AUTHENTICATE					0x2013
#define NEW_BALANCE						0x2014
#define WELCOMEPASSWORD					0x2015
#define GETPASSWORD						0x2016
#define PLAYTOTAL						0x2017
#define PLAYAND							0x2018
#define PLAYRUPEES						0x2019
#define PINNUMBER						0x2020
#define GETPIN							0x2021
#define LANGUAGE						0x2022
#define GETLANGUAGE						0x2023
#define GETOPTION						0x2024
#define PLAYDESTINATION					0x2025
#define GETDESTNO						0x2026
#define INVALIDPINNUMBER				0x2027
#define VALIDPINNUMBER					0x2028
#define INVALIDLANGUAGE					0x2029
#define INVALIDDESTINATION				0x2030
#define PLAYNEWTOTAL					0x2031
#define PLAYNEWAND						0x2032
#define PLAYNEWRUPEES					0x2033
#define INVALIDPASSWORD					0x2034
#define STARTCLI					    0x2035
#define PLAYBALANCE						0x2036
#define GETDN							0x2037
#define PLAYTIME						0x2038
#define	DIALOUT							0x2039
#define PLAYERRORTONES					0x2040
#define GOTPIN							0x2041
#define PROCESSDESTINATION				0x2042
#define ON_CALL							0x2045
#define CONNECT_FAILED					0x2046
#define	IVR_END							CONNECT_FAILED
#define THANKS							0x2047

//v.4
#define		TIMEAVIALABLE				0x3028
#define		PLAYTIMEVALUE				0x3029
#define		MINUTESAND					0x3030
#define		AVIALABLEMINUTES			0x3031
#define		AVIALABLESECONDS			0x3032
#define		TIMEREAMINS					0x3033
#define		CALLCHARGEPASSWORD			0x3036		
#define		CALLCHARGEGETPWD			0x3037
#define		CALLCHARGEAUTHENTICATION	0x3038
#define		PLAYDIGITS					0x3039
#define		PLAYCALLCHARGE				0x3040
#define		CALLCHARGEAND				0x3041
#define		CALLCHARGERUPEES			0x3042
#define		PLAYDURATION				0x3043
#define		CALLCHARGEMINUTESAND		0x3044
#define		CALLCHARGEMINUTES			0x3045
#define		CALLCHARGESECONDS			0x3046
#define		PLAYCOTINUOUS				0x3047
#define		PALYASTRICK					0x3048
#define		GETASTRICK					0x3049
#define		ANALYSEASTRIC				0x3050
#define		NYW							0x3060

namespace Tritel
{

class CTritelIVR :
	public CAnswerMachine
{
public:
	CTritelIVR(CLine& line);
	virtual ~CTritelIVR();

private:
	int Begin(CLine& line);

	void CommCallFlow(CLine& line);
	void RechargeCallFlow();
	void OnPlayFileTerminate(int reason);
	void OnGetDigitsTerminate(int reason,std::string digits);
	virtual void Reset();
	void ShowStatus(char* msg);
	int PlayMoneyValue(int flag);
	int RechargeAccount(std::string& pin);

	CUSTOMERTYPE CT;
	int nTry;
	float MoneyValue;

	int MakeOutCall(std::string& dni);
};

}
