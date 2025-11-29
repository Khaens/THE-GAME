#pragma once
#include "IPlayer.h"
#include "AbilityType.h"
#include "HarryPotter.h"
#include "Gambler.h"
#include "UserModel.h"
#include "PlayerServer.h"
#include "TurnContext.h"

#include <memory>



class PlayerFactory
{
public:
	static std::unique_ptr<IPlayer> CreateFromUser(const UserModel& user, AbilityType type);
	static std::vector<AbilityType> GetRandomUniqueAbilities(size_t playerCount);

};

