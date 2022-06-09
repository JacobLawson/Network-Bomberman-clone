#pragma once
#include "Application.h"

class DataStream
{
public:
	DataStream()
	{
		m_streamCapacity = 128;
		m_streamBuffer = malloc(m_streamCapacity);
		m_currentStreamHead = 0;
	}
	~DataStream() { free(m_streamBuffer); }

	void Resize(int a_size = 0);

	void Write(const void* inData, unsigned int bytes);

		void Read();

	void Clear();

	void* GetBufferData() { return m_streamBuffer; }
	unsigned int GetBufferReadSize() { return m_currentStreamHead; }

private:
	unsigned int m_currentStreamHead;
	unsigned int m_streamCapacity;
	void* m_streamBuffer;
};