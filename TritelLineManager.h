#pragma once
#include <vector>
#include "line.h"
#include "phoneline.h"

namespace Tritel
{
	class CTritelLineManager :
		public CLineManager
	{
	public:
		CTritelLineManager(void);
		virtual ~CTritelLineManager(void);
	protected:
		int OnLineOpen(CLine& line);
	private:
		void LoadSettings(void);
		int AddToConnectorPool(CLine& line);
		int AddToIVRPool(CLine& line);

		std::vector<CAnswerMachine*>	m_vecIVR2;
		std::vector<CCallConnector*>	m_vecConnector2;
		DWORD m_ss7_in_start;
		DWORD m_ss7_in_end;
		DWORD m_ss7_out_start;
		DWORD m_ss7_out_end;
	public:
		int PickLineConnector(CCallConnector** ppOutLine, int Start, int End);
	};
}