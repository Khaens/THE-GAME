#pragma once
#include <iostream>
#include "UserModel.h"
#include "IPlayer.h"
#include "CardServer.h";
#include "Gambler.h"
#include "HarryPotter.h"
#include "TaxEvader.h"
#include "Soothsayer.h"
#include "Peasant.h"
#include <string_view>
#include "TurnContext.h"


class Player :
    public UserModel
{
private:
    std::vector<std::unique_ptr<Card>> m_hand;
    std::unique_ptr<IAbility> ability;
    size_t m_playerIndex = 10;
    bool m_finished = false;
public:
    Player() = default;
    Player(const UserModel& user, std::unique_ptr<IAbility> ability);
    ~Player();

    // Prevent copying (unique_ptr member makes copying invalid)
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    // Explicit move operations so Player is movable into containers like std::vector
    Player(Player&& other) noexcept;
    Player& operator=(Player&& other) noexcept;

    void AddCardToHand(std::unique_ptr<Card> card);
    std::unique_ptr<Card> RemoveCardFromHand(Card* card);
    const std::vector<std::unique_ptr<Card>>& GetHand() const;
    Card* ChooseCard(const std::string& cardValue);
    const std::string& GetUsername() const;
    void SetPlayerIndex(size_t index);
    size_t GetPlayerIndex() const;
    void SetFinished(bool state);
    bool IsFinished() const;
    Card* GetCardFromHand(const std::string& cardValue) const;

    void ShowHand();
    AbilityType GetAbilityType() const;
    void UseAbility(TurnContext& ctx, size_t CurrentPIndex);
    bool CanUseAbility(TurnContext& ctx) const;
    const int GetID();

    const bool IsTaxActive() const;
    void SetTaxActive(bool state) const;

    const bool HPActive() const;
    const bool GetHPFlag() const;
    void SetHPFlag(bool state);
    void SetHPActive(bool state);

    const size_t GetGamblerUses() const;
    const bool GActive() const;
    void SetGActive(bool state);

    void SetSoothState(bool state);
    bool IsSoothActive() const;
    const size_t GetSoothsayerUses() const;

    const size_t GetTaxEvaderUses() const;

    bool GetSameTurn();
    void SetSameTurn(bool sameTurn);
};