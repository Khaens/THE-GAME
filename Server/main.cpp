#include <crow.h>
#include "GameServer.h"
#include "Database.h"

#include "Gambler.h"
int main() {
	Game game(3);
	game.StartGame();
	crow::SimpleApp app;

	CROW_ROUTE(app, "/")([]() {
		return "Hello, World!";
		});

	app.port(18080).multithreaded().run();

	return 0;
}