#pragma once
#include "Application.h"
#include "Player.h"

class GameplayData
{
public:
	GameplayData() { m_someData = nullptr; }
	~GameplayData() { if (m_someData != nullptr) { free(m_someData); } }

	//Serialisation
	unsigned int Serialise(void** serialisedData);
	void Deserialise(void** serialisedData);
	void AddSomeData(char* data, unsigned int size);

	void AddPlayerSerialised(int a_i, char* a_playerData) { m_players[a_i] = a_playerData; }
	char* GetPlayerSerialised(int a_i) { return m_players[a_i]; }
	void AddPlayerSize(int a_i, unsigned int a_size) { m_playerSize[a_i] = a_size; }
	unsigned int GetPlayerSize(int a_i) { return m_playerSize[a_i]; }

	void SetNumberOfPlayers(unsigned int a_playerCount) { m_playerCount = a_playerCount; }
	unsigned int GetPlayerCount() { return m_playerCount; }
private:
	unsigned int m_playerCount;
	unsigned int m_playerSize[6];
	char* m_players[6];

	unsigned int m_id;
	void* m_someData;
	int m_extraSize;
};