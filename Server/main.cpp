#include <crow.h>
#include "GameServer.h"
#include "Database.h"

int main() {

	crow::SimpleApp app;

	CROW_ROUTE(app, "/")([]() {
		return "Hello, World!";
		});

	app.port(18080).multithreaded().run();

	return 0;
}