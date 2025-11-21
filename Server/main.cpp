#include <crow.h>
#include "GameServer.h"
#include "Database.h"
#include <memory>

#include "Gambler.h"
int main() {
	crow::SimpleApp app;

	Database db("users.db");


	CROW_ROUTE(app, "/")([]() {
		return crow::response(200, "THE GAME Server is running!");
		});



	app.port(18080).multithreaded().run();

	return 0;
}