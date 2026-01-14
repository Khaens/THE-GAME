export module PlayerFactory;

export import "IPlayer.h";
export import "AbilityType.h";
export import "Gambler.h";
export import "HarryPotter.h";
export import "TaxEvader.h";
export import "Soothsayer.h";
export import "UserModel.h";
export import "PlayerServer.h";
export import "TurnContext.h";

export class PlayerFactory
{
public:
	static std::unique_ptr<IPlayer> CreateFromUser(const UserModel& user, AbilityType type);
	static std::vector<AbilityType> GetRandomUniqueAbilities(size_t playerCount);
};

