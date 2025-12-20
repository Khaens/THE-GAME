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
            // Cleanup would be needed here in a real app (remove from lobby_connections)
        })
        .onmessage([](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
             // Expecting JSON: { "lobby_id": "...", "action": "..." }
             auto body = crow::json::load(data);
             if(!body) return;

             if (body.has("lobby_id")) {
                 std::string lid = body["lobby_id"].s();
                 
                 // Register connection to lobby if not already
                 std::lock_guard<std::mutex> lock(ws_mutex);
                 bool found = false;
                 for(auto* c : lobby_connections[lid]) {
                     if(c == &conn) found = true;
                 }
                 if(!found) lobby_connections[lid].push_back(&conn);

                 // Broadcast to others in lobby
                 for(auto* c : lobby_connections[lid]) {
                     if(c != &conn) c->send_text(data);
                 }
             }
		});

	CROW_ROUTE(app, "/")([]() {
		return crow::response(200, "THE GAME Server is running!");
		});



	app.bindaddr("0.0.0.0").port(18080).multithreaded().run();

	return 0;
}