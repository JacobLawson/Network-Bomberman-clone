#include <stdafx.h>
#include <GameplayData.h>

unsigned int GameplayData::Serialise(void** serialisedData)
{
	unsigned int sizeOfPlayersData = sizeof(GameplayData) - sizeof(char*) + m_extraSize;
	*serialisedData = malloc(sizeOfPlayersData);
	unsigned int offset = 0;
	memcpy(*serialisedData, &m_id, sizeof(GameplayData) - sizeof(char*));
	offset += (sizeof(GameplayData) - sizeof(char*));
	if (m_someData != nullptr && m_extraSize > 0)
	{
		memcpy((char*)(*serialisedData) + offset, &((char*)(m_someData))[0], m_extraSize);
	}
	return sizeOfPlayersData;
}

void GameplayData::Deserialise(void** serialisedData)
{
	char* databuffer = (char*)(*serialisedData);
	memcpy(&m_id, databuffer, sizeof(GameplayData) - sizeof(char*));
	databuffer += sizeof(GameplayData) - sizeof(char*);
	if (m_extraSize > 0)
	{
		m_someData = malloc(m_extraSize);
		memcpy(m_someData, databuffer, m_extraSize);
	}
}

void GameplayData::AddSomeData(char* data, unsigned int size)
{
	if (m_someData != nullptr)
	{
		free(m_someData);
		m_someData = nullptr;
	}
	m_someData = malloc(size);
	memcpy(m_someData, data, size);
	m_extraSize = size;
}
