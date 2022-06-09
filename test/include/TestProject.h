#ifndef __MY_APPLICATION_H__
#define __MY_APPLICATION_H__

#include "Application.h"
#include <stdafx.h>
//Raknet Includes
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <RakNetTypes.h>

#include "Player.h"
#include "GameplayData.h"
#include "DataStream.h"
#include <MapData.h>

namespace RakNet {
	class RakPeerInterface;
}

// Derived application class that wraps up all globals neatly
class TestProject : public Application
{
public:

	TestProject();
	virtual ~TestProject();

protected:

	typedef enum ConnectionState
	{
		CLIENT_SERVER_DECISION,		//are we a client or server first choice on startup
		SERVER_PROCESSING_EVENTS,
		CLIENT_CONNECTION,
		CLIENT_WAITING_FOR_CONNECTION,
		CLIENT_AUTHORISATION,
		CLIENT_WAITING_FOR_AUTHORISATION,
		CLIENT_WAITING_FOR_SERVER,
		SERVER_CLIENT_CONNECTED,
		START_GAME,
		GAME_RUNNING,
		END_GAME,
		SERVER_HANDLE_CLIENT_DISCONNECT,
		MAX_CONNECTION_STATES

	}ConnectionState;

	typedef enum CSNetMessages {
		AUTHENTICATE = ID_USER_PACKET_ENUM + 1,
		CLIENT_LOGIN_DATA,
		CLIENT_REGISTER_DATA,
		CLIENT_NEW_ID,
		CLIENT_UPDATE_DATA,
		CLIENT_DATA_REGISTERED,
		CLIENT_AUTHORISED,
		CLIENT_NOT_AUTHORISED,
		CSNET_MESSAGE_END,
		UPDATE_SERVER,
		UPDATE_SERVER_MAP,
		SERVER_UPDATED,
		SERVER_UPDATED_MAP,
		UPDATE_CLIENTS,
		UPDATE_CLIENTS_MAP,
		CLIENTS_UPDATE_COMPLETE,
		FETCHED_SERVER,
		FETCHED_MAP

	}CSNetMessages;

	virtual bool onCreate();
	virtual void Update(float a_deltaTime);
	virtual void Draw();
	virtual void Destroy();
	void Gameplay_Client();
	void Gameplay_Server();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;
	glm::mat4	m_playerTransform;

	bool m_isServer;
	bool m_waitingForServer;
	bool m_inLobby;
	bool m_keyPressed;
	bool m_mapInitialised;
	bool m_endGameInitialised;
	bool m_handlingDeath;

	//Raknet variables and functions
	void ServerProcessingEvents();
	bool ClientProcessingEvents();
	void ConsoleLogMessage(const char* a_message);

	//Client functions
	void UpdateServer();
	void RequestMap();
	void UpdateMap();
	void UpdateBomb();
	void BombReset();

	bool LoadMap(const char* a_filename );

	RakNet::RakPeerInterface* m_rakPeer;
	RakNet::SystemAddress m_serverAddress;
	RakNet::SystemAddress m_clientAddressBook[6];

	ConnectionState m_currentConnectionState;
	Player m_player;
	Player m_connectedPlayer[8];
	Player m_gameplayLobby[6];

	MapData m_loadedMap;

	unsigned int m_playerCount;
	unsigned int m_playerLobbyCount;
	unsigned int m_thisClientsPlayerNumber;
	unsigned int m_clientRank;

	DataStream* m_dataStream;

	glm::vec3 m_startingPositions[6] = {glm::vec3(2.0f, 0.0f, 2.0f),
										glm::vec3(2.0f, 0.0f, 21.0f),
										glm::vec3(21.0f, 0.0f, 2.0f),
										glm::vec3(21.0f, 0.0f, 21.0f),
										glm::vec3(12.0f, 0.0f, 21.0f),
										glm::vec3(2.0f, 0.0f, 12.0f)
	};

	glm::vec4 m_playerColours[6] = {glm::vec4(0.f, 0.f, 1.f, 1.f),
									glm::vec4(1.f, 0.f, 0.f, 1.f),
									glm::vec4(0.f, 1.f, 0.f, 1.f),
									glm::vec4(1.f, 0.f, 1.f, 1.f),
									glm::vec4(0.f, 0.f, 0.f, 1.f),
									glm::vec4(1.f, 1.f, 1.f, 1.f) };

	//Bomb
	bool m_bombPlaced;
	bool m_bombExploded;
	bool m_explosionState;
	float m_bombTimer;
	float m_explosionTimer;
	bool m_deathBroadcast;
	glm::vec2 m_playerBombPos;
};

#endif // __MY_APPLICATION_H__