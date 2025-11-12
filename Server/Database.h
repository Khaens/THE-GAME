#pragma once
#include <sqlite_orm/sqlite_orm.h>
#include "../THE-GAME/UserModel.h"

using namespace sqlite_orm;

inline auto initStorage(const std::string& path) {
    return make_storage(path,
        make_table("users",
            make_column("id", &UserModel::id, primary_key().autoincrement()),
            make_column("username", &UserModel::username, unique()),
            make_column("password", &UserModel::password))
    );
}

using Storage = decltype(initStorage(""));

class Database
{
private:
    Storage storage;
    std::string dbPath;

public:
    Database(const std::string& path = "users.db");

};

