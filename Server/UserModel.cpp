#include "UserModel.h"

UserModel::UserModel(int id, const std::string& user, const std::string& pass) 
	: m_id(id), m_username(user), m_password(pass)
{
}

UserModel::UserModel(const UserModel& user) 
    : m_id(user.m_id), m_username(user.m_username), m_password(user.m_password)
{
}

UserModel::UserModel(UserModel&& user) noexcept
    : m_id(user.m_id), m_username(std::move(user.m_username)), m_password(std::move(user.m_password))
{
    user.m_id = -1;
}

UserModel& UserModel::operator=(const UserModel& user)
{
    if (this != &user) {
        m_id = user.m_id;
        m_username = user.m_username;
        m_password = user.m_password;
    }
    return *this;
}

UserModel& UserModel::operator=(UserModel&& user) noexcept
{
    if (this != &user) {
        m_id = user.m_id;
        m_username = std::move(user.m_username);
        m_password = std::move(user.m_password);
        user.m_id = -1;
    }
    return *this;
}

const int& UserModel::GetId() const
{
	return m_id;
}

const std::string& UserModel::GetUsername() const
{
	return m_username;
}

const std::string& UserModel::GetPassword() const
{
	return m_password;
}

void UserModel::SetId(int id)
{
	m_id = id;
}

void UserModel::SetUsername(const std::string& username)
{
	m_username = username;
}

void UserModel::SetPassword(const std::string& password)
{
	m_password = password;
}



