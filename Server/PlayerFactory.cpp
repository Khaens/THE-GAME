#include "PlayerFactory.h"
#include "PlayerServer.h"
#include <stdexcept>
#include <random>

template class Player<Gambler>;
template class Player<HarryPotter>;

std::unique_ptr<IPlayer> PlayerFactory::CreateFromUser(const UserModel& user, AbilityType type)
{
    switch (type) {
    case AbilityType::HarryPotter: {
        return std::make_unique<Player<HarryPotter>>(user);
    }
    case AbilityType::Gambler: {
        return std::make_unique<Player<Gambler>>(user);
    }
    default: return nullptr;
    }
}

std::vector<AbilityType> PlayerFactory::GetRandomUniqueAbilities(size_t playerCount)
{
    std::vector<AbilityType> abilities = {
        AbilityType::Gambler,
        AbilityType::HarryPotter
    };

    if (playerCount > abilities.size())
        throw std::runtime_error("Not enough abilities for unique assignment");

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::shuffle(abilities.begin(), abilities.end(), gen);

    abilities.resize(playerCount);

    return abilities;
}