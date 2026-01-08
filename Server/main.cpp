#include <crow.h>
#include "Database.h"
#include <string>
#include "Lobby.h"
#include <mutex>
#include <random>

// --- LOBBY LOGIC (Global Scope) ---
static std::unordered_map<std::string, std::unique_ptr<Lobby>> lobbies;
static std::mutex lobby_mutex;

static std::unordered_map<std::string, std::vector<crow::websocket::connection*>> lobby_connections;
static std::mutex ws_mutex;

static std::unordered_map<std::string, std::vector<crow::websocket::connection*>> lobby_update_connections;
static std::mutex lobby_ws_mutex;

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
    auto piles = game->GetPiles();
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
        
        std::lock_guard<std::mutex> ws_lock(lobby_ws_mutex);
        crow::json::wvalue update;
        update["type"] = "player_joined";
        update["lobby_id"] = lobby.GetId();
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
            lobby.Start();
            std::cout << "Lobby " << lobby_id << " started!" << std::endl;
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = e.what();
            return crow::response(400, response);
        }
        
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

        if (!lobby.IsUserInLobby(user_id)) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = "User not in lobby";
            return crow::response(400, response);
        }

        if (lobby.IsOwner(user_id)) {
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
          
            lobby_update_connections[lobby_id].clear();
            lobbies.erase(lobby_id);
            
            std::cout << "Lobby " << lobby_id << " deleted (host left)" << std::endl;
            
            crow::json::wvalue response;
            response["success"] = true;
            response["lobby_deleted"] = true;
            return crow::response(200, response);
        }
        else {
            lobby.LeaveLobby(user_id);
            
            std::lock_guard<std::mutex> ws_lock(lobby_ws_mutex);
            
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

    CROW_WEBSOCKET_ROUTE(app, "/ws/lobby")
        .onopen([](crow::websocket::connection& conn) {
            std::cout << "New Lobby WebSocket connection" << std::endl;
        })
        .onclose([](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
            std::cout << "Lobby WebSocket closed" << std::endl;
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
             auto body = crow::json::load(data);
             if(!body) return;

             if (body.has("lobby_id") && body.has("action")) {
                 std::string lid = body["lobby_id"].s();
                 std::string action = body["action"].s();
                 
                 if (action == "subscribe") {
                     std::lock_guard<std::mutex> lock(lobby_ws_mutex);
                     bool found = false;
                     for(auto* c : lobby_update_connections[lid]) {
                         if(c == &conn) found = true;
                     }
                     if(!found) {
                         lobby_update_connections[lid].push_back(&conn);
                         std::cout << "Client subscribed to lobby " << lid << std::endl;
                         
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
                 
                 {
                     std::lock_guard<std::mutex> lock(ws_mutex);
                     bool found = false;
                     for(auto* c : lobby_connections[lid]) {
                         if(c == &conn) found = true;
                     }
                     if(!found) lobby_connections[lid].push_back(&conn);
                 }

                 if (type == "chat") {
                     std::lock_guard<std::mutex> lock(lobby_mutex);
                     if (lobbies.find(lid) != lobbies.end()) {
                         Lobby& lobby = *lobbies[lid];
                         Game* game = lobby.GetGame();
                         if (game) {
                             int uid = body["user_id"].i();
                             std::string username = "Unknown";
                             
                             const auto& players = game->GetPlayers();
                             for(const auto& p : players) {
                                 if (p->GetID() == uid) {
                                     username = p->GetUsername();
                                     break;
                                 }
                             }
                             
                             crow::json::wvalue chat_val = body;
                             chat_val["username"] = username;
                             
                             std::string mod_data = chat_val.dump();
                             
                             std::lock_guard<std::mutex> ws_lock(ws_mutex);
                             for(auto* c : lobby_connections[lid]) {
                                 if(c) c->send_text(mod_data); 
                             }
                         }
                     }
                     return;
                 }
                 
                 if (type == "game_action") {
                     std::string action = body["action"].s();
                     int user_id = body["user_id"].i();
                     
                     std::lock_guard<std::mutex> lock(lobby_mutex);
                     if (lobbies.find(lid) == lobbies.end()) return;
                     Lobby& lobby = *lobbies[lid];
                     Game* game = lobby.GetGame();
                     if (!game) return;
                     
                     int player_idx = -1;
                     const auto& players = game->GetPlayers();
                     for(size_t i=0; i<players.size(); ++i) {
                         if(players[i]->GetID() == user_id) {
                             player_idx = i;
                             break;
                         }
                     }
                     
                     if (player_idx == -1) return; 
                     
                     Info result = Info::TURN_ENDED; 
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
                         crow::json::wvalue over_msg;
                         over_msg["type"] = "game_over";
                         over_msg["lobby_id"] = lid;
                         if (result == Info::GAME_WON) {
                             over_msg["winner_id"] = user_id;
                             over_msg["result"] = "won";
                         } else {
                             over_msg["result"] = "lost"; 
                         }
                         
                         std::string msg = over_msg.dump();
                         std::lock_guard<std::mutex> ws_lock(ws_mutex);
                         for(auto* c : lobby_connections[lid]) {
                             if(c) c->send_text(msg);
                         }
                     }
                     
                     if (result == Info::CARD_NOT_PLAYABLE || result == Info::NOT_CURRENT_PLAYER_TURN) {
                         crow::json::wvalue err;
                         err["type"] = "error";
                         if (result == Info::CARD_NOT_PLAYABLE) err["message"] = "Card not playable";
                         if (result == Info::NOT_CURRENT_PLAYER_TURN) err["message"] = "Not your turn";
                         conn.send_text(err.dump());
                     }
                 }
                 
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