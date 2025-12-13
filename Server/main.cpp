#include <crow.h>
#include "Database.h"
#include <string>
#include "GameServer.h"
#include <mutex>

int main() {
    UserModel user(1, "user", "pass");
    UserModel user2(2, "user2", "pass");
    UserModel user3(3, "user3", "pass");
	UserModel user4(4, "user4", "pass");
	UserModel user5(5, "user5", "pass");
    std::vector<UserModel> users = {
        user, user2, user3, user4, user5
    };
    Game g(users);
    g.StartGame();


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
        // TODO: Implementează logica de creare lobby

        crow::json::wvalue response;
        response["success"] = true;
        response["lobby_id"] = "temp_lobby_123"; // Placeholder
        response["max_players"] = 5;
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
        // TODO: Implementează logica de join lobby

        crow::json::wvalue response;
        response["success"] = true;
        return crow::response(200, response);
            });

	CROW_ROUTE(app, "/api/lobby/<string>/status")
        .methods(crow::HTTPMethod::GET)
        ([](const std::string& lobby_id) {
        // TODO: Implementează logica de status lobby
        crow::json::wvalue response;
        response["lobby_id"] = lobby_id;
        response["current_players"] = 3; // Placeholder
        response["max_players"] = 5;
        response["game_started"] = false;
        return crow::response(200, response);
            });

    CROW_ROUTE(app, "/api/lobby/<string>/start")
        .methods(crow::HTTPMethod::POST)
        ([](const std::string& start) {
		auto body = crow::json::load(start);
		//TODO: Implementează logica de start joc
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }
		std::string lobby_id = body["lobby_id"].s();
			});

    CROW_WEBSOCKET_ROUTE(app, "/ws/game")
        .onopen([&](crow::websocket::connection& conn) {
            std::cout << "WebSocket connection opened" << std::endl;
        })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
            std::cout << "WebSocket connection closed: " << reason << std::endl;
        })
        .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        std::cout << "Received WebSocket message: " << data << std::endl;
		});


	CROW_ROUTE(app, "/")([]() {
		return crow::response(200, "THE GAME Server is running!");
		});



	app.port(18080).multithreaded().run();

	return 0;
}