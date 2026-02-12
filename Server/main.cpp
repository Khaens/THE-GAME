#include <crow.h>
#include <memory>
#include "Database.h"
#include "NetworkUtils.h"
#include "AuthRoutes.h"
#include "LobbyRoutes.h"
#include "GameRoutes.h"

int main() {
    crow::SimpleApp app;

    //db->RestoreFromBackup("backups/pre_sync_20260212_203459.db");

    auto db = std::make_unique<Database>("users.db");
    
    // Centralized access to globals and helpers
    NetworkUtils networkUtils;
    
    // Start Chat Worker
    networkUtils.StartChatWorker();
    
    // Start WebSocket Send Worker (all WS sends go through this thread)
    networkUtils.StartWsWorker();

    // Register Routes (const instances as requested)
    const AuthRoutes authRoutes(app, db.get(), networkUtils);
    const LobbyRoutes lobbyRoutes(app, db.get(), networkUtils);
    const GameRoutes gameRoutes(app, db.get(), networkUtils);

    CROW_ROUTE(app, "/")([]() {
        return crow::response(200, "THE GAME Server is running!");
    });

    app.bindaddr("0.0.0.0").port(18080).multithreaded().run();

    return 0;
}