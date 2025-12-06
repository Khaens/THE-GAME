#pragma once
#include <string>

class AchievementsModel
{
private:
    int id = 0;
    int userId = 0;
    bool allOnRed = false;
    bool harryPotter = false;
    bool seriousPlayer = false;

public:
    AchievementsModel() = default;
    AchievementsModel(int userId);

    int GetId() const;
    int GetUserId() const;
    bool GetAllOnRed() const;
    bool GetHarryPotter() const;
    bool GetSeriousPlayer() const;

    void SetId(int newId);
    void SetUserId(int newUserId);
    void SetAllOnRed(bool value);
    void SetHarryPotter(bool value);
    void SetSeriousPlayer(bool value);
};