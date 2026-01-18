module;

#include <stdexcept>
#include <random>
#include <memory>

module PlayerFactory;

import "PlayerServer.h";
import "TurnContext.h";
import "Gambler.h";
import "HarryPotter.h";
import "TaxEvader.h";
import "Soothsayer.h";
import "UserModel.h";
import "AbilityType.h";

template class Player<Gambler>;
template class Player<HarryPotter>;
template class Player<TaxEvader>;
template class Player<Soothsayer>;
template class Player<Peasant>;

std::unique_ptr<IPlayer> PlayerFactory::CreateFromUser(const UserModel& user, AbilityType type)
{
    switch (type) {
    case AbilityType::HarryPotter: {
        return std::make_unique<Player<HarryPotter>>(user);
    }
    case AbilityType::Gambler: {
        return std::make_unique<Player<Gambler>>(user);
    }
    case AbilityType::TaxEvader: {
        return std::make_unique<Player<TaxEvader>>(user);
    }
    case AbilityType::Soothsayer: {
        return std::make_unique<Player<Soothsayer>>(user);
    }
    case AbilityType::Peasant: {
        return std::make_unique<Player<Peasant>>(user);
	}
    default: return nullptr;
    }
}

std::vector<AbilityType> PlayerFactory::GetRandomUniqueAbilities(size_t playerCount)
{
    std::vector<AbilityType> abilities = {
        AbilityType::Gambler,
        AbilityType::HarryPotter,
        AbilityType::TaxEvader,
        AbilityType::Soothsayer,
		AbilityType::Peasant
    };

    if (playerCount > abilities.size())
        throw std::runtime_error("Not enough abilities for unique assignment");

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::shuffle(abilities.begin(), abilities.end(), gen);

    abilities.resize(playerCount);

    return abilities;
}