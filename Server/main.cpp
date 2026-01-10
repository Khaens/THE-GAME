#include <crow.h>
#include "Database.h"
#include "NetworkUtils.h"
#include "AuthRoutes.h"
#include "LobbyRoutes.h"
#include "GameRoutes.h"

int main() {
    crow::SimpleApp app;
    Database* db = new Database("users.db");
    
    // Centralized access to globals and helpers
    NetworkUtils networkUtils;
    
    // Start Chat Worker
    networkUtils.StartChatWorker();

    // Register Routes (const instances as requested)
    const AuthRoutes authRoutes(app, db, networkUtils);
    const LobbyRoutes lobbyRoutes(app, db, networkUtils);
    const GameRoutes gameRoutes(app, db, networkUtils);

    CROW_ROUTE(app, "/")([]() {
        return crow::response(200, "THE GAME Server is running!");
    });

    app.bindaddr("0.0.0.0").port(18080).multithreaded().run();

    return 0;
}