#pragma once
#include <string>
#include <optional>
#include <cpr/cpr.h>
#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonDocument>

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
    std::string error_message;
};

struct LobbyStatus {
    std::string lobby_id;
    int current_players;
    int max_players;
    bool game_started;
};

class NetworkManager : public QObject {
    Q_OBJECT

private:
    std::string baseUrl;
    QWebSocket m_webSocket;

public:
    explicit NetworkManager(const std::string& serverUrl = "http://localhost:18080", QObject* parent = nullptr);

    // Auth endpoints
    RegisterResponse registerUser(const std::string& username, const std::string& password);
    LoginResponse loginUser(const std::string& username, const std::string& password);

    // Lobby endpoints
    LobbyResponse createLobby(int user_id, const std::string& name, int max_players, const std::string& password);    
    bool joinLobby(int user_id, const std::string& code);
    std::optional<LobbyStatus> getLobbyStatus(const std::string& lobby_id);
    bool startGame(const std::string& lobby_id);

    // Game WebSocket
    void connectToGame(const std::string& lobby_id, int user_id);
    void sendGameAction(const QJsonObject& action);

signals:
    void gameConnected();
    void gameDisconnected();
    void gameMessageReceived(const QJsonObject& message);

private slots:
    void onConnected();
    void onTextMessageReceived(const QString& message);
    void onDisconnected();
};

