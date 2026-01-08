#include "NetworkManager.h"
#include <crow/json.h>

NetworkManager::NetworkManager(const std::string& serverUrl, QObject* parent)
    : QObject(parent), baseUrl(serverUrl) {
    
    // Game WebSocket connections
    connect(&m_webSocket, &QWebSocket::connected, this, &NetworkManager::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &NetworkManager::onTextMessageReceived);
    
    // Lobby WebSocket connections
    connect(&m_lobbyWebSocket, &QWebSocket::connected, this, &NetworkManager::onLobbyConnected);
    connect(&m_lobbyWebSocket, &QWebSocket::disconnected, this, &NetworkManager::onLobbyDisconnected);
    connect(&m_lobbyWebSocket, &QWebSocket::textMessageReceived, this, &NetworkManager::onLobbyTextMessageReceived);
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

LobbyResponse NetworkManager::createLobby(int user_id, const std::string& name, int max_players, const std::string& password){
    crow::json::wvalue payload;
    payload["user_id"] = user_id;
    payload["name"] = name;
    payload["max_players"] = max_players; 

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

bool NetworkManager::startGame(const std::string& lobby_id) {
    crow::json::wvalue payload;
    
    auto response = cpr::Post(
        cpr::Url{ baseUrl + "/api/lobby/" + lobby_id + "/start" },
        cpr::Body{ payload.dump() },
        cpr::Header{ {"Content-Type", "application/json"} }
    );

    return response.status_code == 200;
}

bool NetworkManager::leaveLobby(int user_id, const std::string& lobby_id) {
    crow::json::wvalue payload;
    payload["user_id"] = user_id;
    payload["lobby_id"] = lobby_id;
    
    auto response = cpr::Post(
        cpr::Url{ baseUrl + "/api/lobby/leave" },
        cpr::Body{ payload.dump() },
        cpr::Header{ {"Content-Type", "application/json"} }
    );

    return response.status_code == 200;
}

std::vector<NetworkManager::PlayerInfo> NetworkManager::getLobbyPlayers(const std::string& lobby_id) {
    auto response = cpr::Get(
        cpr::Url{ baseUrl + "/api/lobby/" + lobby_id + "/players" }
    );

    std::vector<PlayerInfo> players;
    
    if (response.status_code == 200) {
        auto data = crow::json::load(response.text);
        if (data.has("players")) {
            auto players_array = data["players"];
            for (size_t i = 0; i < players_array.size(); i++) {
                auto player = players_array[i];
                PlayerInfo info;
                info.user_id = static_cast<int>(player["user_id"].i());
                info.username = player["username"].s();
                info.is_host = player["is_host"].b();
                players.push_back(info);
            }
        }
    }

    return players;
}

void NetworkManager::connectToGame(const std::string& lobby_id, int user_id) {
    m_gameLobbyId = lobby_id;
    m_gameUserId = user_id;

    QString urlStr = QString::fromStdString(baseUrl);
    if (urlStr.startsWith("http")) {
        urlStr.replace("http", "ws");
    } else {
        urlStr.prepend("ws://");
    }
    urlStr += "/ws/game";
    
    qDebug() << "Connecting to Game WebSocket: " << urlStr;
    m_webSocket.open(QUrl(urlStr));
}

void NetworkManager::sendGameAction(const QJsonObject& action) {
    if (m_webSocket.isValid()) {
        QJsonDocument doc(action);
        m_webSocket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    }
}

void NetworkManager::onConnected() {
    qDebug() << "WebSocket Connected!";
    
    if (!m_gameLobbyId.empty() && m_gameUserId != -1) {
        QJsonObject joinMsg;
        joinMsg["type"] = "join_game";
        joinMsg["lobby_id"] = QString::fromStdString(m_gameLobbyId);
        joinMsg["user_id"] = m_gameUserId;
        
        QJsonDocument doc(joinMsg);
        m_webSocket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    }

    emit gameConnected();
}

void NetworkManager::onTextMessageReceived(const QString& message) {
    qDebug() << "Message Received: " << message;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        emit gameMessageReceived(doc.object());
    }
}

void NetworkManager::onDisconnected() {
    qDebug() << "Game WebSocket Disconnected";
    emit gameDisconnected();
}

void NetworkManager::connectToLobby(const std::string& lobby_id) {
    QString urlStr = QString::fromStdString(baseUrl);
    if (urlStr.startsWith("http")) {
        urlStr.replace("http", "ws");
    } else {
        urlStr.prepend("ws://");
    }
    urlStr += "/ws/lobby";
    
    qDebug() << "Connecting to Lobby WebSocket: " << urlStr;
    
    m_currentLobbyId = lobby_id;
    
    m_lobbyWebSocket.open(QUrl(urlStr));
}

void NetworkManager::disconnectFromLobby() {
    if (m_lobbyWebSocket.isValid()) {
        m_lobbyWebSocket.close();
    }
}

void NetworkManager::onLobbyConnected() {
    qDebug() << "Lobby WebSocket Connected!";
    
    if (!m_currentLobbyId.empty()) {
        QJsonObject subscribeMsg;
        subscribeMsg["action"] = "subscribe";
        subscribeMsg["lobby_id"] = QString::fromStdString(m_currentLobbyId);
        
        QJsonDocument doc(subscribeMsg);
        m_lobbyWebSocket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
        qDebug() << "Subscribed to lobby: " << m_currentLobbyId.c_str();
    }
    
    emit lobbyConnected();
}

void NetworkManager::onLobbyTextMessageReceived(const QString& message) {
    qDebug() << "Lobby Message Received: " << message;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        emit lobbyMessageReceived(doc.object());
    }
}

void NetworkManager::onLobbyDisconnected() {
    qDebug() << "Lobby WebSocket Disconnected";
    emit lobbyDisconnected();
}
