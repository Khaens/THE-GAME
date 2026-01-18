#pragma once
#include <string>
#include <optional>
#include <cpr/cpr.h>
#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>

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
    std::string name;
    int remaining_seconds;
};

class NetworkManager : public QObject {
    Q_OBJECT

private:
    std::string baseUrl;
    QWebSocket m_webSocket; 
    QWebSocket m_lobbyWebSocket;  
    std::string m_currentLobbyId;  
    std::string m_gameLobbyId;
    int m_gameUserId = -1;

public:
    explicit NetworkManager(const std::string& serverUrl = "http://localhost:18080", QObject* parent = nullptr);

    // Auth endpoints
    RegisterResponse registerUser(const std::string& username, const std::string& password);
    LoginResponse loginUser(const std::string& username, const std::string& password);

    // Lobby endpoints
    LobbyResponse createLobby(int user_id, const std::string& name, int max_players, const std::string& password);    
    LobbyResponse joinLobby(int user_id, const std::string& code);
    std::optional<LobbyStatus> getLobbyStatus(const std::string& lobby_id);
    QString startGame(const std::string& lobby_id);
    bool leaveLobby(int user_id, const std::string& lobby_id);

    // Achievements
    struct AchievementsData {
        bool success;
        QJsonObject achievements; // key: achievement_name, value: bool unlocked
        std::string error;
    };
    AchievementsData getAchievements(int user_id);

    // Profile Picture
    bool uploadProfilePicture(int user_id, const QByteArray& data);
    QByteArray getProfilePicture(int user_id);
    bool hasProfilePicture(int user_id);
    bool deleteProfilePicture(int user_id);
    
    struct PlayerInfo {
        int user_id;
        std::string username;
        bool is_host;
    };
    std::vector<PlayerInfo> getLobbyPlayers(const std::string& lobby_id);

    // Game WebSocket
    void connectToGame(const std::string& lobby_id, int user_id);
    void sendGameAction(const QJsonObject& action);

    // Lobby WebSocket
    void connectToLobby(const std::string& lobby_id);
    void disconnectFromLobby();

signals:
    void gameConnected();
    void gameDisconnected();
    void gameMessageReceived(const QJsonObject& message);
    
    // Lobby WebSocket signals
    void lobbyConnected();
    void lobbyDisconnected();
    void lobbyMessageReceived(const QJsonObject& message);

    void serverStatusChanged(bool isOnline);

private slots:
    void onConnected();
    void onTextMessageReceived(const QString& message);
    void onDisconnected();
    
    // Lobby WebSocket slots
    void onLobbyConnected();
    void onLobbyTextMessageReceived(const QString& message);
    void onLobbyDisconnected();

private slots:
    void checkServerStatus();

private:
    QTimer* m_serverCheckTimer;
    bool m_serverAvailable = false;
};

