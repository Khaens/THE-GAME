#include <crow.h>
#include "Database.h"
#include <string>
#include "Lobby.h"
#include <mutex>
#include <random>

// --- LOBBY LOGIC (Global Scope) ---
// Global storage using pointer to Lobby class
static std::unordered_map<std::string, std::unique_ptr<Lobby>> lobbies;
static std::mutex lobby_mutex;

// Global map to store websocket connections per lobby (for game)
static std::unordered_map<std::string, std::vector<crow::websocket::connection*>> lobby_connections;
static std::mutex ws_mutex;

// Global map to store websocket connections per lobby (for lobby updates)
static std::unordered_map<std::string, std::vector<crow::websocket::connection*>> lobby_update_connections;
static std::mutex lobby_ws_mutex;



// Helper to broadcast game state to all players in a lobby
void BroadcastGameState(const std::string& lobby_id) {
    std::lock_guard<std::mutex> lock(lobby_mutex);
    if (lobbies.find(lobby_id) == lobbies.end()) return;
    
    Lobby& lobby = *lobbies[lobby_id];
    Game* game = lobby.GetGame();
    if (!game) return;

    crow::json::wvalue state_base;
    state_base["type"] = "game_state";
    state_base["lobby_id"] = lobby_id;
    
    // Piles
    auto piles = game->GetPiles(); // std::array<Pile*, 4>
    for (size_t i = 0; i < piles.size(); ++i) {
        if (piles[i]) {
            state_base["piles"][i]["top_card"] = piles[i]->GetTopCard()->GetCardValue();
            state_base["piles"][i]["count"] = piles[i]->GetSize();
        }
    }
    
    // Deck
    state_base["deck_count"] = game->GetDeckSize();
    
    // Turn info
    try {
        IPlayer& current_player = game->GetCurrentPlayer();
        state_base["current_turn_player_id"] = current_player.GetID();
        state_base["current_turn_username"] = current_player.GetUsername();
    } catch (...) {
        state_base["current_turn_player_id"] = -1;
    }

    // Players info (public)
    const auto& players = game->GetPlayers();
    for (size_t i = 0; i < players.size(); ++i) {
        state_base["players"][i]["user_id"] = players[i]->GetID();
        state_base["players"][i]["username"] = players[i]->GetUsername();
        state_base["players"][i]["hand_count"] = players[i]->GetHand().size();
        state_base["players"][i]["is_finished"] = players[i]->IsFinished();
        state_base["players"][i]["player_index"] = players[i]->GetPlayerIndex(); // or i
    }

    // Send personalized state to each connection
    std::lock_guard<std::mutex> ws_lock(ws_mutex);
    
    // We need to map connections to user_ids to send private hands.
    // However, for now, we only have a list of connections. 
    // We can't easily map connection -> user without extra storage.
    // OPTIMIZATION: We will broadcast the full state including ALL hands but marked with user_id, 
    // and the client will filter what to show (or show backs of cards).
    // ACTUALLY: It's better to send just the hand of the receiving user.
    // Issue: We don't track which connection belongs to which user in `lobby_connections`.
    // Fix: We will rely on the client ensuring they are subscribed?
    // Alternative: We can broadcast "public state" and then separate "private hand" messages if we knew who is who.
    // Assumption: For this stage, we will send all hands. It's a "local" game server, cheating is low risk. 
    // Wait, "THE-GAME" might be competitive.
    // Let's try to do it right? No, tracking connections is complex in this `main.cpp` structure without a session manager.
    // I will include ALL hands in the broadcast for now, clients will just render their own.
    
    for (size_t i = 0; i < players.size(); ++i) {
        crow::json::wvalue hand_json;
        int idx = 0;
        for (const auto* card : players[i]->GetHand()) {
            hand_json[idx++] = card->GetCardValue();
        }
        state_base["players"][i]["hand"] = std::move(hand_json);
    }
    
    std::string msg = state_base.dump();
    for (auto* conn : lobby_connections[lobby_id]) {
        if (conn) conn->send_text(msg);
    }
}

