#include "UserModel.h"

UserModel::UserModel(int id, const std::string& user, const std::string& pass) 
	: m_id(id), m_username(user), m_password(pass)
{
}

UserModel::UserModel(const UserModel& user) : m_id(user.GetId()), m_password(user.GetPassword())
, m_username(user.GetUsername())
{
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



