#pragma once
#include <crow.h>
#include "Database.h"
#include "NetworkUtils.h"

class GameRoutes {
public:
    GameRoutes(crow::SimpleApp& app, Database* db, NetworkUtils& networkUtils);

private:
    void RegisterRoutes(crow::SimpleApp& app, Database* db, NetworkUtils& networkUtils);
};
