#include "NetworkManager.h"
#include <crow/json.h>

NetworkManager::NetworkManager(const std::string& serverUrl)
    : baseUrl(serverUrl) {
}

RegisterResponse NetworkManager::registerUser(const std::string& username, const std::string& password) {
    crow::json::wvalue payload;
    payload["username"] = username;
    payload["password"] = password;

    auto response = cpr::Post(
        cpr::Url{ baseUrl + "/api/register" },
        cpr::Body{ payload.dump() },
        cpr::Header{ {"Content-Type", "application/json"} }
    );

    auto data = crow::json::load(response.text);
    if (response.status_code == 201) {
        return RegisterResponse{
            true,
            static_cast<int>(data["user_id"].i()),
            ""
        };
    }

    return RegisterResponse{
        false,
        -1,
        data["error"].s()
    };
}

LoginResponse NetworkManager::loginUser(const std::string& username, const std::string& password) {
    crow::json::wvalue payload;
    payload["username"] = username;
    payload["password"] = password;

    auto response = cpr::Post(
        cpr::Url{ baseUrl + "/api/login" },
        cpr::Body{ payload.dump() },
        cpr::Header{ {"Content-Type", "application/json"} }
    );

    auto data = crow::json::load(response.text);
    if (response.status_code == 200) {
        return LoginResponse{
            true,
            static_cast<int>(data["user_id"].i()),
            data["username"].s(),
            ""
        };
    }

    return LoginResponse{
        false,
        -1,
        "",
        data["error"].s()
    };
}

LobbyResponse NetworkManager::createLobby(int user_id) {
    crow::json::wvalue payload;
    payload["user_id"] = user_id;

    auto response = cpr::Post(
        cpr::Url{ baseUrl + "/api/lobby/create" },
        cpr::Body{ payload.dump() },
        cpr::Header{ {"Content-Type", "application/json"} }
    );

    auto data = crow::json::load(response.text);
    if (response.status_code == 201) {
        return LobbyResponse{
            true,
            data["lobby_id"].s(),
            static_cast<int>(data["max_players"].i()),
            static_cast<int>(data["current_players"].i())
        };
    }

    return LobbyResponse{ false, "", 0, 0 };
}

bool NetworkManager::joinLobby(int user_id, const std::string& lobby_id) {
    crow::json::wvalue payload;
    payload["user_id"] = user_id;
    payload["lobby_id"] = lobby_id;

    auto response = cpr::Post(
        cpr::Url{ baseUrl + "/api/lobby/join" },
        cpr::Body{ payload.dump() },
        cpr::Header{ {"Content-Type", "application/json"} }
    );

    return response.status_code == 200;
}

std::optional<LobbyStatus> NetworkManager::getLobbyStatus(const std::string& lobby_id) {
    auto response = cpr::Get(
        cpr::Url{ baseUrl + "/api/lobby/" + lobby_id + "/status" }
    );

    if (response.status_code == 200) {
        auto data = crow::json::load(response.text);
        return LobbyStatus{
            data["lobby_id"].s(),
            static_cast<int>(data["current_players"].i()),
            static_cast<int>(data["max_players"].i()),
            data["game_started"].b()
        };
    }

    return std::nullopt;
}
