#pragma once
#include <cstdint>

struct AchievementsModel
{
    int id = 0;        
    int userId = 0;
    unsigned int wins;
    unsigned int losses;
    std::uint8_t winRate;

	AchievementsModel() = default;
};

