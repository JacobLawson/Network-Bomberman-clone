#pragma once
#include "Application.h"

enum MATRIX_ROW
{
	RIGHT_VECTOR,
	UP_VECTOR,
	FORWARD_VECTOR,
	POSITION_VECTOR
};

class Player
{
public:
	Player() 
	{
		m_someData = nullptr; 
		transform = {{1, 0, 0, 0},
					{0, 1, 0, 0},
					{0, 0, 1, 0},
					{0, 0, 0, 1}};
	}
	~Player() {	if (m_someData != nullptr) { free(m_someData); } }

	unsigned int GetPlayerID() { return m_id; }
	void SetPlayerID(unsigned int a_id) { m_id = a_id; }

	unsigned int GetPlayerIDNum() { return m_playerNumber; }
	void SetPlayerIDNum(unsigned int a_id) { m_playerNumber = a_id; }

	unsigned int GetPlayerLogins() { return m_numberOfLogins; }
	void SetPlayerLogins(unsigned int a_id) { m_numberOfLogins = a_id; }

	char* GetName() { return m_name; }
	unsigned int GetNameLength() { return 256; }
	char* GetPassword() { return m_password; }
	unsigned int GetPasswordLength() { return 256; }

	void SetGameplayState(bool a_state) { m_GameplayStarted = a_state; }
	bool GetGameplayState() { return m_GameplayStarted; }


	glm::vec3 GetPosition() { return glm::vec3(x, y, z); }
	void SetPositionX(float a_xpos) { x = a_xpos; }
	void SetPositionY(float a_ypos) { y = a_ypos; }
	void SetPositionZ(float a_zpos) { z = a_zpos; }

	void Draw();

	unsigned int Serialise(void** serialisedData);

	void Deserialise(void** serialisedData);

	void AddSomeData(char* data, unsigned int size);

	//transform
	const glm::mat4& GetEntityMatrix() { return transform; }

	void SetEntityMatrixRow(MATRIX_ROW a_row, glm::vec3 a_vec3);
	glm::vec3 GetEntityMatrixRow(MATRIX_ROW a_row);

	//Death
	void SetDeath(bool a_val) { m_dead = a_val; }
	bool GetDeath() { return m_dead; }
	void SetRank(unsigned int a_rank) { m_rank = a_rank; }
	unsigned int GetRank() { return m_rank; }

private:
	unsigned int m_id;
	unsigned int m_playerNumber;
	unsigned int m_numberOfLogins;
	bool m_GameplayStarted;
	float x; float y; float z;
	glm::mat4 transform;
	char m_name[256];
	char m_password[256];
	int m_extraSize;
	void* m_someData;

	//Death
	bool m_dead;
	unsigned int m_rank;

	
};