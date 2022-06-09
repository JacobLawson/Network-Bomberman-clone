#pragma once
#include "Application.h"

class MapData
{
public:
	MapData() { m_someData = nullptr; }
	~MapData()
	{
		if (m_someData != nullptr)
		{
			free(m_someData);
		}
	};

	void InitialiseMapData();
	void SetMapData(int a_map[24][24]);
	int GetMapItem(int a_x, int a_y) { return m_map[a_x][a_y]; }
	void RemoveItem(int a_x, int a_y);
	void AddMapItem(int a_x, int a_y, int a_itemCode);
	void RemoveAllExplosions();

	void UpdateMap();
	void DrawMap();

	//matrix
	void SetEntityMatrixRow(glm::mat4 a_matrix, int a_row, glm::vec3 a_vec3);
	glm::vec3 GetEntityMatrixRow(glm::mat4 a_matrix, int a_row);

	//Serialisation
	unsigned int Serialise(void** serialisedData);

	void Deserialise(void** serialisedData);

	void AddSomeData(char* data, unsigned int size);

private:
	unsigned int m_id;
	void* m_someData;
	int m_extraSize;
	int  m_map[24][24];
	int m_mapPositionData[24][24];

	//key
	//-2 - Explosion
	//-1 - Bomb
	// 1 - wall
	// 2 - breakable
	// 3 - bomb
};