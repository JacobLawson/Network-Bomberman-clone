#include <stdafx.h>
#include "TestProject.h"
#include <BitStream.h>

#define MAX_CLIENTS 8
#define SERVER_PORT 60000

TestProject::TestProject()
{

}

TestProject::~TestProject()
{

}

bool TestProject::onCreate()
{
	m_playerTransform = glm::mat4(1.f);
	m_explosionTimer = -1.f;
	m_isServer = false;
	//Instantiate RakNet
	m_rakPeer = RakNet::RakPeerInterface::GetInstance();
	//Set current connection state
	m_currentConnectionState = CLIENT_SERVER_DECISION;
	m_dataStream = new DataStream();
	//m_player.AddSomeData("Hello extra data", 17);
	// initialise the Gizmos helper class
	Gizmos::create();
	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse(glm::lookAt(glm::vec3(10, 10, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, m_windowWidth / (float)m_windowHeight, 0.1f, 1000.0f);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f, 0.25f, 0.25f, 1.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return true;
}

void TestProject::ConsoleLogMessage(const char* a_message)
{
	Application_Log* log = Application_Log::Get();
	if (log != nullptr)
	{
		log->addLog(LOG_LEVEL::LOG_INFO, a_message);
	}
}

void TestProject::Update(float a_deltaTime)
{
	Application_Log* log = Application_Log::Get();
#pragma region Gizmos
	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement(m_cameraMatrix, a_deltaTime, 10);

	// clear all gizmos from last frame
	Gizmos::clear();

	// add an identity matrix gizmo
	Gizmos::addTransform(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
	// add a 20x20 grid on the XZ-plane
	for (int i = 0; i < 25; ++i)
	{
		Gizmos::addLine(glm::vec3(0 + i, 0, 24), glm::vec3(0 + i, 0, 0),
			i == 24 ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 0, 0, 1));

		Gizmos::addLine(glm::vec3(24, 0, 0 + i), glm::vec3(0, 0, 0 + i),
			i == 24 ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 0, 0, 1));

		
	}

#pragma endregion

	//Setup imgui next window size and pos
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 windowSize = ImVec2(400.f, 250.f);
	ImVec2 windowPos = ImVec2(io.DisplaySize.x * 0.5f - windowSize.x * 0.5f, io.DisplaySize.y * 0.5f - windowSize.y * 0.5f);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	//Proccess current State
	static bool show_connection_window = true;
	switch (m_currentConnectionState)
	{
	case (ConnectionState::CLIENT_SERVER_DECISION):
	{
		ImGui::Begin("Establish Connection", &show_connection_window);
		{
			ImGui::Text("Please Choose if you are a client or server");
			if (ImGui::Button("Client", ImVec2(60, 30)))
			{
				m_isServer = false;
				RakNet::SocketDescriptor sd;
				m_rakPeer->Startup(1, &sd, 1);
				ConsoleLogMessage("Client Start up...");
				m_currentConnectionState = ConnectionState::CLIENT_CONNECTION;
			}
			if (ImGui::Button("Server", ImVec2(60, 30)))
			{
#pragma region mapData
				LoadMap("Map.map");
				//LoadMap("C:/Users/Work/Documents/GitHub/ct5052-network-bomberman-s1901022/test/Map.map");
#pragma endregion

				m_isServer = true;
				m_playerCount = 0;
				RakNet::SocketDescriptor sd(SERVER_PORT, 0);
				m_rakPeer->Startup(MAX_CLIENTS, &sd, 1);
				m_rakPeer->SetMaximumIncomingConnections(MAX_CLIENTS);
				ConsoleLogMessage("Server Start up...");
				m_currentConnectionState = ConnectionState::SERVER_PROCESSING_EVENTS;
			}
		} ImGui::End();
		break;
	}
	case (ConnectionState::CLIENT_CONNECTION):
 	{
		ImGui::Begin("Connect to Server", &show_connection_window);
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Enter Server IP Address");
			static int ipAddress[4] = { 127, 0, 0, 1 };
			ImGui::InputInt4("Server Address:", ipAddress);
			if (ImGui::Button("Connect"))
			{
				ConsoleLogMessage("Client Connection Started");
				std::stringstream ss;
				ss << ipAddress[0] << "." << ipAddress[1] << "." << ipAddress[2] << "." << ipAddress[3];
				m_rakPeer->Connect(ss.str().c_str(), SERVER_PORT, 0, 0);
				m_currentConnectionState = ConnectionState::CLIENT_WAITING_FOR_CONNECTION;
			}
		}ImGui::End();
		break;
	}
	case (ConnectionState::SERVER_PROCESSING_EVENTS):
	{
		ServerProcessingEvents();
		break;
	}
	case (ConnectionState::CLIENT_WAITING_FOR_CONNECTION):
	{
		ImGui::Begin("Waiting for Connection", &show_connection_window);
		{
			ImGui::Text("Waiting for Connection... %c", "|/-\\"[(int)(Utility::getTotalTime() / 0.05f) & 3]);
			RakNet::Packet* packet = m_rakPeer->Receive();
			while (packet != nullptr)
			{
				switch (packet->data[0])
				{
				case ID_CONNECTION_REQUEST_ACCEPTED:
				{
					ConsoleLogMessage("Our Connection request has been accepted");
					break;
				}
				case(CSNetMessages::AUTHENTICATE):
				{
					m_serverAddress = packet->systemAddress;
					RakNet::RakString rs;
					RakNet::BitStream bsIn(packet->data, packet->length, false);
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);
					ConsoleLogMessage(rs.C_String());
					m_currentConnectionState = ConnectionState::CLIENT_AUTHORISATION;
					break;
				}
				case ID_NO_FREE_INCOMING_CONNECTIONS:
				{
					ImGui::Text("The Server is full");
					m_rakPeer->CloseConnection(packet->systemAddress, true);
					ConsoleLogMessage("Connection Closed due to server being full");
					m_currentConnectionState = ConnectionState::CLIENT_SERVER_DECISION;
					break;
				}
				default:
					break;
				}
				m_rakPeer->DeallocatePacket(packet);
				packet = m_rakPeer->Receive();
			}
		} ImGui::End();
		break;
	}
	case (ConnectionState::CLIENT_AUTHORISATION):
	{
		ImGui::Begin("Enter Login Details", &show_connection_window);
		{
			ImGui::InputText("Username", m_player.GetName(), m_player.GetNameLength());
			ImGui::InputText("Password", m_player.GetPassword(), m_player.GetPasswordLength());

			char* playerData = nullptr;
			unsigned int playerSize = m_player.Serialise((void**)(&playerData));
			m_dataStream->Clear();
			m_dataStream->Write(playerData, playerSize);

			if (ImGui::Button("Login"))
			{
				RakNet::BitStream loginCreds;
				loginCreds.Write((RakNet::MessageID)CSNetMessages::CLIENT_LOGIN_DATA);				
				
				loginCreds.Write((char*)(m_dataStream->GetBufferData()), m_dataStream->GetBufferReadSize());
				m_rakPeer->Send(&loginCreds, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_serverAddress, false);
				ConsoleLogMessage("Sending Login Data");
				m_currentConnectionState = ConnectionState::CLIENT_WAITING_FOR_AUTHORISATION;
			}
			if (ImGui::Button("Register"))
			{
				RakNet::BitStream registrationCreds;
				registrationCreds.Write((RakNet::MessageID)CSNetMessages::CLIENT_REGISTER_DATA);

				registrationCreds.Write((char*)(m_dataStream->GetBufferData()), m_dataStream->GetBufferReadSize());
				m_rakPeer->Send(&registrationCreds, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_serverAddress, false);
				ConsoleLogMessage("Sending Login Data");
				m_currentConnectionState = ConnectionState::CLIENT_WAITING_FOR_AUTHORISATION;
			}
			free(playerData);
		}ImGui::End();
		break;
	}
	case (ConnectionState::CLIENT_WAITING_FOR_AUTHORISATION):
	{
		ImGui::Begin("Waiting for Server to authenticate", &show_connection_window);
		{
			ImGui::Text("Waiing for Connection... %c", "|/-\\"[(int)(Utility::getTotalTime() / 0.05f) & 3]);
			RakNet::Packet* packet = m_rakPeer->Receive();
			while (packet != nullptr)
			{
				switch (packet->data[0])
				{
				case(CSNetMessages::CLIENT_DATA_REGISTERED):
				{
					m_serverAddress = packet->systemAddress;
					RakNet::RakString rs;
					RakNet::BitStream bsIn(packet->data, packet->length, false);
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);
					ConsoleLogMessage(rs.C_String());
					m_currentConnectionState = ConnectionState::CLIENT_AUTHORISATION;
					break;
				}
				case(CSNetMessages::CLIENT_AUTHORISED):
				{
					m_serverAddress = packet->systemAddress;
					RakNet::RakString rs;
					RakNet::BitStream bsIn(packet->data, packet->length, false);
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					unsigned int bytesToRead = packet->length - sizeof(RakNet::MessageID);
					char* playerData = (char*)malloc(bytesToRead);
					bsIn.Read(playerData, bytesToRead);

					//Deserialise this clients player
					Player player;
					player.Deserialise((void**)(&playerData));
					ConsoleLogMessage(player.GetName());
					free(playerData);

					//tell the client which player it is playing as
					m_thisClientsPlayerNumber = player.GetPlayerIDNum();
					//place player into lobby array 
					m_gameplayLobby[m_thisClientsPlayerNumber - 1] = player;
					UpdateServer();

					m_currentConnectionState = ConnectionState::START_GAME;

					break;
				}
				case(CSNetMessages::CLIENT_NOT_AUTHORISED):
				{
					m_serverAddress = packet->systemAddress;
					RakNet::RakString rs;
					RakNet::BitStream bsIn(packet->data, packet->length, false);
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);
					ConsoleLogMessage(rs.C_String());
					m_currentConnectionState = ConnectionState::CLIENT_AUTHORISATION;
					break;
				}
				default:
					break;
				}
				m_rakPeer->DeallocatePacket(packet);
				packet = m_rakPeer->Receive();
			}
		}ImGui::End();
		break;
	}
	case(ConnectionState::START_GAME):
	{
		ClientProcessingEvents();

		ImGui::Begin("Lobby", &show_connection_window);
		{
			//Initialise the player
			m_gameplayLobby[m_thisClientsPlayerNumber - 1].SetRank(0);

			//Login Count
			std::string sLoginCount = std::to_string(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPlayerLogins());
			char const* Loginchar = sLoginCount.c_str();

			ImGui::Text("User: ");
			ImGui::Text(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetName());
			ImGui::Text("Log in: ");
			ImGui::Text(Loginchar);

			//number of Players in Lobby
			unsigned int lobbyCount = 0;
			for (unsigned int i = 0; i < 6; i++)
			{
				if (m_gameplayLobby[i].GetPlayerIDNum() != 0)
				{
					lobbyCount++;
				}
			}
			m_playerLobbyCount = lobbyCount;
			std::string sLobbyCount = std::to_string(lobbyCount);
			char const* Lobbychar = sLobbyCount.c_str();

			ImGui::Text("Players in Lobby: ");
			ImGui::Text(Lobbychar);



			//Wait for at least four players to join
			if (m_gameplayLobby[0].GetPlayerIDNum() != 0)	//player four has joined
			{
				ImGui::Text("Ready to play!");
				if (ImGui::Button("Start Game"))
				{
					m_gameplayLobby[m_thisClientsPlayerNumber - 1].SetGameplayState(true);
					UpdateServer();
					ConsoleLogMessage("Updating Start at server");
				}

				if (m_gameplayLobby[0].GetGameplayState() == true)
				{
					ConsoleLogMessage("Starting Game!");
					m_gameplayLobby[m_thisClientsPlayerNumber - 1].SetGameplayState(true);
					m_gameplayLobby[m_thisClientsPlayerNumber - 1].SetPositionX(m_startingPositions[m_thisClientsPlayerNumber - 1].x);
					m_gameplayLobby[m_thisClientsPlayerNumber - 1].SetPositionZ(m_startingPositions[m_thisClientsPlayerNumber - 1].z);
					UpdateServer();
					m_currentConnectionState = ConnectionState::GAME_RUNNING;
				}
			}
			else
			{
				ImGui::Text("Waiting for Players to Join");
			}

		}ImGui::End();

		break;
	}
	case (ConnectionState::GAME_RUNNING):
	{
		bool serverCommunication = ClientProcessingEvents();
				
		//Player inputs for controls
		if (glfwGetKey(m_window, GLFW_KEY_LEFT) == true)	//Left
		{
			if (!m_keyPressed)
			{
				if (m_loadedMap.GetMapItem(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().x - 1, m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().z) < 1)
				{
					m_gameplayLobby[m_thisClientsPlayerNumber - 1].SetPositionX(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().x - 1);
					UpdateServer();
					m_keyPressed = true;
				}
			}
		}
		else if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == true)	//Right
		{
			if (!m_keyPressed)
			{
				if (m_loadedMap.GetMapItem(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().x + 1, m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().z) < 1)
				{
					m_gameplayLobby[m_thisClientsPlayerNumber - 1].SetPositionX(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().x + 1);
					UpdateServer();
					m_keyPressed = true;
				}
			}
		}
		else if (glfwGetKey(m_window, GLFW_KEY_DOWN) == true)	//Down
		{
			if (!m_keyPressed)
			{
				if (m_loadedMap.GetMapItem(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().x, m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().z + 1) < 1)
				{
					m_gameplayLobby[m_thisClientsPlayerNumber - 1].SetPositionZ(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().z + 1);
					UpdateServer();
					m_keyPressed = true;
				}
			}
		}
		else if (glfwGetKey(m_window, GLFW_KEY_UP) == true)	//Up
		{
			if (!m_keyPressed)
			{
				if (m_loadedMap.GetMapItem(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().x, m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().z - 1) < 1)
				{
					m_gameplayLobby[m_thisClientsPlayerNumber - 1].SetPositionZ(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().z - 1);
					UpdateServer();
					m_keyPressed = true;
				}
			}
		}
		//Check if player has been killed by an explosion
		else if (m_loadedMap.GetMapItem(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().x, m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().z) == -2)
		{
			while (!ClientProcessingEvents())
			{
				m_gameplayLobby[m_thisClientsPlayerNumber - 1].SetDeath(true);

				unsigned int provisionalRank = 6;
				for (int i = 0; i < 6; i++)
				{
					if (m_gameplayLobby[i].GetPlayerIDNum() > 0)
					{
						provisionalRank--;
					}
				}
				m_gameplayLobby[m_thisClientsPlayerNumber - 1].SetRank(provisionalRank);
				UpdateServer();
			}
			m_currentConnectionState = ConnectionState::END_GAME;
		}
		//Bomb
		else if (glfwGetKey(m_window, GLFW_KEY_SPACE) == true)
		{
			//place a bomb if ones not placed yet
			if (m_bombPlaced != true)
			{
				m_loadedMap.AddMapItem(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().x, m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().z, -1);
				m_bombPlaced = true;
				m_playerBombPos = glm::vec2(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().x, m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetPosition().z);
				m_bombTimer = 5.0f;
				UpdateMap();
			}
		}
		else
		{
			m_keyPressed = false;
		}

		//Update Bomb
		if (m_bombPlaced == true)
		{
			UpdateBomb();
		}

		//Bomb update and explosion
		if (m_bombExploded == true && m_bombPlaced == true)
		{
			//Explosions are blocked by walls
			if (m_loadedMap.GetMapItem(m_playerBombPos.x - 1, m_playerBombPos.y) < 1)
			{
				m_loadedMap.AddMapItem(m_playerBombPos.x - 1, m_playerBombPos.y, -2);
			}
			if (m_loadedMap.GetMapItem(m_playerBombPos.x + 1, m_playerBombPos.y) < 1)
			{
				m_loadedMap.AddMapItem(m_playerBombPos.x + 1, m_playerBombPos.y, -2);
			}
			if (m_loadedMap.GetMapItem(m_playerBombPos.x, m_playerBombPos.y - 1) < 1)
			{
				m_loadedMap.AddMapItem(m_playerBombPos.x, m_playerBombPos.y - 1, -2);
			}
			if (m_loadedMap.GetMapItem(m_playerBombPos.x, m_playerBombPos.y + 1) < 1)
			{
				m_loadedMap.AddMapItem(m_playerBombPos.x, m_playerBombPos.y + 1, -2);
			}

			m_loadedMap.RemoveItem(m_playerBombPos.x, m_playerBombPos.y);
			//Blow up left
			if (m_loadedMap.GetMapItem(m_playerBombPos.x - 1, m_playerBombPos.y) == 2)
			{
				m_loadedMap.RemoveItem(m_playerBombPos.x - 1, m_playerBombPos.y);
			}
			//Blow up right
			if (m_loadedMap.GetMapItem(m_playerBombPos.x + 1, m_playerBombPos.y) == 2)
			{
				m_loadedMap.RemoveItem(m_playerBombPos.x + 1, m_playerBombPos.y);
			}
			//Blow up up
			if (m_loadedMap.GetMapItem(m_playerBombPos.x, m_playerBombPos.y - 1) == 2)
			{
				m_loadedMap.RemoveItem(m_playerBombPos.x, m_playerBombPos.y - 1);
			}
			//Blow up down
			if (m_loadedMap.GetMapItem(m_playerBombPos.x, m_playerBombPos.y + 1) == 2)
			{
				m_loadedMap.RemoveItem(m_playerBombPos.x, m_playerBombPos.y + 1);
			}
			m_explosionTimer = 1.0f;
			m_explosionState = true;
			UpdateMap();
			BombReset();
		}

		if (m_explosionState == true)
		{
			if (m_explosionTimer > 0.0f)
			{
				m_explosionTimer -= Utility::getDeltaTime();
			}
			else
			{
				m_loadedMap.RemoveAllExplosions();
				UpdateMap();
				m_explosionState = false;
			}
		}

		//the inital request to the server to get the map
		if (!serverCommunication && !m_mapInitialised)
		{
			RequestMap();
			m_mapInitialised = true;
		}
		//DrawMap
		m_loadedMap.DrawMap();

		//Output updates TEST
		for (unsigned int i = 0; i < 6; i++)
		{
			//Check to make sure player is valid
			if (m_gameplayLobby[i].GetPlayerIDNum() != 0)
			{
				ConsoleLogMessage(m_gameplayLobby[i].GetName());

				//Draw Players
				//check is alive
				if (m_gameplayLobby[i].GetDeath() == false)
				{
					m_gameplayLobby[i].SetEntityMatrixRow(POSITION_VECTOR, glm::vec3(m_gameplayLobby[i].GetEntityMatrixRow(POSITION_VECTOR).x, 0.0f, m_gameplayLobby[i].GetEntityMatrixRow(POSITION_VECTOR).z));
					Gizmos::addBox(m_gameplayLobby[i].GetPosition(), glm::vec3(1.f), true, m_playerColours[i], m_gameplayLobby[i].GetEntityMatrix());
				}
			}
		}

		break;
	}
	case (ConnectionState::END_GAME):
	{
		bool serverCommunication = ClientProcessingEvents();

		ImGui::Begin("GameOver", &show_connection_window);
		{
			ImGui::Text("User: ");
			ImGui::Text(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetName());

			std::string sLobbRank = std::to_string(m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetRank());
			char const* LobbyRank = sLobbRank.c_str();

			ImGui::Text("Rank: ");
			ImGui::Text(LobbyRank);

			if (m_gameplayLobby[m_thisClientsPlayerNumber - 1].GetRank() <= 1)
			{
				ImGui::Text("Winner ");
			}

			if (ImGui::Button("Return"))
			{
				m_currentConnectionState = CLIENT_SERVER_DECISION;
			}

		} ImGui::End();

	}
		break;
	default:
		break;
	}

	static bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);
	if (log != nullptr && show_demo_window)
	{
		log->showLog(&show_demo_window);
	}
	//show application log window
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(m_window, GLFW_KEY_L) == GLFW_PRESS) {
		show_demo_window = !show_demo_window;
	}
	// quit our application when escape is pressed
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void TestProject::Draw()
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);

	// draw the gizmos from this frame
	Gizmos::draw(viewMatrix, m_projectionMatrix);
}

