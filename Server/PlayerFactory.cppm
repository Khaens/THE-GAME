export module PlayerFactory;

import "IPlayer.h";
import "AbilityType.h";
import "UserModel.h";

export class PlayerFactory
{
public:
	static std::unique_ptr<IPlayer> CreateFromUser(const UserModel& user, AbilityType type);
	static std::vector<AbilityType> GetRandomUniqueAbilities(size_t playerCount);
};