#include "NetworkManager.h"
#include <crow/json.h>

NetworkManager::NetworkManager(const std::string& serverUrl, QObject* parent)
    : QObject(parent), baseUrl(serverUrl) {
    
    connect(&m_webSocket, &QWebSocket::connected, this, &NetworkManager::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &NetworkManager::onTextMessageReceived);
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

void NetworkManager::connectToGame(const std::string& lobby_id, int user_id) {
    QString urlStr = QString::fromStdString(baseUrl);
    // Simple conversion from http to ws
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
    qDebug() << "WebSocket Disconnected";
    emit gameDisconnected();
}
