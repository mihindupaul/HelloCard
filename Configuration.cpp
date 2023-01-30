#include "stdafx.h"
#include <sstream>
#include ".\configuration.h"
#include <atlbase.h>

CConfiguration* CConfiguration::m_pInstance =  NULL;

CConfiguration::CConfiguration(void)
:m_ServerIP("127.0.0.1")
{
	LoadSettings();

	////	Configur Dropping Tone
	//DROP_TONE.AddTone(25,425,-10,0,-10,25);
	//DROP_TONE.AddTone(25,425,-10,0,-10,25);
	//DROP_TONE.Cycles(20);

	////	Configure Route Tone (progress tone)
	//PROGRESS_TONE.AddTone(5,425);
	//PROGRESS_TONE.AddTone(5,425);
	//PROGRESS_TONE.Cycles(20);
}

CConfiguration::~CConfiguration(void)
{
}

std::string CConfiguration::GetVoxFile(std::string file)
{
	char tmp_path[MAX_PATH];

	sprintf(tmp_path,"%s%s",voxpath,file.c_str());
	
	return std::string(tmp_path);
}

CConfiguration& CConfiguration::Instance(void)
{
	if(m_pInstance == NULL)
	{
		m_pInstance = new CConfiguration();
	}
	return *m_pInstance;
}

void CConfiguration::LoadSettings(void)
{
	ATL::CRegKey rk;
	TCHAR ip[MAX_PATH];
	ULONG len;

	if(rk.Open(HKEY_LOCAL_MACHINE,"Software\\Tritel\\Hello\\",KEY_READ) == ERROR_SUCCESS)
	{
		len = MAX_PATH;
		rk.QueryStringValue("Phrases",voxpath,&len);
		len = MAX_PATH;
		if(rk.QueryStringValue("IpAddress",ip,&len) == ERROR_SUCCESS)
		{
			m_ServerIP.assign(ip,len);
		}
	}
}

void CConfiguration::Cleanup(void)
{
	if(m_pInstance)
		delete m_pInstance;
}

std::string CConfiguration::GetLangVoxFile(std::string file_name,CConfiguration::Language l)
{
	std::stringstream ss;

	ss << voxpath << "\\";
	
	switch(l)
	{
	case ENGLISH:
		ss << "English";
		break;
	case TAMIL:
		ss << "Tamil";
		break;
	case SINHALA:
		ss << "Sinhala";
		break;
	}
	
	ss << file_name;

	return ss.str();
}

std::string& CConfiguration::GetServerIP(void)
{
	return m_ServerIP;
}

int CConfiguration::OutLineOffset(void)
{
	return 31;
}
