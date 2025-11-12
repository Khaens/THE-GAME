#include "Database.h"

Database::Database(const std::string& path) : storage(initStorage(path)), dbPath(path)
{
	storage.sync_schema();
}
