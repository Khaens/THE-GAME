#include "UserModel.h"

UserModel::UserModel() : id(-1), username(""), password("")
{
}

UserModel::UserModel(int id, const std::string& user, const std::string& pass)
	: id(id), username(user), password(pass)
{
}


