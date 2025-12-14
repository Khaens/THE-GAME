#include <crow.h>
#include "Database.h"
#include <string>
#include "GameServer.h"
#include <mutex>

// --- LOBBY LOGIC (Global Scope) ---
struct Lobby {
    std::string id;
    std::string name;
    int owner_id;
    int max_players;
    std::vector<int> players;
    bool started = false;
    std::string password;
};

// Global storage to avoid lambda capture issues
static std::unordered_map<std::string, Lobby> lobbies;
static std::mutex lobby_mutex;

// Global map to store websocket connections per lobby
static std::unordered_map<std::string, std::vector<crow::websocket::connection*>> lobby_connections;
static std::mutex ws_mutex;

// Helper to generate random 6-char ID
std::string generateLobbyId() {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string s(6, ' ');
    for (int i = 0; i < 6; ++i) s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
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
	Database db("users.db");

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

        if (db.UserExists(username)) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error"] = "User already exists";
            return crow::response(409, response);
        }

        try {
            UserModel user{-1, username, password};
            int id = db.InsertUser(user);
            
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
            UserModel user = db.GetUserByUsername(username);
            if (db.VerifyLogin(username,password)) {
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
        std::string id = generateLobbyId();
        while (lobbies.count(id)) id = generateLobbyId();

        Lobby new_lobby;
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
        ([](const crow::request& req) {
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

        Lobby& lobby = lobbies[lobby_id];
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
        ([](const std::string& lobby_id) {
        
        std::lock_guard<std::mutex> lock(lobby_mutex);
        if (lobbies.find(lobby_id) == lobbies.end()) {
            return crow::response(404, "Lobby not found");
        }

        const Lobby& lobby = lobbies[lobby_id];

        crow::json::wvalue response;
        response["lobby_id"] = lobby.id;
        response["current_players"] = (int)lobby.players.size(); 
        response["max_players"] = lobby.max_players;
        response["game_started"] = lobby.started;
        return crow::response(200, response);
    });

    CROW_ROUTE(app, "/api/lobby/<string>/start")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req, const std::string& lobby_id) {
		
        std::lock_guard<std::mutex> lock(lobby_mutex);
        if (lobbies.find(lobby_id) == lobbies.end()) {
            return crow::response(404, "Lobby not found");
        }

        lobbies[lobby_id].started = true;
        std::cout << "Lobby " << lobby_id << " started!" << std::endl;
        
        crow::json::wvalue response;
        response["success"] = true;
        return crow::response(200, response);
    });

    CROW_WEBSOCKET_ROUTE(app, "/ws/game")
        .onopen([&](crow::websocket::connection& conn) {
            std::cout << "New WebSocket connection" << std::endl;
        })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
            std::cout << "WebSocket closed" << std::endl;
            // Cleanup would be needed here in a real app (remove from lobby_connections)
        })
        .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
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



	app.port(18080).multithreaded().run();

	return 0;
}