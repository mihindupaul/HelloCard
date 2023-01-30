#pragma once
#include <string>
#include "tone.h"
#include "voiceheader.h"

class CConfiguration
{
public:
	//	language used by IVR
	enum Language
	{
		ENGLISH,
		SINHALA,
		TAMIL,
		COMMON
	};

	CConfiguration(void);
	virtual ~CConfiguration(void);
	std::string GetVoxFile(std::string file);
	static CConfiguration& Instance(void);
	static void Cleanup(void);
	std::string GetLangVoxFile(std::string file_name,CConfiguration::Language l);

private:
	static CConfiguration* m_pInstance;
	char voxpath[MAX_PATH];
	std::string m_ServerIP;
	void LoadSettings(void);

public:
	std::string& GetServerIP(void);
	int OutLineOffset(void);
};
