#pragma once
#include "GameServer.h"
#include "Database.h"
class Lobby
{
private:
	std::string m_id;
	std::vector<UserModel> m_Users;
	Game* m_game = nullptr;
	Database* m_db;
	static std::string GenerateRandomId(size_t length = 8);
public:
	Lobby(Database* db);
	void JoinLobby(std::string playerName);
};

