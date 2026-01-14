#pragma once
#include "GameServer.h"
#include "Database.h"
#include <memory>

constexpr size_t LOBBY_PASS_LENGTH = 4;

enum class LobbyStatus {
	Waiting,
	Started
};

class Lobby
{
private:
	std::string m_id;
	std::string m_name;
	std::string m_password;
	std::vector<UserModel> m_Users;
	int m_ownerId = -1;
	Database* m_db;
	LobbyStatus m_status = LobbyStatus::Waiting;
	std::unique_ptr<Game> m_game = nullptr;
	
	static std::string GenerateRandomId(size_t length = LOBBY_PASS_LENGTH);

public:
	Lobby(Database* db, const std::string& name = "New Lobby", int maxPlayers = 4, const std::string& password = "");
	
	// Core methods
	void JoinLobby(int userId);
	void JoinLobby(const std::string& playerName);
	void LeaveLobby(int userId);
	void Start();
	
	// Getters
	LobbyStatus GetStatus() const;
	const std::string& GetId() const;
	const std::string& GetName() const;
	int GetMaxPlayers() const;
	int GetCurrentPlayers() const;
	int GetOwnerId() const;
	const std::vector<UserModel>& GetUsers() const;
	bool HasPassword() const;
	bool CheckPassword(const std::string& pwd) const;
	Game* GetGame();
	bool IsStarted() const;
	
	// Utility
	bool IsOwner(int userId) const;
	bool IsUserInLobby(int userId) const;
	std::vector<int> GetPlayerIds() const;
};

