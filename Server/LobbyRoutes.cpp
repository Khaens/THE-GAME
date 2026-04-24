#include "LobbyRoutes.h"
#include <iostream>
#include <algorithm>
#include <string>

LobbyRoutes::LobbyRoutes(crow::SimpleApp& app, Database* db, NetworkUtils& networkUtils) {
    RegisterRoutes(app, db, networkUtils);
}

void LobbyRoutes::RegisterRoutes(crow::SimpleApp& app, Database* db, NetworkUtils& networkUtils) {
    
    // CREATE LOBBY
    CROW_ROUTE(app, "/api/lobby/create")
        .methods(crow::HTTPMethod::POST)
        ([db, &networkUtils](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        int user_id = body["user_id"].i();
        std::string name = body.has("name") ? (std::string)body["name"].s() : std::string("New Lobby");
        int max_players = body.has("max_players") ? body["max_players"].i() : 4;
        std::string password = body.has("password") ? (std::string)body["password"].s() : std::string("");

        std::lock_guard<std::mutex> lock(networkUtils.lobby_mutex);
        
        auto new_lobby = std::make_unique<Lobby>(db, name, max_players, password);
        
        try {
            new_lobby->JoinLobby(user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = e.what();
            return crow::response(400, response);
        }
        
        std::string id = new_lobby->GetId();
        networkUtils.lobbies[id] = std::move(new_lobby);

        crow::json::wvalue response;
        response["success"] = true;
        response["lobby_id"] = id;
        response["max_players"] = max_players;
        response["current_players"] = 1;
        return crow::response(201, response);
    });

    // JOIN LOBBY
    CROW_ROUTE(app, "/api/lobby/join")
        .methods(crow::HTTPMethod::POST)
        ([db, &networkUtils](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        int user_id = body["user_id"].i();
        std::string lobby_id = body["lobby_id"].s();
        
        std::lock_guard<std::mutex> lock(networkUtils.lobby_mutex);
        if (networkUtils.lobbies.find(lobby_id) == networkUtils.lobbies.end()) {
             crow::json::wvalue response;
             response["success"] = false;
             response["error_message"] = "Lobby not found";
             return crow::response(404, response);
        }

        Lobby& lobby = *networkUtils.lobbies[lobby_id];
        
        if (lobby.IsUserInLobby(user_id)) {
            crow::json::wvalue response;
            response["success"] = true;
            response["lobby_id"] = std::string(lobby.GetId());
            response["max_players"] = lobby.GetMaxPlayers();
            response["current_players"] = lobby.GetCurrentPlayers();
            return crow::response(200, response);
        }
        
        try {
            lobby.JoinLobby(user_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = e.what();
            return crow::response(409, response);
        }
        
        std::lock_guard<std::mutex> ws_lock(networkUtils.lobby_ws_mutex);
        crow::json::wvalue update;
        update["type"] = "player_joined";
        update["lobby_id"] = std::string(lobby.GetId());
        update["user_id"] = user_id;
        update["current_players"] = lobby.GetCurrentPlayers();
        update["max_players"] = lobby.GetMaxPlayers();
        
        std::string username = "Player_" + std::to_string(user_id);
        try {
            UserModel user = db->GetUserById(user_id);
            username = user.GetUsername();
        } catch (...) {
			username = "Player_" + std::to_string(user_id);
        }
        update["username"] = username;
        
        std::string update_msg = update.dump();
        for (auto* conn : networkUtils.lobby_update_connections[lobby_id]) {
            networkUtils.SafeSendText(conn, update_msg);
        }

        crow::json::wvalue response;
        response["success"] = true;
        response["lobby_id"] = std::string(lobby.GetId());
        response["max_players"] = lobby.GetMaxPlayers();
        response["current_players"] = lobby.GetCurrentPlayers();
        return crow::response(200, response);
    });

    // LOBBY STATUS
	CROW_ROUTE(app, "/api/lobby/<string>/status")
        .methods(crow::HTTPMethod::GET)
        ([&networkUtils](const std::string& lobby_id) {
        
        std::lock_guard<std::mutex> lock(networkUtils.lobby_mutex);
        if (networkUtils.lobbies.find(lobby_id) == networkUtils.lobbies.end()) {
            return crow::response(404, "Lobby not found");
        }

        const Lobby& lobby = *networkUtils.lobbies[lobby_id];

        crow::json::wvalue response;
        response["lobby_id"] = std::string(lobby.GetId());
        response["current_players"] = lobby.GetCurrentPlayers(); 
        response["max_players"] = lobby.GetMaxPlayers();
        response["game_started"] = lobby.IsStarted();
        response["name"] = std::string(lobby.GetName());
        auto duration = std::chrono::steady_clock::now() - lobby.GetRoundStartTime();
        int elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        int remaining = std::max(0, 60 - elapsed_seconds);
        response["remaining_seconds"] = remaining;
        return crow::response(200, response);
    });

    // LOBBY PLAYERS
    CROW_ROUTE(app, "/api/lobby/<string>/players")
        .methods(crow::HTTPMethod::GET)
        ([&networkUtils](const std::string& lobby_id) {
        
        std::lock_guard<std::mutex> lock(networkUtils.lobby_mutex);
        if (networkUtils.lobbies.find(lobby_id) == networkUtils.lobbies.end()) {
            return crow::response(404, "Lobby not found");
        }

        const Lobby& lobby = *networkUtils.lobbies[lobby_id];

        crow::json::wvalue response;
        response["lobby_id"] = std::string(lobby.GetId());
        
        int player_index = 0;
        for (const auto& user : lobby.GetUsers()) {
            response["players"][player_index]["user_id"] = user.GetId();
            response["players"][player_index]["username"] = std::string(user.GetUsername());
            response["players"][player_index]["is_host"] = lobby.IsOwner(user.GetId());
            player_index++;
        }
        return crow::response(200, response);
    });

    // START LOBBY
    CROW_ROUTE(app, "/api/lobby/<string>/start")
        .methods(crow::HTTPMethod::POST)
        ([&networkUtils](const crow::request& req, const std::string& lobby_id) {
		
        std::lock_guard<std::mutex> lock(networkUtils.lobby_mutex);
        if (networkUtils.lobbies.find(lobby_id) == networkUtils.lobbies.end()) {
            return crow::response(404, "Lobby not found");
        }

        Lobby& lobby = *networkUtils.lobbies[lobby_id];
        
        try {
            lobby.Start();
            std::cout << "Lobby " << lobby_id << " started!" << std::endl;
            networkUtils.BroadcastGameStateLocked(lobby_id);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = e.what();
            return crow::response(400, response);
        }
        
        std::lock_guard<std::mutex> ws_lock(networkUtils.lobby_ws_mutex);
        crow::json::wvalue update;
        update["type"] = "game_started";
        update["lobby_id"] = lobby_id;
        std::string update_msg = update.dump();
        for (auto* conn : networkUtils.lobby_update_connections[lobby_id]) {
            networkUtils.SafeSendText(conn, update_msg);
        }
        
        crow::json::wvalue response;
        response["success"] = true;
        return crow::response(200, response);
    });

    // LEAVE LOBBY
    CROW_ROUTE(app, "/api/lobby/leave")
        .methods(crow::HTTPMethod::POST)
        ([db, &networkUtils](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        int user_id = body["user_id"].i();
        std::string lobby_id = body["lobby_id"].s();

        std::lock_guard<std::mutex> lock(networkUtils.lobby_mutex);
        if (networkUtils.lobbies.find(lobby_id) == networkUtils.lobbies.end()) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = "Lobby not found";
            return crow::response(404, response);
        }

        Lobby& lobby = *networkUtils.lobbies[lobby_id];

        if (!lobby.IsUserInLobby(user_id)) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = "User not in lobby";
            return crow::response(400, response);
        }

        if (lobby.IsOwner(user_id)) {
            std::lock_guard<std::mutex> ws_lock(networkUtils.lobby_ws_mutex);
            
            crow::json::wvalue update;
            update["type"] = "lobby_closed";
            update["lobby_id"] = lobby_id;
            update["reason"] = "Host left the lobby";
            std::string update_msg = update.dump();
            
            for (auto* conn : networkUtils.lobby_update_connections[lobby_id]) {
                networkUtils.SafeSendText(conn, update_msg);
            }
          
            {
                std::lock_guard<std::mutex> game_ws_lock(networkUtils.ws_mutex);
                if (networkUtils.lobby_connections.count(lobby_id)) {
                    for (auto* conn : networkUtils.lobby_connections[lobby_id]) {
                        networkUtils.SafeSendText(conn, update_msg);
                    }
                    networkUtils.lobby_connections.erase(lobby_id);
                }
            }

            networkUtils.lobby_update_connections[lobby_id].clear();
            networkUtils.lobbies.erase(lobby_id);
            
            std::cout << "Lobby " << lobby_id << " deleted (host left)" << std::endl;
            
            crow::json::wvalue response;
            response["success"] = true;
            response["lobby_deleted"] = true;
            return crow::response(200, response);
        }
        else {
            lobby.LeaveLobby(user_id);
            
            // Explicitly remove the user's connection from game connections if they are leaving manually
            {
                std::lock_guard<std::mutex> game_ws_lock(networkUtils.ws_mutex);
                if (networkUtils.lobby_connections.count(lobby_id)) {
                    auto& conns = networkUtils.lobby_connections[lobby_id];
                    std::lock_guard<std::mutex> state_lock(networkUtils.m_clientStatesMutex);
                    conns.erase(std::remove_if(conns.begin(), conns.end(), [&](crow::websocket::connection* c){
                        if (networkUtils.m_clientStates.count(c)) {
                            return networkUtils.m_clientStates[c].user_id == user_id;
                        }
                        return false;
                    }), conns.end());
                }
            }

            std::lock_guard<std::mutex> ws_lock(networkUtils.lobby_ws_mutex);
            
            std::string username = "Player_" + std::to_string(user_id);
            try {
                UserModel user = db->GetUserById(user_id);
                username = std::string(user.GetUsername());
            } catch (...) {}
            
            crow::json::wvalue update;
            update["type"] = "player_left";
            update["lobby_id"] = lobby_id;
            update["user_id"] = user_id;
            update["username"] = username;
            update["current_players"] = lobby.GetCurrentPlayers();
            update["max_players"] = lobby.GetMaxPlayers();
            std::string update_msg = update.dump();
            
            for (auto* conn : networkUtils.lobby_update_connections[lobby_id]) {
                networkUtils.SafeSendText(conn, update_msg);
            }
            
            // If game is started, trigger AI turns and broadcast state
            if (lobby.IsStarted() && lobby.GetGame()) {
                bool state_changed = false;
                Info result = Info::TURN_ENDED;
                networkUtils.ProcessInactiveTurns(lobby.GetGame(), result, state_changed);
                
                if (result == Info::GAME_WON || result == Info::GAME_LOST) {
                    // Handle game over logic similar to HandlePlayerDisconnect
                     crow::json::wvalue over_msg;
                     over_msg["type"] = "game_over";
                     over_msg["lobby_id"] = lobby_id;
                     over_msg["result"] = (result == Info::GAME_WON ? "won" : "lost");
                     std::string msg = over_msg.dump();
                     
                     {
                         std::lock_guard<std::mutex> ws_lock(networkUtils.ws_mutex);
                         if (networkUtils.lobby_connections.count(lobby_id)) {
                             for(auto* c : networkUtils.lobby_connections[lobby_id]) {
                                 networkUtils.SafeSendText(c, msg);
                             }
                             networkUtils.lobby_connections.erase(lobby_id);
                         }
                     }
                     networkUtils.lobbies.erase(lobby_id);
                     
                     crow::json::wvalue response;
                     response["success"] = true;
                     response["lobby_deleted"] = true;
                     return crow::response(200, response);
                }
                
                if (state_changed) {
                    networkUtils.BroadcastGameStateLocked(lobby_id);
                }
            }
            
            std::cout << "Player " << user_id << " left lobby " << lobby_id << std::endl;
            
            crow::json::wvalue response;
            response["success"] = true;
            response["lobby_deleted"] = false;
            
            if (lobby.GetCurrentPlayers() == 0) {
                networkUtils.lobbies.erase(lobby_id);
                response["lobby_deleted"] = true;
                std::cout << "Lobby " << lobby_id << " destroyed (empty)" << std::endl;
            } else {
                response["current_players"] = lobby.GetCurrentPlayers();
            }
            return crow::response(200, response);
        }
    });

    // LOBBY WEBSOCKET
    CROW_WEBSOCKET_ROUTE(app, "/ws/lobby")
        .onopen([&networkUtils](crow::websocket::connection& conn) {
            std::cout << "New Lobby WebSocket connection: " << &conn << std::endl;
            {
                std::lock_guard<std::mutex> lock(networkUtils.m_validConnMutex);
                networkUtils.m_validConnections.insert(&conn);
            }
        })
        .onclose([&networkUtils](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
            std::cout << "Lobby WebSocket closed: " << &conn << std::endl;
            {
                std::lock_guard<std::mutex> lock(networkUtils.m_validConnMutex);
                networkUtils.m_validConnections.erase(&conn);
            }
            
            std::lock_guard<std::mutex> lock(networkUtils.lobby_ws_mutex);
            for (auto& [lobby_id, connections] : networkUtils.lobby_update_connections) {
                connections.erase(
                    std::remove(connections.begin(), connections.end(), &conn),
                    connections.end()
                );
            }
        })
        .onmessage([&networkUtils](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
             auto body = crow::json::load(data);
             if(!body) return;

             if (body.has("lobby_id") && body.has("action")) {
                 std::string lid = body["lobby_id"].s();
                 std::string action = body["action"].s();
                 
                 if (action == "subscribe") {
                     std::lock_guard<std::mutex> lock(networkUtils.lobby_ws_mutex);
                     bool found = false;
                     for(auto* c : networkUtils.lobby_update_connections[lid]) {
                         if(c == &conn) found = true;
                     }
                     if(!found) {
                         auto& conns = networkUtils.lobby_update_connections[lid];
                         conns.push_back(&conn);
                         std::cout << "Client subscribed to lobby " << lid << std::endl;
                         
                         std::lock_guard<std::mutex> lobby_lock(networkUtils.lobby_mutex);
                         if (networkUtils.lobbies.find(lid) != networkUtils.lobbies.end()) {
                             const Lobby& lobby = *networkUtils.lobbies[lid];
                             crow::json::wvalue update;
                             update["type"] = "lobby_state";
                             update["lobby_id"] = lid;
                             update["current_players"] = lobby.GetCurrentPlayers();
                             update["max_players"] = lobby.GetMaxPlayers();
                             update["game_started"] = lobby.IsStarted();
                             update["name"] = std::string(lobby.GetName());
                             auto duration = std::chrono::steady_clock::now() - lobby.GetRoundStartTime();
                             int elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
                             int remaining = std::max(0, 60 - elapsed_seconds);
                             update["remaining_seconds"] = remaining;
                             std::string update_msg = update.dump();
                             networkUtils.SafeSendText(&conn, update_msg);
                         }
                     }
                 }
             }
        });
}
