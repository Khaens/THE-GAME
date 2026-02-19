export module PlayerFactory;

import "PlayerServer.h";
import "AbilityType.h";
import "UserModel.h";

export class PlayerFactory
{
public:
	static Player CreateFromUser(const UserModel& user, AbilityType type);
	static std::vector<AbilityType> GetRandomUniqueAbilities(size_t playerCount);
};