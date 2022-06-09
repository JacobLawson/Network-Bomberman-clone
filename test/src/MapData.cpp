#include <stdafx.h>
#include "MapData.h"

void MapData::InitialiseMapData()
{
	
}

void MapData::SetMapData(int a_map[24][24])
{
	for (int x = 0; x < 24; x++)
	{
		for (int y = 0; y < 24; y++)
		{
			m_map[x][y] = a_map[x][y];
		}
	}
}

void MapData::RemoveItem(int a_x, int a_y)
{
	m_map[a_x][a_y] = 0;
}

void MapData::AddMapItem(int a_x, int a_y, int a_itemCode)
{
	m_map[a_x][a_y] = a_itemCode;
}

void MapData::RemoveAllExplosions()
{
	for (int x = 0; x < 24; x++)
	{
		for (int y = 0; y < 24; y++)
		{
			if (m_map[x][y] == -2)
			{
				m_map[x][y] = 0;
			}
		}
	}
}

void MapData::UpdateMap()
{
}

void MapData::DrawMap()
{
	float DrawX = -0.f;
	float DrawY = -0.f;
	for (int x = 0; x < 24; x++)
	{
		for (int y = 0; y < 24; y++)
		{
			glm::mat4 myMatrix = glm::mat4(1.f);
			glm::vec3 drawpos = glm::vec3(DrawX, 0.f, DrawY);
			SetEntityMatrixRow(myMatrix, 3, drawpos);
			//Draw Wall
			if (m_map[x][y] == 1)
			{			
				Gizmos::addBox(drawpos, glm::vec3(1.f), true, glm::vec4(1.f, 1.f, 1.f, 1.f), myMatrix);
			}
			else if (m_map[x][y] == 2)	//Draw Blocks
			{
				Gizmos::addBox(drawpos, glm::vec3(1.f), true, glm::vec4(0.5f, 0.5f, 0.5f, 0.5f), myMatrix);
			}
			else if (m_map[x][y] == -1) //Draw Bomb
			{
				Gizmos::addSphere(drawpos, 10, 10, 1.f, glm::vec4(0.f, 0.f, 0.f, 1.f), &myMatrix);
			}
			else if (m_map[x][y] == -2) //Draw Explosion
			{
				Gizmos::addBox(drawpos, glm::vec3(1.f), true, glm::vec4(1.f, 0.25f, 0.f, 1.f), myMatrix);
			}
			DrawY += 1.f;
		}
		DrawY = -0.f;
		DrawX += 1.f;
	}
}

void MapData::SetEntityMatrixRow(glm::mat4 a_matrix, int a_row, glm::vec3 a_vec3)
{
	a_matrix[a_row] = glm::vec4(a_vec3, (a_row == 3 ? 1.f : 0.0f));
}

glm::vec3 MapData::GetEntityMatrixRow(glm::mat4 a_matrix, int a_row)
{
	return a_matrix[a_row];
}

unsigned int MapData::Serialise(void** serialisedData)
{
	unsigned int sizeOfPlayerData = sizeof(MapData) - sizeof(char*) + m_extraSize;
	*serialisedData = malloc(sizeOfPlayerData);
	unsigned int offset = 0;
	memcpy(*serialisedData, &m_id, sizeof(MapData) - sizeof(char*));
	offset += (sizeof(MapData) - sizeof(char*));
	if (m_someData != nullptr && m_extraSize > 0)
	{
		memcpy((char*)(*serialisedData) + offset, &((char*)(m_someData))[0], m_extraSize);
	}
	return sizeOfPlayerData;
}

void MapData::Deserialise(void** serialisedData)
{
	char* databuffer = (char*)(*serialisedData);
	memcpy(&m_id, databuffer, sizeof(MapData) - sizeof(char*));
	databuffer += sizeof(MapData) - sizeof(char*);
	if (m_extraSize > 0)
	{
		m_someData = malloc(m_extraSize);
		memcpy(m_someData, databuffer, m_extraSize);
	}
}

void MapData::AddSomeData(char* data, unsigned int size)
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