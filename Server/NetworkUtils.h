#pragma once
#include <crow.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <memory>
#include <set>
#include "Lobby.h"

// Struct for chat messages
struct ChatMessage {
    std::string lobby_id;
    std::string message_json; 
};

// Struct for ALL websocket messages (unified queue)
struct WsMessage {
    crow::websocket::connection* conn;
    std::string message;
};

class NetworkUtils {
public:
    NetworkUtils();
    ~NetworkUtils() = default;

    // --- Global State wrappers ---
    
    // Core game state
    std::unordered_map<std::string, std::unique_ptr<Lobby>> lobbies;
    std::mutex lobby_mutex;

    // WebSocket connections for game
    std::unordered_map<std::string, std::vector<crow::websocket::connection*>> lobby_connections;
    std::mutex ws_mutex;

    // WebSocket connections for lobby updates
    std::unordered_map<std::string, std::vector<crow::websocket::connection*>> lobby_update_connections;
    std::mutex lobby_ws_mutex;

    // Chat Queue
    std::queue<ChatMessage> chatQueue;
    std::mutex chatMutex;
    std::condition_variable chatCv;

    std::list<std::string> m_pendingMessages;
    std::mutex m_pendingMsgMutex;

    std::queue<WsMessage> wsQueue;
    std::mutex wsSendMutex;
    std::condition_variable wsSendCv;
    bool m_wsWorkerRunning = true;
    
    // Track valid connections to avoid sending to closed ones
    std::set<crow::websocket::connection*> m_validConnections;
    std::mutex m_validConnMutex;

    // --- Helper Functions ---
    void BroadcastGameState(const std::string& lobby_id, crow::websocket::connection* targetConn = nullptr);
    void BroadcastGameStateLocked(const std::string& lobby_id, crow::websocket::connection* targetConn = nullptr);
    std::string CensorMessage(const std::string& input);

    void ChatWorker();
    
    void StartChatWorker();
    void StartWsWorker(); 
    void WsWorker(); 
    
    void SafeSendText(crow::websocket::connection* conn, const std::string& msg);
    void CleanupPendingMessages();
};
