#include <crow.h>
#include "Database.h"
#include <string>
#include "Lobby.h"
#include <mutex>
#include <random>

// --- LOBBY LOGIC (Global Scope) ---
struct LobbyData {
    std::string id;
    std::string name;
    int owner_id;
    int max_players;
    std::vector<int> players;
    bool started = false;
    std::string password;
};

// Global storage to avoid lambda capture issues
static std::unordered_map<std::string, LobbyData> lobbies;
static std::mutex lobby_mutex;

// Global map to store websocket connections per lobby (for game)
static std::unordered_map<std::string, std::vector<crow::websocket::connection*>> lobby_connections;
static std::mutex ws_mutex;

// Global map to store websocket connections per lobby (for lobby updates)
static std::unordered_map<std::string, std::vector<crow::websocket::connection*>> lobby_update_connections;
static std::mutex lobby_ws_mutex;

std::string generateLobbyId() {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static std::random_device rd;  // Hardware entropy source
    static std::mt19937 gen(rd()); // Mersenne Twister generator
    static std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    
    std::string s(6, ' ');
    for (int i = 0; i < 6; ++i) {
        s[i] = alphanum[dis(gen)];
    }
    return s;
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
        ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        int user_id = body["user_id"].i();
        std::string name = body.has("name") ? (std::string)body["name"].s() : std::string("New Lobby");
        int max_players = body.has("max_players") ? body["max_players"].i() : 4;
        std::string password = body.has("password") ? (std::string)body["password"].s() : std::string("");

        std::lock_guard<std::mutex> lock(lobby_mutex);
        
        // Create new lobby
        std::string id = generateLobbyId();
        LobbyData new_lobby;
        new_lobby.id = id;
        new_lobby.name = name;
        new_lobby.owner_id = user_id;
        new_lobby.max_players = max_players;
        new_lobby.password = password;
        new_lobby.players.push_back(user_id);

        lobbies[id] = new_lobby;

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

        LobbyData& lobby = lobbies[lobby_id];
        if (lobby.players.size() >= lobby.max_players) {
             crow::json::wvalue response;
             response["success"] = false;
             response["error_message"] = "Lobby is full";
             return crow::response(409, response);
        }

        // Check if already in
        bool already_in = false;
        for(int pid : lobby.players) if(pid == user_id) already_in = true;
        
        if(!already_in) {
            lobby.players.push_back(user_id);
            
            // Notify all connected clients in this lobby about the new player
            std::lock_guard<std::mutex> ws_lock(lobby_ws_mutex);
            crow::json::wvalue update;
            update["type"] = "player_joined";
            update["lobby_id"] = lobby.id;
            update["user_id"] = user_id;
            update["current_players"] = (int)lobby.players.size();
            update["max_players"] = lobby.max_players;
            
            // Get username from database
            std::string username = "Player_" + std::to_string(user_id);
            try {
                std::vector<UserModel> all_users = db->GetAllUsers();
                for (const auto& user : all_users) {
                    if (user.GetId() == user_id) {
                        username = user.GetUsername();
                        break;
                    }
                }
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
        }

        crow::json::wvalue response;
        response["success"] = true;
        response["lobby_id"] = lobby.id;
        response["max_players"] = lobby.max_players;
        response["current_players"] = (int)lobby.players.size();
        return crow::response(200, response);
    });

	CROW_ROUTE(app, "/api/lobby/<string>/status")
        .methods(crow::HTTPMethod::GET)
        ([&db](const std::string& lobby_id) {
        
        std::lock_guard<std::mutex> lock(lobby_mutex);
        if (lobbies.find(lobby_id) == lobbies.end()) {
            return crow::response(404, "Lobby not found");
        }

        const LobbyData& lobby = lobbies[lobby_id];

        crow::json::wvalue response;
        response["lobby_id"] = lobby.id;
        response["current_players"] = (int)lobby.players.size(); 
        response["max_players"] = lobby.max_players;
        response["game_started"] = lobby.started;
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

        const LobbyData& lobby = lobbies[lobby_id];

        crow::json::wvalue response;
        response["lobby_id"] = lobby.id;
        
        // Build players array with usernames
        // Use a vector of indices and build the array directly
        int player_index = 0;
        for (int user_id : lobby.players) {
            std::string username = "Player_" + std::to_string(user_id);
            try {
                // Get user by ID - we need to add this method or use a workaround
                // For now, we'll get all users and find the one we need
                std::vector<UserModel> all_users = db->GetAllUsers();
                for (const auto& user : all_users) {
                    if (user.GetId() == user_id) {
                        username = user.GetUsername();
                        break;
                    }
                }
            } catch (...) {
                // Use default username if we can't get it
            }
            
            response["players"][player_index]["user_id"] = user_id;
            response["players"][player_index]["username"] = username;
            response["players"][player_index]["is_host"] = (user_id == lobby.owner_id);
            player_index++;
        }
        return crow::response(200, response);
    });

    CROW_ROUTE(app, "/api/lobby/<string>/start")
        .methods(crow::HTTPMethod::POST)
        ([&db](const crow::request& req, const std::string& lobby_id) {
		
        std::lock_guard<std::mutex> lock(lobby_mutex);
        if (lobbies.find(lobby_id) == lobbies.end()) {
            return crow::response(404, "Lobby not found");
        }

        lobbies[lobby_id].started = true;
        std::cout << "Lobby " << lobby_id << " started!" << std::endl;
        
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

        LobbyData& lobby = lobbies[lobby_id];

        // Check if user is in the lobby
        auto it = std::find(lobby.players.begin(), lobby.players.end(), user_id);
        if (it == lobby.players.end()) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error_message"] = "User not in lobby";
            return crow::response(400, response);
        }

        // Check if user is the host
        if (user_id == lobby.owner_id) {
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
            lobby.players.erase(it);
            
            // Notify others
            std::lock_guard<std::mutex> ws_lock(lobby_ws_mutex);
            
            // Get username for notification
            std::string username = "Player_" + std::to_string(user_id);
            try {
                std::vector<UserModel> all_users = db->GetAllUsers();
                for (const auto& user : all_users) {
                    if (user.GetId() == user_id) {
                        username = user.GetUsername();
                        break;
                    }
                }
            } catch (...) {}
            
            crow::json::wvalue update;
            update["type"] = "player_left";
            update["lobby_id"] = lobby_id;
            update["user_id"] = user_id;
            update["username"] = username;
            update["current_players"] = (int)lobby.players.size();
            update["max_players"] = lobby.max_players;
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
            response["current_players"] = (int)lobby.players.size();
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
                             const LobbyData& lobby = lobbies[lid];
                             crow::json::wvalue update;
                             update["type"] = "lobby_state";
                             update["lobby_id"] = lid;
                             update["current_players"] = (int)lobby.players.size();
                             update["max_players"] = lobby.max_players;
                             update["game_started"] = lobby.started;
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

	Lobby lobby(db);
	lobby.Start();
	Game game();



	CROW_ROUTE(app, "/")([]() {
		return crow::response(200, "THE GAME Server is running!");
		});



	app.bindaddr("0.0.0.0").port(18080).multithreaded().run();

	return 0;
}