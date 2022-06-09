#include <stdafx.h>
#include "Player.h"

void Player::Draw()
{
	Gizmos::addBox(GetEntityMatrixRow(POSITION_VECTOR), glm::vec3(1.f), true, glm::vec4(1.f), GetEntityMatrix());
}

unsigned int Player::Serialise(void** serialisedData)
{
	unsigned int sizeOfPlayerData = sizeof(Player) - sizeof(char*) + m_extraSize;
	*serialisedData = malloc(sizeOfPlayerData);
	unsigned int offset = 0;
	memcpy(*serialisedData, &m_id, sizeof(Player) - sizeof(char*));
	offset += (sizeof(Player) - sizeof(char*));
	if (m_someData != nullptr && m_extraSize > 0)
	{
		memcpy((char*)(*serialisedData) + offset, &((char*)(m_someData))[0], m_extraSize);
	}
	return sizeOfPlayerData;
}

void Player::Deserialise(void** serialisedData)
{
	char* databuffer = (char*)(*serialisedData);
	memcpy(&m_id, databuffer, sizeof(Player) - sizeof(char*));
	databuffer += sizeof(Player) - sizeof(char*);
	if (m_extraSize > 0)
	{
		m_someData = malloc(m_extraSize);
		memcpy(m_someData, databuffer, m_extraSize);
	}

}

void Player::AddSomeData(char* data, unsigned int size)
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

void Player::SetEntityMatrixRow(MATRIX_ROW a_row, glm::vec3 a_vec3)
{
	transform[a_row] = glm::vec4(a_vec3, (a_row == POSITION_VECTOR ? 1.f : 0.0f));
}

glm::vec3 Player::GetEntityMatrixRow(MATRIX_ROW a_row)
{
	return transform[a_row];
}
