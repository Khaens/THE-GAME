#pragma once
#include <crow.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <memory>
#include "Lobby.h"

// Struct for chat messages
struct ChatMessage {
    std::string lobby_id;
    std::string message_json; 
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

    // --- Helper Functions ---
    void BroadcastGameState(const std::string& lobby_id, crow::websocket::connection* targetConn = nullptr);
    void BroadcastGameStateLocked(const std::string& lobby_id, crow::websocket::connection* targetConn = nullptr);
    std::string CensorMessage(const std::string& input);
    void ChatWorker();
    
    // Helper to start the worker thread
    void StartChatWorker();
};
