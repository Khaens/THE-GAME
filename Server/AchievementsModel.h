#pragma once
#include <string>

class AchievementsModel
{
private:
    int id = 0;
    int userId = 0;
    bool allOnRed = false;
    bool harryPotter = false;
    bool soothsayer = false;
    bool taxEvader = false;
    bool seriousPlayer = false;
    bool jack = false;
    bool zeroEffort = false;
    bool vanillaW = false;
    bool highRisk = false;
    bool perfectGame = false;
    bool sixSeven = false;


public:
    AchievementsModel() = default;
    AchievementsModel(int userId);

    int GetId() const;
    int GetUserId() const;
    bool GetAllOnRed() const;
    bool GetHarryPotter() const;
    bool GetSoothsayer() const;
    bool GetTaxEvader() const;
    bool GetSeriousPlayer() const;
    bool GetJack() const;
    bool GetZeroEffort() const;
    bool GetVanillaW() const;
    bool GetHighRisk() const;
    bool GetPerfectGame() const;
    bool GetSixSeven() const;

    void SetId(int newId);
    void SetUserId(int newUserId);
    void SetAllOnRed(bool value);
    void SetHarryPotter(bool value);
    void SetSoothsayer(bool value);
    void SetTaxEvader(bool value);
    void SetSeriousPlayer(bool value);
    void SetJack(bool value);
    void SetZeroEffort(bool value);
    void SetVanillaW(bool value);
    void SetHighRisk(bool value);
    void SetPerfectGame(bool value);
    void SetSixSeven(bool value);
};
