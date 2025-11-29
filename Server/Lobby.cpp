#include "Lobby.h"

Lobby::Lobby(Database* db) : m_id(GenerateRandomId())
{
    m_db = db;
}

void Lobby::JoinLobby(std::string playerName)
{
    for (auto& user : m_Users)
        if (user.GetUsername() == playerName)
            throw std::runtime_error("Player already in lobby");

    UserModel user = m_db->GetUserByUsername(playerName);

    m_Users.push_back(std::move(user));
}

std::string Lobby::GenerateRandomId(size_t length)
{
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; i++)
        result += charset[dist(rng)];
    return result;
}
