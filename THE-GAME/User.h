#pragma once
#include <string>
//#include <sqlite_orm/sqlite_orm.h>

class User
{
private:
	std::string m_username, m_password;
public:
	User(const std::string& username, const std::string& password);
	const std::string& GetUsername() const;
	const std::string& GetPassword() const;

	void SetPassword(const std::string& newPassword);
	void login() const;
};

