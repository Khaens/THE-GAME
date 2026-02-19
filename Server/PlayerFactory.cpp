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


Player PlayerFactory::CreateFromUser(const UserModel& user, AbilityType type)
{
    switch (type) {
    case AbilityType::HarryPotter: {
        return Player(user, std::make_unique<HarryPotter>());
    }
    case AbilityType::Gambler: {
        return Player(user, std::make_unique<Gambler>());
    }
    case AbilityType::TaxEvader: {
        return Player(user, std::make_unique<TaxEvader>());
    }
    case AbilityType::Soothsayer: {
        return Player(user, std::make_unique<Soothsayer>());
    }
    case AbilityType::Peasant: {
        return Player(user, std::make_unique<Peasant>());
    }
    default: {
		throw std::invalid_argument("Invalid ability type");
    }
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