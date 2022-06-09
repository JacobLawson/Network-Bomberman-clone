#include <stdafx.h>
#include "DataStream.h"

void DataStream::Resize(int a_size)
{
	if (a_size > 0 && a_size > sizeof(m_streamBuffer))
	{
		void* newbuffer = malloc(a_size);
		memcpy(newbuffer, m_streamBuffer, m_currentStreamHead);
		free(m_streamBuffer);
		m_streamBuffer = newbuffer;
	}
	else
	{
		void* newbuffer = malloc(m_streamCapacity * 2);
		memcpy(newbuffer, m_streamBuffer, m_currentStreamHead);
		free(m_streamBuffer);
		m_streamCapacity *= 2;
		m_streamBuffer = newbuffer;
	}
}

void DataStream::Write(const void* inData, unsigned int bytes)
{
	//Is there enough data?
	unsigned int finalWritePos = m_currentStreamHead + bytes;
	while (finalWritePos > m_streamCapacity)
	{
		Resize();
	}
	memcpy(m_streamBuffer, inData, bytes);
	m_currentStreamHead = finalWritePos;
}

void DataStream::Read()
{

}

void DataStream::Clear()
{
	memset(m_streamBuffer, 0, m_streamCapacity);
	m_currentStreamHead = 0;
}