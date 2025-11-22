#pragma once
#include <string>
#include <optional>
#include <cpr/cpr.h>

struct LoginResponse {
    bool success;
    int user_id;
    std::string username;
    std::string error;
};

struct RegisterResponse {
    bool success;
    int user_id;
    std::string error;
};

struct LobbyResponse {
    bool success;
    std::string lobby_id;
    int max_players;
    int current_players;
};

struct LobbyStatus {
    std::string lobby_id;
    int current_players;
    int max_players;
    bool game_started;
};

class NetworkManager {
private:
    std::string baseUrl;

public:
    NetworkManager(const std::string& serverUrl = "http://localhost:18080");

    // Auth endpoints
    RegisterResponse registerUser(const std::string& username, const std::string& password);
    LoginResponse loginUser(const std::string& username, const std::string& password);

    // Lobby endpoints
    LobbyResponse createLobby(int user_id);
    bool joinLobby(int user_id, const std::string& lobby_id);
    std::optional<LobbyStatus> getLobbyStatus(const std::string& lobby_id);
};

