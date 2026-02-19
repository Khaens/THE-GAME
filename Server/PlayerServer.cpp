#include "PlayerServer.h"
#include <utility>

// Move constructor
Player::Player(Player&& other) noexcept
    : UserModel(std::move(other))
    , m_hand(std::move(other.m_hand))
    , ability(std::move(other.ability))
    , m_playerIndex(other.m_playerIndex)
    , m_finished(other.m_finished)
{
    // Leave other in a valid state
    other.m_playerIndex = 10;
    other.m_finished = false;
}

// Move assignment
Player& Player::operator=(Player&& other) noexcept
{
    if (this != &other) {
        UserModel::operator=(std::move(other));
        m_hand = std::move(other.m_hand);
        ability = std::move(other.ability);
        m_playerIndex = other.m_playerIndex;
        m_finished = other.m_finished;

        other.m_playerIndex = 10;
        other.m_finished = false;
    }
    return *this;
}

Player::Player(const UserModel& user, std::unique_ptr<IAbility> ability)
	: UserModel(user), ability(std::move(ability))
{
}

Player::~Player()
{
    m_hand.clear();
}


void Player::AddCardToHand(std::unique_ptr<Card> card)
{
    m_hand.push_back(std::move(card));
}


std::unique_ptr<Card> Player::RemoveCardFromHand(Card* card)
{
    auto it = std::find_if(m_hand.begin(), m_hand.end(),
        [&card](const std::unique_ptr<Card>& c) { return c.get() == card; });

    if (it != m_hand.end()) {
        std::unique_ptr<Card> removedCard = std::move(*it);
        m_hand.erase(it);
        return removedCard;
    }
    return nullptr;
}


const std::vector<std::unique_ptr<Card>>& Player::GetHand() const
{
    return m_hand;
}

Card* Player::ChooseCard(const std::string& cardValue)
{
    auto it = std::find_if(m_hand.begin(), m_hand.end(),
        [&cardValue](const std::unique_ptr<Card>& c) { return c->GetCardValue() == cardValue; });

    if (it != m_hand.end()) {
        return it->get();
    }
    return nullptr;
}

const std::string& Player::GetUsername() const
{
    return UserModel::GetUsername();
}


void Player::SetPlayerIndex(size_t index)
{
    m_playerIndex = index;
}


size_t Player::GetPlayerIndex() const
{
    return m_playerIndex;
}


void Player::SetFinished(bool state)
{
    m_finished = state;
}


bool Player::IsFinished() const
{
    return m_finished;
}

Card* Player::GetCardFromHand(const std::string& cardValue) const
{
    for (const auto& card : m_hand) {
        if (card->GetCardValue() == cardValue) {
            return card.get();
        }
    }
    return nullptr;
}


void Player::ShowHand()
{
    for (const auto& card : m_hand) {
        std::cout << card->GetCardValue() << " ";
    }
    std::cout << std::endl;
}


AbilityType Player::GetAbilityType() const
{
    return ability->GetAbilityType();
}


void Player::UseAbility(TurnContext& ctx, size_t currentPIndex)
{
    ability->UseAbility(ctx, currentPIndex);
}


bool Player::CanUseAbility(TurnContext& ctx) const
{
    return ability->CanUseAbility(ctx);
}


const int Player::GetID()
{
    return UserModel::GetId();
}



const bool Player::IsTaxActive() const
{
    return ability->IsTaxActive();
}


void Player::SetTaxActive(bool state) const
{
    ability->SetTaxActive(state);
}


const bool Player::HPActive() const
{
    return ability->HPActive();
}

const bool Player::GetHPFlag() const
{
    return ability->GetHPFlag();
}


void Player::SetHPFlag(bool state)
{
    ability->SetHPFlag(state);
}


void Player::SetHPActive(bool state)
{
    ability->SetHPActive(state);
}

const size_t Player::GetGamblerUses() const
{
    return ability->GetGamblerUses();
}

const bool Player::GActive() const
{
    return ability->GActive();
}

void Player::SetGActive(bool state)
{
    ability->SetGActive(state);
}

void Player::SetSoothState(bool state)
{
    ability->SetSoothState(state);
}

bool Player::IsSoothActive() const
{
    return ability->IsSoothActive();
}

const size_t Player::GetSoothsayerUses() const
{
    return ability->GetSoothsayerUses();
}

const size_t Player::GetTaxEvaderUses() const
{
    return ability->GetTaxEvaderUses();
}

bool Player::GetSameTurn()
{
    return ability->GetSameTurn();
}

void Player::SetSameTurn(bool sameTurn)
{
    ability->SetSameTurn(sameTurn);
}
