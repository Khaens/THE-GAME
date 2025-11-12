#include "Database.h"

Database::Database(const std::string& path) : storage(initStorage(path)), dbPath(path)
{
	storage.sync_schema();
}

int Database::insertUser(const UserModel& user) {
    return storage.insert(user);
}

UserModel Database::getUserById(int id) {
    return storage.get<UserModel>(id);
}

UserModel Database::getUserByUsername(const std::string& username) {
    using namespace sqlite_orm;
    auto users = storage.get_all<UserModel>(
        where(c(&UserModel::username) == username)
    );
    if (users.empty()) {
        throw std::runtime_error("User not found");
    }
    return users[0];
}

std::vector<UserModel> Database::getAllUsers() {
    return storage.get_all<UserModel>();
}

void Database::updateUser(const UserModel& user) {
    storage.update(user);
}

void Database::deleteUser(int id) {
    storage.remove<UserModel>(id);
}

bool Database::userExists(const std::string& username) {
    using namespace sqlite_orm;
    auto count = storage.count<UserModel>(
        where(c(&UserModel::username) == username)
    );
    return count > 0;
}
