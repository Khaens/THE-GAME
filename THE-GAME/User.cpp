#include "User.h"

User::User(const std::string& username, const std::string& password) : m_username(username), m_password(password)
{
}

const std::string& User::GetUsername() const
{
	return m_username;
}

const std::string& User::GetPassword() const
{
	return m_password;
}

void User::SetPassword(const std::string& newPassword)
{
	m_password = newPassword;
}

void User::login() const
{

}

UserModel User::ToModel() const
{
	return UserModel{ -1, m_username, m_password };
}

