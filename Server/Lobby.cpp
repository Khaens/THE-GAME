#include "Lobby.h"
#include <algorithm>

Lobby::Lobby(Database* db, const std::string& name, int maxPlayers, const std::string& password)
    : m_id(GenerateRandomId()), m_name(name), m_maxPlayers(maxPlayers), m_password(password), m_db(db)
{
}

void Lobby::JoinLobby(int userId)
{
    // Check if already in lobby
    for (const auto& user : m_Users) {
        if (user.GetId() == userId) {
            throw std::runtime_error("Player already in lobby");
        }
    }

    // Check if lobby is full
    if (static_cast<int>(m_Users.size()) >= m_maxPlayers) {
        throw std::runtime_error("Lobby is full");
    }

    UserModel user = m_db->GetUserById(userId);
    
    // First player becomes owner
    if (m_Users.empty()) {
        m_ownerId = userId;
    }
    
    m_Users.push_back(std::move(user));
}

void Lobby::JoinLobby(const std::string& playerName)
{
    for (const auto& user : m_Users) {
        if (user.GetUsername() == playerName) {
            throw std::runtime_error("Player already in lobby");
        }
    }

    if (static_cast<int>(m_Users.size()) >= m_maxPlayers) {
        throw std::runtime_error("Lobby is full");
    }

    UserModel user = m_db->GetUserByUsername(playerName);
    
    if (m_Users.empty()) {
        m_ownerId = user.GetId();
    }
    
    m_Users.push_back(std::move(user));
}

void Lobby::LeaveLobby(int userId)
{
    auto it = std::find_if(m_Users.begin(), m_Users.end(),
        [userId](const UserModel& user) { return user.GetId() == userId; });
    
    if (it != m_Users.end()) {
        m_Users.erase(it);
    }
}

void Lobby::Start()
{
    if (m_Users.size() < 2) {
        throw std::runtime_error("Need at least 2 players to start");
    }
    
    m_status = LobbyStatus::Started;
    m_game = std::make_unique<Game>(m_Users, *m_db);
    m_game->StartGame();
}

LobbyStatus Lobby::GetStatus() const
{
    return m_status;
}

const std::string& Lobby::GetId() const
{
    return m_id;
}

const std::string& Lobby::GetName() const
{
    return m_name;
}

int Lobby::GetMaxPlayers() const
{
    return m_maxPlayers;
}

int Lobby::GetCurrentPlayers() const
{
    return static_cast<int>(m_Users.size());
}

int Lobby::GetOwnerId() const
{
    return m_ownerId;
}

const std::vector<UserModel>& Lobby::GetUsers() const
{
    return m_Users;
}

bool Lobby::HasPassword() const
{
    return !m_password.empty();
}

bool Lobby::CheckPassword(const std::string& pwd) const
{
    return m_password == pwd;
}

Game* Lobby::GetGame()
{
    return m_game.get();
}

bool Lobby::IsStarted() const
{
    return m_status == LobbyStatus::Started;
}

bool Lobby::IsOwner(int userId) const
{
    return m_ownerId == userId;
}

bool Lobby::IsUserInLobby(int userId) const
{
    for (const auto& user : m_Users) {
        if (user.GetId() == userId) {
            return true;
        }
    }
    return false;
}

std::vector<int> Lobby::GetPlayerIds() const
{
    std::vector<int> ids;
    ids.reserve(m_Users.size());
    for (const auto& user : m_Users) {
        ids.push_back(user.GetId());
    }
    return ids;
}

std::string Lobby::GenerateRandomId(size_t length)
{
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; i++)
        result += charset[dist(rng)];
    return result;
}
