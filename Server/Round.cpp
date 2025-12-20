#include "Round.h"

void Round::OneRound(Game& game, TurnContext& m_ctx)
{
	IPlayer& currentPlayer = game.GetCurrentPlayer();
	std::array<Pile*, PILES_AMOUNT> piles = game.GetPiles();
	std::cout << "A1: " << piles[0]->GetTopCard()->GetCardValue() << " | "
		<< "A2: " << piles[1]->GetTopCard()->GetCardValue() << " | "
		<< "D1: " << piles[2]->GetTopCard()->GetCardValue() << " | "
		<< "D2: " << piles[3]->GetTopCard()->GetCardValue() << "\n";
	std::cout << "Your hand:\n";
	currentPlayer.ShowHand();
	bool cardPlaced = false;
	while (!cardPlaced) {
		Card* chosenCard = nullptr;
		std::cout << "Pick a card to play from your hand.\n";
		std::string chosenCardValue;
		std::cin >> chosenCardValue;
		chosenCard = currentPlayer.ChooseCard(chosenCardValue);
		Pile* chosenPile = nullptr;
		while (!chosenPile) {
			std::cout << "Choose a pile to place the card on (A1/A2 for Ascending, D1/D2 for Descending): ";
			std::string pileChoice;
			std::cin >> pileChoice;
			chosenPile = GetPile(pileChoice, piles);
			if (!chosenPile) std::cout << "That's not a valid Pile! Try again!\n";
		}
		if (CanPlaceCard(game, chosenCard, chosenPile, m_ctx)) {
			cardPlaced = true;
			if (m_ctx.HPplayerIndex != -1 && game.GetPlayers()[m_ctx.HPplayerIndex]->HPActive()) {
				if (std::stoi(chosenCard->GetCardValue()) == std::stoi(chosenPile->GetTopCard()->GetCardValue()) + 10 ||
					std::stoi(chosenCard->GetCardValue()) == std::stoi(chosenPile->GetTopCard()->GetCardValue()) - 10) {
					game.GetPlayers()[m_ctx.HPplayerIndex]->SetHPFlag(false);
				}
			}
			chosenPile->PlaceCard(chosenCard);
			if (game.m_gameStats[currentPlayer.GetID()].lastPlayedCardValue != -1) {
				int diff = std::abs(std::stoi(chosenCard->GetCardValue()) - game.m_gameStats[currentPlayer.GetID()].lastPlayedCardValue);
				if (diff > 3) {
					game.m_gameStats[currentPlayer.GetID()].perfectGame = false;
				}
			}
			game.m_gameStats[currentPlayer.GetID()].lastPlayedCardValue = std::stoi(chosenCard->GetCardValue());
			currentPlayer.RemoveCardFromHand(chosenCard);
		}
		else {
			std::cout << "Cannot place that card on the chosen pile. Try again.\n";
		}
	}
}

void Round::FirstRoundDealing(Game& game)
{
	const auto& m_players = game.GetPlayers();
	Deck& deck = game.GetDeck();
	for (size_t i = 0; i < m_players.size(); i++) {
		for (size_t j = 0; j < 6; j++) {
			Card* dealtCard = deck.DrawCard();
			m_players[i]->AddCardToHand(dealtCard);
		}
	}
}

bool Round::CanPlaceCard(Game& game, const Card* card, Pile* pile, TurnContext& m_ctx)
{
	int top = std::stoi(pile->GetTopCard()->GetCardValue());
	int value = std::stoi(card->GetCardValue());
	if (m_ctx.HPplayerIndex != -1
		&& game.GetPlayers()[m_ctx.HPplayerIndex]->HPActive()) return true;

	if (pile->GetPileType() == PileType::ASCENDING)
		return (value > top) || (value == top - 10);
	else
		return (value < top) || (value == top + 10);
}

