#include "stdafx.h"
#include <time.h>
#include <algorithm>
#include ".\ss7line.h"
#include "dm3cc_parm.h"

CSS7Line::CSS7Line(CLineManager* manager)
: CLine(manager)
{
	
}

CSS7Line::~CSS7Line(void)
{
}

int CSS7Line::MakeCall(std::string number,std::string caller_number)
{
	S7_MAKECALL_BLK s7_makecall;

	::memset(&s7_makecall,0,sizeof(S7_MAKECALL_BLK));

	//Data structure contains ISUP message parameters
	S7_IE_BLK		ie_blk;

	//Used to send an Information Element (IE) block in some technologies
	GC_IE_BLK gc_ie_blk;

	ie_blk.length=8;
	ie_blk.data[0]=0x31;
	ie_blk.data[1]=0x02;
	ie_blk.data[2]=0x00;
	ie_blk.data[3]=0x00;
	ie_blk.data[4]=0x39;
	ie_blk.data[5]=0x02;
	ie_blk.data[6]=0x31;
	ie_blk.data[7]=0xc0;

	gc_ie_blk.gclib=NULL;
	gc_ie_blk.cclib=&ie_blk;

	if(gc_SetInfoElem(ldev,&gc_ie_blk)!=GC_SUCCESS)
	{
		return 1;		
	}

	//	Enable per-call call analysis
	GC_PARM_BLKP gcParmBlk = NULL;

	int cpaType = GC_CA_ENABLE_ALL;
	gc_util_insert_parm_ref( &gcParmBlk,CCSET_CALLANALYSIS,	CCPARM_CA_MODE,	sizeof(int),&cpaType);

	int cpaSpeedValue = PAMD_ACCU;
	gc_util_insert_parm_ref( &gcParmBlk,CCSET_CALLANALYSIS,	CCPARM_CA_PAMDSPDVAL,sizeof(int),&cpaSpeedValue);

	gclib_makecall.ext_datap = gcParmBlk;

	//	TODO: replace using this
//	number.compare(0,2,"00");

	if(strncmp(number.c_str(), "00", 2) )
	{
		s7_makecall.ss7.destination_number_type=SS7_NATIONAL_NUMBER;
	}
	else
		s7_makecall.ss7.destination_number_type=SS7_INTERNATIONAL_NUMBER;


	s7_makecall.ss7.trans_medium_req=TMR_SPEECH;
	s7_makecall.ss7.destination_number_plan=SS7_ISDN_NUMB_PLAN;
	s7_makecall.ss7.internal_network_number=INN_ALLOWED;


	s7_makecall.ss7.origination_number_type=SS7_NATIONAL_NUMBER;

	s7_makecall.ss7.origination_number_plan=SS7_ISDN_NUMB_PLAN;
	s7_makecall.ss7.origination_present_restrict=PRESENTATION_ALLOWED;
	s7_makecall.ss7.origination_screening=SCREEN_NETWORK_PROVIDED;
	s7_makecall.ss7.calling_party_category=SS7_ORDINARY_SUBS_CATEGORY;

	s7_makecall.ss7.user_to_user_indicators = UUI_UUS1_REQ_NE ;
	s7_makecall.ss7.forward_call_indicators=FCI_CONNECTION_BOTH; //WAHT IS THIS0x6000;

	////TeSt

	s7_makecall.ss7.usrinfo_bufp = NULL;
	s7_makecall.ss7.satellite_indicator = SI_NOSATELLITES;
	s7_makecall.ss7.echo_device_indicator = EDI_ECHOCANCEL_NOTINCLUDED;
	s7_makecall.ss7.continuity_check_indicator = CCI_CC_NOTREQUIRED;

	strcpy(s7_makecall.ss7.origination_phone_number, caller_number.c_str());

	//	s7_makecall.ss7.user_to_user_indicators=0;

	/*if(gc_SetCallingNum(  ldev,chCallingNumber )!= GC_SUCCESS)
	{
	}*/

	crn=0;
	//	prepare make call blk
	makecallblk.cclib = &s7_makecall;
	makecallblk.gclib = &gclib_makecall;

	if (gc_MakeCall(ldev,&crn,const_cast<char*>(number.c_str()) , &makecallblk, 60, EV_ASYNC) != GC_SUCCESS)
	{
		//SL: Modified to show the reason for failure.
		GC_INFO	tmp;
		gc_ErrorInfo(&tmp);
		LOG4CPLUS_ERROR(LINE_LOGGER,"Make Call Failed" << tmp.gcMsg << tmp.gcValue << tmp.gcMsg);
		return 1;	
	}

	//	Delete the CPA parameter Blk
	gc_util_delete_parm_blk(gcParmBlk);
	
	return 0;
}

int CSS7Line::Open(int index,int board,int time_slot,int vb,int vch)
{
	//	voice problem handling
	GC_PARM_BLKP		parmblkp = NULL;
	char devname[160];
	GC_INFO	error_info;

	//	payment protection option
	//if(index > 310) return 0;

	this->channel = index;

	//	this is the magic
	sprintf(devname, "N_dtiB%dT%d:P_SS7:V_dxxxB%dC%d", board,time_slot,vb,vch);

	//	this is the original code
	//sprintf(devname, "N_dtiB%dT%d:P_SS7:V_dxxxB%dC%d", board,time_slot,board,ch); 

	m_sDeviceName = devname;
	
	LOG4CPLUS_INFO(LINE_LOGGER,	"Openning.."  << m_sDeviceName);

	//	user attribute is set to refer *this 
	if (gc_OpenEx(&ldev, devname, EV_SYNC,(void*)this) != GC_SUCCESS)
	{
		if(gc_ErrorInfo(&error_info) == GC_SUCCESS)
		{
			LOG4CPLUS_ERROR(LINE_LOGGER,error_info.gcMsg << error_info.ccMsg);
		}
		else
		{
			LOG4CPLUS_FATAL(LINE_LOGGER,"gc_OpenEx() Failed");
		}
		return -1;
	}



	//	This is for Playing voice messages
	METAEVENT me;
	OnOpened(me);

#ifdef _DEBUG
	SetTrace(true);
#endif

	return 0;
}

std::string CSS7Line::GetANI(void) const
{
	char ani[GC_ADDRSIZE];

	if( gc_GetCallInfo(crn,ORIGINATION_ADDRESS,ani) != GC_SUCCESS)
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,__FUNCTION__);
		return "";
	}
	return std::string(ani);
}

std::string CSS7Line::GetDNIS(void) const
{
	char dnis[GC_ADDRSIZE];

	if( gc_GetCallInfo(crn,DESTINATION_ADDRESS,dnis) != GC_SUCCESS)
	{
		LOG4CPLUS_ERROR(LINE_LOGGER,__FUNCTION__);
		return "";
	}
	return std::string(dnis);
}
