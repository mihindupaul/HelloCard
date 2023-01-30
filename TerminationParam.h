#pragma once
#include "srllib.h"

#define MAX_TABLE_SIZE	6

class TPT
{
public:

	class TP : private DV_TPT
	{
		friend class TPT;

	public:
		TP(unsigned short termno,unsigned short flags,unsigned short value)
		{
			this->tp_termno	 = termno;
			this->tp_length	 = value;
			this->tp_flags	 = flags;
		}
	};

	TPT(): m_nNextParam(0)
	{
		::memset(m_list,0,sizeof(DV_TPT)*MAX_TABLE_SIZE);
	}

	TPT& operator<<(const TP& rhs)
	{
		//	assert for table overflow
		assert(m_nNextParam <= MAX_TABLE_SIZE);

		//	add the given parameter to table
		m_list[m_nNextParam].tp_termno = rhs.tp_termno;
		m_list[m_nNextParam].tp_length = rhs.tp_length;
		m_list[m_nNextParam].tp_flags = rhs.tp_flags;
		//	this is the last item
		m_list[m_nNextParam].tp_type = IO_EOT;
		
		//	this is the last parameter
		if(m_nNextParam > 0)
		{
			m_list[m_nNextParam-1].tp_type = IO_CONT;
		}

		m_nNextParam++;	//	increment to next parameter

		return *this;
	}

	operator const DV_TPT*() const 	{return m_list;}

private:
	DV_TPT	m_list[MAX_TABLE_SIZE];	// maximum size of the 
	int m_nNextParam;
};