Pile* Round::GetPile(const std::string& pileChoice, std::array<Pile*, PILES_AMOUNT> piles)
{
	if (pileChoice == "A1") return piles[0];
	else if (pileChoice == "A2") return piles[1];
	else if (pileChoice == "D1") return piles[2];
	else if (pileChoice == "D2") return piles[3];
	return nullptr;
}

int Round::NrOfPlayableCardsInHand(Game& game, TurnContext& m_ctx)
{
	IPlayer& currentPlayer = game.GetCurrentPlayer();
	std::array<Pile*, PILES_AMOUNT> piles = game.GetPiles();
	const auto& m_players = game.GetPlayers();

	int count = 0;
	for (Card* card : currentPlayer.GetHand()) {
		if ((CanPlaceCard(game, card, piles[0], m_ctx) || CanPlaceCard(game, card, piles[1], m_ctx) ||
			CanPlaceCard(game, card, piles[2], m_ctx) || CanPlaceCard(game, card, piles[3], m_ctx)) ||
			(m_ctx.HPplayerIndex != -1 && m_players[m_ctx.HPplayerIndex]->HPActive())) {
			count++;
		}
	}
	return count;
}

void Round::UpdateContext(Game& game, TurnContext& m_ctx, IPlayer& currentPlayer)
{
	if (game.GetDeckSize() == 0) m_ctx.endgame = true;
	if (m_ctx.endgame) m_ctx.baseRequired = 1;
	else m_ctx.baseRequired = 2;
	if (m_ctx.endgame && currentPlayer.GetPlayerIndex() == m_ctx.GamblerPlayerIndex) {
		if (currentPlayer.GetHand().size() > 1 &&
			currentPlayer.GetGamblerUses() > 0) {
			m_ctx.currentRequired = 2;
		}
	}
	else if (m_ctx.TaxEvPlayerIndex != -1 && game.GetPlayers()[m_ctx.TaxEvPlayerIndex]->IsTaxActive() &&
		currentPlayer.GetPlayerIndex() == (m_ctx.TaxEvPlayerIndex + 1) % game.GetPlayers().size()) {
		m_ctx.currentRequired = m_ctx.baseRequired * 2;
		game.GetPlayers()[m_ctx.TaxEvPlayerIndex]->SetTaxActive(false);
	}
	else m_ctx.currentRequired = m_ctx.baseRequired;
}

bool Round::IsGameWon(Game& game, IPlayer& currentPlayer)
{
	if (currentPlayer.GetHand().size() == 0) currentPlayer.SetFinished(true);
	bool gameWon = false;
	for (size_t i = 0; i < game.GetPlayers().size(); i++) {
		if (!game.GetPlayers()[i]->IsFinished()) {
			gameWon = false;
		}
	}
	while (!currentPlayer.IsFinished()) {
		game.NextPlayer();
		currentPlayer = game.GetCurrentPlayer();
	}
	return gameWon;
}

void Round::AbilityUse(Game& game, TurnContext& m_ctx, IPlayer& currentPlayer)
{
	if (currentPlayer.CanUseAbility(m_ctx)) {
		std::cout << "\n" << currentPlayer.GetUsername() << ", do you want to use your ability this turn? (y/n): ";
		char useAbility;
		std::cin >> useAbility;
		if (std::tolower(useAbility) == 'y') {
			currentPlayer.UseAbility(m_ctx, currentPlayer.GetPlayerIndex());
			if (m_ctx.SoothPlayerIndex == currentPlayer.GetPlayerIndex() &&
				currentPlayer.IsSoothActive()) {
				std::cout << "Other player's cards: \n";
				for (size_t i = 0; i < game.GetPlayers().size(); i++) {
					if (i != currentPlayer.GetPlayerIndex()) {
						std::cout << game.GetPlayers()[i]->GetUsername() << ": ";
						game.GetPlayers()[i]->ShowHand();
					}
				}
				currentPlayer.SetSoothState(false);
			}
		}
	}
}
