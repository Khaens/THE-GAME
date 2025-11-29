#pragma once
#include "GameServer.h"
#include "Database.h"

enum class LobbyStatus {
	Waiting,
	Started
};

class Lobby
{
private:
	std::string m_id;
	std::vector<UserModel> m_Users;
	UserModel m_lobbyCreator;
	Database* m_db;
	LobbyStatus m_status = LobbyStatus::Waiting;
	static std::string GenerateRandomId(size_t length = 8);
public:
	Lobby(Database* db);
	void JoinLobby(std::string playerName);
	LobbyStatus GetStatus();
	void Start();
};

