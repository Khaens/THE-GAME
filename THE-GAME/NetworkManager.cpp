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

    // Auto-reconnect / Server Check Timer
    m_serverCheckTimer = new QTimer(this);
    connect(m_serverCheckTimer, &QTimer::timeout, this, &NetworkManager::checkServerStatus);
    m_serverCheckTimer->start(2000); 
}

void NetworkManager::checkServerStatus() {
    auto response = cpr::Head(
        cpr::Url{ baseUrl + "/" },
        cpr::Timeout{ 1000 } 
    );

    bool isOnline = (response.status_code != 0);

    if (isOnline && !m_serverAvailable) {
        qDebug() << "Server is now ONLINE!";
        m_serverAvailable = true;
        emit serverStatusChanged(true);
    }
    else if (!isOnline && m_serverAvailable) {
        qDebug() << "Server went OFFLINE!";
        m_serverAvailable = false;
        emit serverStatusChanged(false);
    }
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

    if (response.status_code == 0) {
       return RegisterResponse{ false, -1, "Server invalid/offline" };
    }

    auto data = crow::json::load(response.text);
    if (!data) {
        return RegisterResponse{ false, -1, "Invalid response from server" };
    }

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

    if (response.status_code == 0) {
       return LoginResponse{ false, -1, "", "Server invalid/offline" };
    }

    auto data = crow::json::load(response.text);
    if (!data) {
        return LoginResponse{ false, -1, "", "Invalid response from server" };
    }

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

    if (response.status_code == 0) {
        return LobbyResponse{ false, "", 0, 0, "Server invalid/offline" };
    }

    auto data = crow::json::load(response.text);
    if (!data) {
        return LobbyResponse{ false, "", 0, 0, "Invalid response" };
    }

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

LobbyResponse NetworkManager::joinLobby(int user_id, const std::string& lobby_id) {
    crow::json::wvalue payload;
    payload["user_id"] = user_id;
    payload["lobby_id"] = lobby_id;

    auto response = cpr::Post(
        cpr::Url{ baseUrl + "/api/lobby/join" },
        cpr::Body{ payload.dump() },
        cpr::Header{ {"Content-Type", "application/json"} }
    );

    if (response.status_code == 0) {
        return LobbyResponse{ false, "", 0, 0, "Server invalid/offline" };
    }

    auto data = crow::json::load(response.text);
    if (!data) {
        return LobbyResponse{ false, "", 0, 0, "Invalid response" };
    }

    if (response.status_code == 200) {
        return LobbyResponse{
            true,
            data["lobby_id"].s(),
            static_cast<int>(data["max_players"].i()),
            static_cast<int>(data["current_players"].i()),
            ""
        };
    }

    return LobbyResponse{
        false,
        "",
        0,
        0,
        data.has("error_message") ? (std::string)data["error_message"].s() : std::string("Failed to join")
    };
}

std::optional<LobbyStatus> NetworkManager::getLobbyStatus(const std::string& lobby_id) {
    auto response = cpr::Get(
        cpr::Url{ baseUrl + "/api/lobby/" + lobby_id + "/status" }
    );

    if (response.status_code == 0) return std::nullopt;

    if (response.status_code == 200) {
        auto data = crow::json::load(response.text);
        if (!data) return std::nullopt;

        LobbyStatus status;
        status.lobby_id = data["lobby_id"].s();
        status.current_players = static_cast<int>(data["current_players"].i());
        status.max_players = static_cast<int>(data["max_players"].i());
        status.game_started = data["game_started"].b();
        
        if (data.has("name")) status.name = data["name"].s();
        if (data.has("remaining_seconds")) status.remaining_seconds = static_cast<int>(data["remaining_seconds"].i());
        else status.remaining_seconds = 60; 

        return status;
    }

    return std::nullopt;
}

QString NetworkManager::startGame(const std::string& lobby_id) {
    crow::json::wvalue payload;
    
    auto response = cpr::Post(
        cpr::Url{ baseUrl + "/api/lobby/" + lobby_id + "/start" },
        cpr::Body{ payload.dump() },
        cpr::Header{ {"Content-Type", "application/json"} }
    );

    if (response.status_code == 0) return "Server invalid/offline";
    
    if (response.status_code == 200) return "";

    auto data = crow::json::load(response.text);
    if (!data) return "Invalid response from server";
    
    if (data.has("error_message")) {
        return QString::fromStdString(data["error_message"].s());
    }
    
    return "Failed to start game (Unknown Error)";
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

    if (response.status_code == 0) return false;
    return response.status_code == 200;
}

NetworkManager::AchievementsData NetworkManager::getAchievements(int user_id) {
    auto response = cpr::Get(
        cpr::Url{ baseUrl + "/api/user/" + std::to_string(user_id) + "/achievements" }
    );

    if (response.status_code == 0) {
        return { false, {}, "Server invalid/offline" };
    }

    if (response.status_code == 200) {
        auto data = crow::json::load(response.text);
        if (!data) return { false, {}, "Invalid response" };

        QJsonObject achObj;
        std::vector<std::string> keys = data.keys();
        for(const auto& key : keys) {
            if (key == "success" || key == "error") continue;
            achObj[QString::fromStdString(key)] = data[key].b();
        }
        
        return { true, achObj, "" };
    }

    return { false, {}, "Failed to fetch achievements" };
}

NetworkManager::StatisticsData NetworkManager::getStatistics(int user_id) {
    auto response = cpr::Get(
        cpr::Url{ baseUrl + "/api/user/" + std::to_string(user_id) + "/statistics" }
    );

    if (response.status_code == 0) {
        return { false, 0.0f, "Server invalid/offline" };
    }

    if (response.status_code == 200) {
        auto data = crow::json::load(response.text);
        if (!data) return { false, 0.0f, "Invalid response" };

        if (data.has("success") && data["success"].b() && data.has("performance_score")) {
            float score = static_cast<float>(data["performance_score"].d());
            return { true, score, "" };
        }
    }

    return { false, 0.0f, "Failed to fetch statistics" };
}

NetworkManager::PlaytimeData NetworkManager::getPlaytime(int user_id) {
    auto response = cpr::Get(
        cpr::Url{ baseUrl + "/api/user/" + std::to_string(user_id) + "/playtime" }
    );

    if (response.status_code == 0) {
        return { false, 0.0f, "Server invalid/offline" };
    }

    if (response.status_code == 200) {
        auto data = crow::json::load(response.text);
        if (!data) return { false, 0.0f, "Invalid response" };

        if (data.has("success") && data["success"].b() && data.has("hours")) {
            float hours = static_cast<float>(data["hours"].d());
            return { true, hours, "" };
        }
    }

    return { false, 0.0f, "Failed to fetch playtime" };
}

bool NetworkManager::uploadProfilePicture(int user_id, const QByteArray& data) {
    std::string dataStr(data.constData(), data.length());

    auto response = cpr::Post(
        cpr::Url{ baseUrl + "/api/user/" + std::to_string(user_id) + "/profile-picture" },
        cpr::Body{ dataStr },
        cpr::Header{ {"Content-Type", "application/octet-stream"} }
    );

    if (response.status_code == 0) return false;
    return response.status_code == 200;
}

QByteArray NetworkManager::getProfilePicture(int user_id) {
    auto response = cpr::Get(
        cpr::Url{ baseUrl + "/api/user/" + std::to_string(user_id) + "/profile-picture" }
    );

    if (response.status_code == 0) return QByteArray();

    if (response.status_code == 200) {
        return QByteArray(response.text.c_str(), response.text.length());
    }

    return QByteArray();
}

bool NetworkManager::hasProfilePicture(int user_id) {
    auto response = cpr::Get(
        cpr::Url{ baseUrl + "/api/user/" + std::to_string(user_id) + "/has-profile-picture" }
    );

    if (response.status_code == 0) return false;

    if (response.status_code == 200) {
        auto data = crow::json::load(response.text);
        if (data && data.has("has_image")) {
            return data["has_image"].b();
        }
    }
    return false;
}

bool NetworkManager::deleteProfilePicture(int user_id) {
    auto response = cpr::Delete(
        cpr::Url{ baseUrl + "/api/user/" + std::to_string(user_id) + "/profile-picture" }
    );
    if (response.status_code == 0) return false;
    return response.status_code == 200;
}

std::vector<NetworkManager::PlayerInfo> NetworkManager::getLobbyPlayers(const std::string& lobby_id) {
    auto response = cpr::Get(
        cpr::Url{ baseUrl + "/api/lobby/" + lobby_id + "/players" }
    );

    std::vector<PlayerInfo> players;
    
    if (response.status_code == 0) return players;

    if (response.status_code == 200) {
        auto data = crow::json::load(response.text);
        if (!data) return players;

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