void TestProject::Destroy()
{
	RakNet::RakPeerInterface::DestroyInstance(m_rakPeer);
	Gizmos::destroy();
}


void TestProject::ServerProcessingEvents()
{
	RakNet::Packet* packet = m_rakPeer->Receive();
	while (packet != nullptr)
	{
		switch (packet->data[0])
		{
		case(ID_REMOTE_DISCONNECTION_NOTIFICATION):
		{
			ConsoleLogMessage("Another Client has disconnected");
			break;
		}
		case(ID_REMOTE_CONNECTION_LOST):
		{
			ConsoleLogMessage("Another Client has lost connection");
			break;
		}
		case(ID_REMOTE_NEW_INCOMING_CONNECTION):
		{
			ConsoleLogMessage("Another Client is attempting to connect");
			break;
		}
		case(ID_CONNECTION_REQUEST_ACCEPTED):
		{
			ConsoleLogMessage("Our Connection Request has been Accepted");
			break;
		}
		case(ID_NEW_INCOMING_CONNECTION):
		{
			ConsoleLogMessage("Client is attempting to connect");
			RakNet::BitStream connectionAuthorise;
			connectionAuthorise.Write((RakNet::MessageID)CSNetMessages::AUTHENTICATE);
			connectionAuthorise.Write("Identify Yourself!");
			m_rakPeer->Send(&connectionAuthorise, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			break;
		}
		case(ID_NO_FREE_INCOMING_CONNECTIONS):
		{
			ConsoleLogMessage("The Server os full");
			break;
		}
		case(ID_DISCONNECTION_NOTIFICATION):
		{
			ConsoleLogMessage("A Client has Disconnected");
			break;
		}
		case(ID_CONNECTION_LOST):
		{
			ConsoleLogMessage("A Client has Lost the Connection Lost");
			break;
		}
		case(CSNetMessages::CLIENT_LOGIN_DATA):
		{
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			unsigned int bytesToRead = packet->length - sizeof(RakNet::MessageID);
			char* playerData = (char*)malloc(bytesToRead);
			bsIn.Read(playerData, bytesToRead);

			Player player;
			player.Deserialise((void**)(&playerData));
			ConsoleLogMessage("Recieved Username and password from client");
			ConsoleLogMessage("Locating Player in Registered Players");
			free(playerData);
			bool foundPlayer = false;
			for (unsigned int i = 0; i < m_playerCount; i++)
			{
				if (strcmp(m_connectedPlayer[i].GetName(), player.GetName()) == 0)
				{
					if (strcmp(m_connectedPlayer[i].GetPassword(), player.GetPassword()) == 0)
					{
						m_connectedPlayer[i].SetPlayerLogins(player.GetPlayerLogins() + 1);

						bool spaceFound = false;
						unsigned int thisPlayerInLobby = 0;
						//loop through lobby to add the player
						for (unsigned int j = 0; j < 6; j++)
						{
							if (!spaceFound)
							{
								//check to make sure space is empty
								if (m_gameplayLobby[j].GetPlayerIDNum() == 0)
								{
									//add player network address to server address array
									m_clientAddressBook[j] = packet->systemAddress;
									//send over number of logins for player
									player.SetPlayerLogins(m_connectedPlayer[i].GetPlayerLogins());
									//Give player number (1-6)
									player.SetPlayerIDNum(j + 1);
									//save player into lobby
									m_gameplayLobby[j] = player;

									//increment player count in game
									m_playerLobbyCount++;

									//breakout
									thisPlayerInLobby = j;
									spaceFound = true;
								}
							}
						}

						if (spaceFound)
						{
							//Send player data back to client
							char* playerData = nullptr;
							unsigned int playerSize = m_gameplayLobby[thisPlayerInLobby].Serialise((void**)(&playerData));
							m_dataStream->Clear();
							m_dataStream->Write(playerData, playerSize);

							RakNet::BitStream connectionAuthorise;
							connectionAuthorise.Write((RakNet::MessageID)CSNetMessages::CLIENT_AUTHORISED);
							connectionAuthorise.Write((char*)(m_dataStream->GetBufferData()), m_dataStream->GetBufferReadSize());
							//connectionAuthorise.Write("Successful Login");

							m_rakPeer->Send(&connectionAuthorise, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
							free(playerData);

							foundPlayer = true;
						}
						break;
					}
					else 
					{
						RakNet::BitStream connectionAuthorise;
						connectionAuthorise.Write((RakNet::MessageID)CSNetMessages::CLIENT_AUTHORISED);
						connectionAuthorise.Write("Login Failed - Incorrect Password");
						m_rakPeer->Send(&connectionAuthorise, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
						break;
					}
				}
			}
			if (!foundPlayer)
			{
				RakNet::BitStream connectionAuthorise;
				connectionAuthorise.Write((RakNet::MessageID)CSNetMessages::CLIENT_NOT_AUTHORISED);
				connectionAuthorise.Write("Login Failed - Player not found");
				m_rakPeer->Send(&connectionAuthorise, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				break;
			}
			break;
		}
		case (CSNetMessages::CLIENT_REGISTER_DATA):
		{
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			unsigned int bytesToRead = packet->length - sizeof(RakNet::MessageID);
			char* playerData = (char*)malloc(bytesToRead);
			bsIn.Read(playerData, bytesToRead);
			unsigned int cp = m_playerCount;
			m_connectedPlayer[cp].Deserialise((void**)(&playerData));

			ConsoleLogMessage("Recieved Username and password from client");
			ConsoleLogMessage(m_connectedPlayer[cp].GetName());
			ConsoleLogMessage(m_connectedPlayer[cp].GetPassword());
			m_playerCount++;
			free(playerData);

			RakNet::BitStream connectionAuthorise;
			connectionAuthorise.Write((RakNet::MessageID)CSNetMessages::CLIENT_DATA_REGISTERED);
			connectionAuthorise.Write("Successful Registration!");
			m_rakPeer->Send(&connectionAuthorise, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);

			break;
		}
		case (CSNetMessages::UPDATE_CLIENTS):
		{
			ConsoleLogMessage("Sending player data to logged in clients");

			for (unsigned int i = 0; i < 6; i++)	//Client
			{	
				for (unsigned int j = 0; j < 6; j++)	//Players
				{
					//Send player data back to client
					char* playerData = nullptr;
					unsigned int playerSize = m_gameplayLobby[j].Serialise((void**)(&playerData));
					m_dataStream->Clear();
					m_dataStream->Write(playerData, playerSize);

					RakNet::BitStream clientData;
					clientData.Write((RakNet::MessageID)CSNetMessages::FETCHED_SERVER);
					clientData.Write((char*)(m_dataStream->GetBufferData()), m_dataStream->GetBufferReadSize());
					m_rakPeer->Send(&clientData, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_clientAddressBook[i], false);
					free(playerData);
				}
			}
			RakNet::BitStream updateStatus;
			updateStatus.Write((RakNet::MessageID)CSNetMessages::CLIENTS_UPDATE_COMPLETE);
			m_rakPeer->Send(&updateStatus, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			packet = nullptr;
			break;
		}
		case (CSNetMessages::UPDATE_SERVER):
		{
			//Read in player Data
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			unsigned int bytesToRead = packet->length - sizeof(RakNet::MessageID);
			char* playerData = (char*)malloc(bytesToRead);
			bsIn.Read(playerData, bytesToRead);

			Player player;
			player.Deserialise((void**)(&playerData));
			m_gameplayLobby[player.GetPlayerIDNum() - 1] = player;

			//Allow all users to start game when a single client press of button
			if (player.GetGameplayState() == true)
			{
				for (int i = 0; i < 6; i++)
				{
					if (m_gameplayLobby[i].GetPlayerIDNum() != 0)
					{
						m_gameplayLobby[i].SetGameplayState(true);
					}
				}
			}

			RakNet::BitStream updateComplete;
			updateComplete.Write((RakNet::MessageID)CSNetMessages::SERVER_UPDATED);
			updateComplete.Write("Server has updated our player");
			m_rakPeer->Send(&updateComplete, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			free(playerData);
			break;
		}
		case(CSNetMessages::UPDATE_SERVER_MAP):
		{
			//Read in Map Data
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			unsigned int bytesToRead = packet->length - sizeof(RakNet::MessageID);
			char* mapData = (char*)malloc(bytesToRead);
			bsIn.Read(mapData, bytesToRead);
;
			m_loadedMap.Deserialise((void**)(&mapData));
			free(mapData);

			RakNet::BitStream updateComplete;
			updateComplete.Write((RakNet::MessageID)CSNetMessages::SERVER_UPDATED_MAP);
			updateComplete.Write("Server has updated our Map");
			m_rakPeer->Send(&updateComplete, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);

			break;
		}
		case (CSNetMessages::UPDATE_CLIENTS_MAP):
		{
			for (unsigned int i = 0; i < 6; i++)	//Client
			{
				for (unsigned int j = 0; j < 6; j++)	//Players
				{
					//Send player data back to client
					char* mapData = nullptr;
					unsigned int mapSize = m_loadedMap.Serialise((void**)(&mapData));
					m_dataStream->Clear();
					m_dataStream->Write(mapData, mapSize);

					RakNet::BitStream clientData;
					clientData.Write((RakNet::MessageID)CSNetMessages::FETCHED_MAP);
					clientData.Write((char*)(m_dataStream->GetBufferData()), m_dataStream->GetBufferReadSize());
					m_rakPeer->Send(&clientData, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_clientAddressBook[i], false);
					free(mapData);
				}
			}
			RakNet::BitStream updateStatus;
			updateStatus.Write((RakNet::MessageID)CSNetMessages::CLIENTS_UPDATE_COMPLETE);
			m_rakPeer->Send(&updateStatus, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			packet = nullptr;
			break;
		}
		default:
			ConsoleLogMessage("Unknown Connection Message Recieved");
			break;
		}
		m_rakPeer->DeallocatePacket(packet);
		packet = m_rakPeer->Receive();
	}
}

bool TestProject::ClientProcessingEvents()
{
	bool proccessingEvent = false;
	RakNet::Packet* packet = m_rakPeer->Receive();
	while (packet != nullptr)
	{
		switch (packet->data[0])
		{
		case (CSNetMessages::SERVER_UPDATED):
		{
			proccessingEvent = true;
			//Recieve Update awknowledgement from the server
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(rs);
			ConsoleLogMessage(rs.C_String());

			RakNet::BitStream serverFetch;
			serverFetch.Write((RakNet::MessageID)CSNetMessages::UPDATE_CLIENTS);
			m_rakPeer->Send(&serverFetch, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_serverAddress, false);
			packet = nullptr;
			break;
		}
		case (CSNetMessages::FETCHED_SERVER):
		{
			proccessingEvent = true;
			ConsoleLogMessage("Fetched server data");
			//recieve lobby player info
			m_serverAddress = packet->systemAddress;
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			unsigned int bytesToRead = packet->length - sizeof(RakNet::MessageID);
			char* playerData = (char*)malloc(bytesToRead);
			bsIn.Read(playerData, bytesToRead);

			//Deserialise this recieved player data
			Player player;
			player.Deserialise((void**)(&playerData));
			ConsoleLogMessage("Recieved updated Data for:");
			ConsoleLogMessage(player.GetName());
			//place player into lobby array 
			if (player.GetPlayerIDNum() != 0)
			{
				m_gameplayLobby[player.GetPlayerIDNum() - 1] = player;
			}			
			packet = nullptr;
			free(playerData);		
			break;
		}
		case (CSNetMessages::CLIENTS_UPDATE_COMPLETE):
		{
			proccessingEvent = true;
			packet = nullptr;
			break;
		}
		case (CSNetMessages::SERVER_UPDATED_MAP):
		{
			proccessingEvent = true;
			//Recieve Update awknowledgement from the server
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(rs);
			ConsoleLogMessage(rs.C_String());

			RakNet::BitStream serverFetch;
			serverFetch.Write((RakNet::MessageID)CSNetMessages::UPDATE_CLIENTS_MAP);
			m_rakPeer->Send(&serverFetch, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_serverAddress, false);
			packet = nullptr;
			break;
		}
		case (CSNetMessages::FETCHED_MAP):
		{
			proccessingEvent = true;
			m_serverAddress = packet->systemAddress;
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			unsigned int bytesToRead = packet->length - sizeof(RakNet::MessageID);
			char* mapData = (char*)malloc(bytesToRead);
			bsIn.Read(mapData, bytesToRead);
			//Deserialize and Store Map object
			m_loadedMap.Deserialise((void**)(&mapData));
			ConsoleLogMessage("Map Saved to client");
			packet = nullptr;
			free(mapData);
			break;
		}
		default:
			break;
		}
	}
	return proccessingEvent;
}

void TestProject::UpdateServer()
{
	//Send update to server
	char* playerData = nullptr;
	unsigned int playerSize = m_gameplayLobby[m_thisClientsPlayerNumber - 1].Serialise((void**)(&playerData));
	m_dataStream->Clear();
	m_dataStream->Write(playerData, playerSize);
	RakNet::BitStream connectionAuthorise;
	connectionAuthorise.Write((RakNet::MessageID)CSNetMessages::UPDATE_SERVER);
	connectionAuthorise.Write((char*)(m_dataStream->GetBufferData()), m_dataStream->GetBufferReadSize());
	m_rakPeer->Send(&connectionAuthorise, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_serverAddress, false);
	free(playerData);
}

void TestProject::RequestMap()
{
	RakNet::BitStream connectionAuthorise;
	connectionAuthorise.Write((RakNet::MessageID)CSNetMessages::UPDATE_CLIENTS_MAP);
	m_rakPeer->Send(&connectionAuthorise, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_serverAddress, false);
}

void TestProject::UpdateMap()
{
	//Send update to server
	char* mapData = nullptr;
	unsigned int mapSize = m_loadedMap.Serialise((void**)(&mapData));
	m_dataStream->Clear();
	m_dataStream->Write(mapData, mapSize);
	RakNet::BitStream connectionAuthorise;
	connectionAuthorise.Write((RakNet::MessageID)CSNetMessages::UPDATE_SERVER_MAP);
	connectionAuthorise.Write((char*)(m_dataStream->GetBufferData()), m_dataStream->GetBufferReadSize());
	m_rakPeer->Send(&connectionAuthorise, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_serverAddress, false);
	free(mapData);
}

void TestProject::UpdateBomb()
{
	//Blow up the bomb
	if (m_bombTimer <= 0)
	{
		m_bombExploded = true;
	}
	m_bombTimer -= Utility::getDeltaTime();
}

void TestProject::BombReset()
{
	m_bombPlaced = false;
	m_bombExploded = false;
	m_bombTimer = 0;
	m_playerBombPos.x = 0.f;
	m_playerBombPos.y = 0.f;
}

bool TestProject::LoadMap(const char* a_filename)
{
	std::fstream levelFile;
	levelFile.open(a_filename, std::ios_base::in | std::ios_base::binary);
	//test to see if the file has opened correctly
	if (levelFile.is_open())
	{
		levelFile.ignore(std::numeric_limits<std::streamsize>::max());
		std::streamsize fileSize = levelFile.gcount();
		levelFile.clear();
		levelFile.seekg(0, std::ios_base::beg);
		if (fileSize == 0)
		{
			levelFile.close();
			return false;
		}
		int mapFileData[24][24];
		for (int row = 0; row < 24; row++) {  // stop loops if nothing to read
			for (int column = 0; column < 24; column++) {
				levelFile >> mapFileData[row][column];
			}
		}
		
		m_loadedMap.SetMapData(mapFileData);
		return true;
	}
	return false;
}