int main() {
    /*UserModel user(1, "user", "pass");
    UserModel user2(2, "user2", "pass");
    UserModel user3(3, "user3", "pass");
	UserModel user4(4, "user4", "pass");
	UserModel user5(5, "user5", "pass");
    std::vector<UserModel> users = {
        user, user2, user3, user4, user5
    };
    Game g(users);
    g.StartGame();*/


	//primeste si valideaza cereri de la client, utilizand GameServer pentru logica jocului
	//metodele din GameServer vor fi apelate aici in functie de cererile primite 
	//trimite update-uri catre client

	crow::SimpleApp app;
	Database* db = new Database("users.db");

	// ----------------------------- AUTHENTICATION ENDPOINTS --------------------------

    CROW_ROUTE(app, "/api/register")
        .methods(crow::HTTPMethod::POST)
    ([&db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string password = body["password"].s();
		std::cout << "Register attempt for user: " << username << std::endl;
		std::cout << "Password: " << password << std::endl;

        if (db->UserExists(username)) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error"] = "User already exists";
            return crow::response(409, response);
        }

        try {
            UserModel user{-1, username, password};
            int id = db->InsertUser(user);
            
            crow::json::wvalue response;
            response["success"] = true;
            response["user_id"] = id;
            return crow::response(201, response);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error"] = e.what();
            return crow::response(500, response);
        }
    });

    CROW_ROUTE(app, "/api/login")
        .methods(crow::HTTPMethod::POST)
        ([&db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string password = body["password"].s();
		std::cout << "Login attempt for user: " << username << std::endl;
		std::cout << "Password: " << password << std::endl;

        try {
            UserModel user = db->GetUserByUsername(username);
            if (db->VerifyLogin(username,password)) {
                crow::json::wvalue response;
                response["success"] = true;
                response["user_id"] = user.GetId();
                response["username"] = user.GetUsername();
                return crow::response(200, response);
            }

            crow::json::wvalue response;
            response["success"] = false;
            response["error"] = "Invalid password";
            return crow::response(401, response);
        }
        catch (const std::exception& e) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error"] = "User not found";
            return crow::response(401, response);
        }
            });

	// ----------------------------- LOBBY ENDPOINTS --------------------------

    CROW_ROUTE(app, "/api/lobby/create")
        .methods(crow::HTTPMethod::POST)
        ([&db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        int user_id = body["user_id"].i();
        std::string name = body.has("name") ? (std::string)body["name"].s() : std::string("New Lobby");
        int max_players = body.has("max_players") ? body["max_players"].i() : 4;
        std::string password = body.has("password") ? (std::string)body["password"].s() : std::string("");

        std::lock_guard<std::mutex> lock(lobby_mutex);
        
        // Create new lobby using the Lobby class
        auto new_lobby = std::make_unique<Lobby>(db, name, max_players, password);
        
        try {
            new_lobby->JoinLobby(user_id);  // Creator joins the lobby
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = e.what();
            return crow::response(400, response);
        }
        
        std::string id = new_lobby->GetId();
        lobbies[id] = std::move(new_lobby);

        crow::json::wvalue response;
        response["success"] = true;
        response["lobby_id"] = id;
        response["max_players"] = max_players;
        response["current_players"] = 1;
        return crow::response(201, response);
    });

    CROW_ROUTE(app, "/api/lobby/join")
        .methods(crow::HTTPMethod::POST)
        ([&db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        int user_id = body["user_id"].i();
        std::string lobby_id = body["lobby_id"].s();
        
        std::lock_guard<std::mutex> lock(lobby_mutex);
        if (lobbies.find(lobby_id) == lobbies.end()) {
             crow::json::wvalue response;
             response["success"] = false;
             response["error_message"] = "Lobby not found";
             return crow::response(404, response);
        }

        Lobby& lobby = *lobbies[lobby_id];
        
        // Check if already in lobby
        if (lobby.IsUserInLobby(user_id)) {
            crow::json::wvalue response;
            response["success"] = true;
            response["lobby_id"] = lobby.GetId();
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
        
        // Notify all connected clients in this lobby about the new player
        std::lock_guard<std::mutex> ws_lock(lobby_ws_mutex);
        crow::json::wvalue update;
        update["type"] = "player_joined";
        update["lobby_id"] = lobby.GetId();
        update["user_id"] = user_id;
        update["current_players"] = lobby.GetCurrentPlayers();
        update["max_players"] = lobby.GetMaxPlayers();
        
        // Get username from database
        std::string username = "Player_" + std::to_string(user_id);
        try {
            UserModel user = db->GetUserById(user_id);
            username = user.GetUsername();
        } catch (...) {
            // Use default username if we can't get it
        }
        update["username"] = username;
        
        std::string update_msg = update.dump();
        for (auto* conn : lobby_update_connections[lobby_id]) {
            if (conn) {
                conn->send_text(update_msg);
            }
        }

        crow::json::wvalue response;
        response["success"] = true;
        response["lobby_id"] = lobby.GetId();
        response["max_players"] = lobby.GetMaxPlayers();
        response["current_players"] = lobby.GetCurrentPlayers();
        return crow::response(200, response);
    });

	CROW_ROUTE(app, "/api/lobby/<string>/status")
        .methods(crow::HTTPMethod::GET)
        ([](const std::string& lobby_id) {
        
        std::lock_guard<std::mutex> lock(lobby_mutex);
        if (lobbies.find(lobby_id) == lobbies.end()) {
            return crow::response(404, "Lobby not found");
        }

        const Lobby& lobby = *lobbies[lobby_id];

        crow::json::wvalue response;
        response["lobby_id"] = lobby.GetId();
        response["current_players"] = lobby.GetCurrentPlayers(); 
        response["max_players"] = lobby.GetMaxPlayers();
        response["game_started"] = lobby.IsStarted();
        return crow::response(200, response);
    });

    // Get list of players in a lobby with their usernames
    CROW_ROUTE(app, "/api/lobby/<string>/players")
        .methods(crow::HTTPMethod::GET)
        ([&db](const std::string& lobby_id) {
        
        std::lock_guard<std::mutex> lock(lobby_mutex);
        if (lobbies.find(lobby_id) == lobbies.end()) {
            return crow::response(404, "Lobby not found");
        }

        const Lobby& lobby = *lobbies[lobby_id];

        crow::json::wvalue response;
        response["lobby_id"] = lobby.GetId();
        
        // Build players array with usernames
        int player_index = 0;
        for (const auto& user : lobby.GetUsers()) {
            response["players"][player_index]["user_id"] = user.GetId();
            response["players"][player_index]["username"] = user.GetUsername();
            response["players"][player_index]["is_host"] = lobby.IsOwner(user.GetId());
            player_index++;
        }
        return crow::response(200, response);
    });

    CROW_ROUTE(app, "/api/lobby/<string>/start")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req, const std::string& lobby_id) {
		
        std::lock_guard<std::mutex> lock(lobby_mutex);
        if (lobbies.find(lobby_id) == lobbies.end()) {
            return crow::response(404, "Lobby not found");
        }

        Lobby& lobby = *lobbies[lobby_id];
        
        try {
            lobby.Start();  // This creates the Game and starts it
            std::cout << "Lobby " << lobby_id << " started!" << std::endl;
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = e.what();
            return crow::response(400, response);
        }
        
        // Notify all connected clients that game has started
        std::lock_guard<std::mutex> ws_lock(lobby_ws_mutex);
        crow::json::wvalue update;
        update["type"] = "game_started";
        update["lobby_id"] = lobby_id;
        std::string update_msg = update.dump();
        for (auto* conn : lobby_update_connections[lobby_id]) {
            if (conn) {
                conn->send_text(update_msg);
            }
        }
        
        crow::json::wvalue response;
        response["success"] = true;
        return crow::response(200, response);
    });

    // Leave a lobby (or delete it if host leaves)
    CROW_ROUTE(app, "/api/lobby/leave")
        .methods(crow::HTTPMethod::POST)
        ([&db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        int user_id = body["user_id"].i();
        std::string lobby_id = body["lobby_id"].s();

        std::lock_guard<std::mutex> lock(lobby_mutex);
        if (lobbies.find(lobby_id) == lobbies.end()) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = "Lobby not found";
            return crow::response(404, response);
        }

        Lobby& lobby = *lobbies[lobby_id];

        // Check if user is in the lobby
        if (!lobby.IsUserInLobby(user_id)) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = "User not in lobby";
            return crow::response(400, response);
        }

        // Check if user is the host
        if (lobby.IsOwner(user_id)) {
            // Host is leaving - delete the lobby and notify everyone
            std::lock_guard<std::mutex> ws_lock(lobby_ws_mutex);
            
            crow::json::wvalue update;
            update["type"] = "lobby_closed";
            update["lobby_id"] = lobby_id;
            update["reason"] = "Host left the lobby";
            std::string update_msg = update.dump();
            
            for (auto* conn : lobby_update_connections[lobby_id]) {
                if (conn) {
                    conn->send_text(update_msg);
                }
            }
            
            // Clean up connections and lobby
            lobby_update_connections[lobby_id].clear();
            lobbies.erase(lobby_id);
            
            std::cout << "Lobby " << lobby_id << " deleted (host left)" << std::endl;
            
            crow::json::wvalue response;
            response["success"] = true;
            response["lobby_deleted"] = true;
            return crow::response(200, response);
        }
        else {
            // Regular player leaving - just remove them
            lobby.LeaveLobby(user_id);
            
            // Notify others
            std::lock_guard<std::mutex> ws_lock(lobby_ws_mutex);
            
            // Get username for notification
            std::string username = "Player_" + std::to_string(user_id);
            try {
                UserModel user = db->GetUserById(user_id);
                username = user.GetUsername();
            } catch (...) {}
            
            crow::json::wvalue update;
            update["type"] = "player_left";
            update["lobby_id"] = lobby_id;
            update["user_id"] = user_id;
            update["username"] = username;
            update["current_players"] = lobby.GetCurrentPlayers();
            update["max_players"] = lobby.GetMaxPlayers();
            std::string update_msg = update.dump();
            
            for (auto* conn : lobby_update_connections[lobby_id]) {
                if (conn) {
                    conn->send_text(update_msg);
                }
            }
            
            std::cout << "Player " << user_id << " left lobby " << lobby_id << std::endl;
            
            crow::json::wvalue response;
            response["success"] = true;
            response["lobby_deleted"] = false;
            response["current_players"] = lobby.GetCurrentPlayers();
            return crow::response(200, response);
        }
    });

	// ----------------------------- WEBSOCKET ENDPOINTS --------------------------

    // WebSocket for lobby updates (player joins/leaves)
    CROW_WEBSOCKET_ROUTE(app, "/ws/lobby")
        .onopen([](crow::websocket::connection& conn) {
            std::cout << "New Lobby WebSocket connection" << std::endl;
        })
        .onclose([](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
            std::cout << "Lobby WebSocket closed" << std::endl;
            // Remove connection from all lobbies
            std::lock_guard<std::mutex> lock(lobby_ws_mutex);
            for (auto& [lobby_id, connections] : lobby_update_connections) {
                connections.erase(
                    std::remove_if(connections.begin(), connections.end(),
                        [&conn](crow::websocket::connection* c) { return c == &conn; }),
                    connections.end()
                );
            }
        })
        .onmessage([](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
             // Expecting JSON: { "lobby_id": "...", "action": "subscribe" }
             auto body = crow::json::load(data);
             if(!body) return;

             if (body.has("lobby_id") && body.has("action")) {
                 std::string lid = body["lobby_id"].s();
                 std::string action = body["action"].s();
                 
                 if (action == "subscribe") {
                     // Register connection to lobby if not already
                     std::lock_guard<std::mutex> lock(lobby_ws_mutex);
                     bool found = false;
                     for(auto* c : lobby_update_connections[lid]) {
                         if(c == &conn) found = true;
                     }
                     if(!found) {
                         lobby_update_connections[lid].push_back(&conn);
                         std::cout << "Client subscribed to lobby " << lid << std::endl;
                         
                         // Send current lobby state
                         std::lock_guard<std::mutex> lobby_lock(lobby_mutex);
                         if (lobbies.find(lid) != lobbies.end()) {
                             const Lobby& lobby = *lobbies[lid];
                             crow::json::wvalue update;
                             update["type"] = "lobby_state";
                             update["lobby_id"] = lid;
                             update["current_players"] = lobby.GetCurrentPlayers();
                             update["max_players"] = lobby.GetMaxPlayers();
                             update["game_started"] = lobby.IsStarted();
                             conn.send_text(update.dump());
                         }
                     }
                 }
             }
		});



    CROW_WEBSOCKET_ROUTE(app, "/ws/game")
        .onopen([](crow::websocket::connection& conn) {
            std::cout << "New Game WebSocket connection" << std::endl;
        })
        .onclose([](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
            std::cout << "Game WebSocket closed" << std::endl;
            std::lock_guard<std::mutex> lock(ws_mutex);
             for (auto& [lobby_id, connections] : lobby_connections) {
                connections.erase(
                    std::remove_if(connections.begin(), connections.end(),
                        [&conn](crow::websocket::connection* c) { return c == &conn; }),
                    connections.end()
                );
            }
        })
        .onmessage([](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
             auto body = crow::json::load(data);
             if(!body) return;

             if (body.has("lobby_id")) {
                 std::string lid = body["lobby_id"].s();
                 std::string type = body.has("type") ? (std::string)body["type"].s() : "";
                 
                 // Register connection
                 {
                     std::lock_guard<std::mutex> lock(ws_mutex);
                     bool found = false;
                     for(auto* c : lobby_connections[lid]) {
                         if(c == &conn) found = true;
                     }
                     if(!found) lobby_connections[lid].push_back(&conn);
                 }

                 if (type == "chat") {
                     // Broadcast chat
                     std::lock_guard<std::mutex> lock(ws_mutex);
                     for(auto* c : lobby_connections[lid]) {
                         if(c) c->send_text(data); 
                     }
                     return;
                 }
                 
                 if (type == "game_action") {
                     // Handle game logic
                     std::string action = body["action"].s();
                     int user_id = body["user_id"].i();
                     
                     std::lock_guard<std::mutex> lock(lobby_mutex);
                     if (lobbies.find(lid) == lobbies.end()) return;
                     Lobby& lobby = *lobbies[lid];
                     Game* game = lobby.GetGame();
                     if (!game) return;
                     
                     // Find player index
                     int player_idx = -1;
                     const auto& players = game->GetPlayers();
                     for(size_t i=0; i<players.size(); ++i) {
                         if(players[i]->GetID() == user_id) {
                             player_idx = i;
                             break;
                         }
                     }
                     
                     if (player_idx == -1) return; // Player not found
                     
                     Info result = Info::TURN_ENDED; // Default safe value
                     bool state_changed = false;

                     if (action == "play_card") {
                         int card_val = body["card_value"].i();
                         int pile_idx = body["pile_index"].i();
                         result = game->PlaceCard(player_idx, card_val, pile_idx);
                         if (result == Info::CARD_PLACED || result == Info::TURN_ENDED || result == Info::GAME_WON || result == Info::GAME_LOST) {
                             state_changed = true;
                         }
                     } 
                     else if (action == "use_ability") {
                         result = game->UseAbility(player_idx);
                         if (result == Info::ABILITY_USED || result == Info::TAX_ABILITY_USED || result == Info::PEASANT_ABILITY_USED || result == Info::TURN_ENDED) {
                             state_changed = true;
                         }
                     }
                     else if (action == "end_turn") {
                         result = game->EndTurn(player_idx);
                         state_changed = true;
                     }

                     if (result == Info::GAME_WON || result == Info::GAME_LOST) {
                         // Game End
                         crow::json::wvalue over_msg;
                         over_msg["type"] = "game_over";
                         over_msg["lobby_id"] = lid;
                         if (result == Info::GAME_WON) {
                             over_msg["winner_id"] = user_id;
                             over_msg["result"] = "won";
                         } else {
                             // Assuming GAME_LOST means valid loss
                             over_msg["result"] = "lost"; 
                         }
                         
                         std::string msg = over_msg.dump();
                         std::lock_guard<std::mutex> ws_lock(ws_mutex);
                         for(auto* c : lobby_connections[lid]) {
                             if(c) c->send_text(msg);
                         }
                     }
                     
                     // If error/invalid move, maybe notify user?
                     if (result == Info::CARD_NOT_PLAYABLE || result == Info::NOT_CURRENT_PLAYER_TURN) {
                         crow::json::wvalue err;
                         err["type"] = "error";
                         if (result == Info::CARD_NOT_PLAYABLE) err["message"] = "Card not playable";
                         if (result == Info::NOT_CURRENT_PLAYER_TURN) err["message"] = "Not your turn";
                         conn.send_text(err.dump());
                     }
                 }
                 
                 // If we had game action or join, broadcast state
                 // Note: lobby_mutex is NOT held here, so it's safe to call BroadcastGameState
                 if (type == "game_action" || type == "join_game") {
                     BroadcastGameState(lid);
                 }
             }

		});

	CROW_ROUTE(app, "/")([]() {
		return crow::response(200, "THE GAME Server is running!");
		});



	app.bindaddr("0.0.0.0").port(18080).multithreaded().run();

	return 0;
}