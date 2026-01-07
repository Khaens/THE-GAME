#include "Round.h"


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
	try {
        if (!pile || !pile->GetTopCard() || !card) return false;

		std::string pileStr = pile->GetTopCard()->GetCardValue();
		std::string cardStr = card->GetCardValue();
		
		int top = std::stoi(pileStr);
		int value = std::stoi(cardStr);
		if (m_ctx.HPplayerIndex != -1
			&& game.GetPlayers()[m_ctx.HPplayerIndex]->HPActive()) return true;

		if (pile->GetPileType() == PileType::ASCENDING)
			return (value > top) || (value == top - 10);
		else
			return (value < top) || (value == top + 10);
	}
	catch (...) {
		std::cout << "CRASH AVOIDED in CanPlaceCard! Invalid Data." << std::endl;
		return false;
	}
}

Pile* Round::GetPile(int pileChoice, std::array<Pile*, PILES_AMOUNT> piles)
{
	if (pileChoice == 1) return piles[0];
	else if (pileChoice == 2) return piles[1];
	else if (pileChoice == 3) return piles[2];
	else if (pileChoice == 4) return piles[3];
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
	bool gameWon = true;
	for (size_t i = 0; i < game.GetPlayers().size(); i++) {
		if (!game.GetPlayers()[i]->IsFinished()) {
			gameWon = false;
		}
	}
	if (gameWon) return gameWon;
	// Advance to next non-finished player if necessary
	size_t checks = 0;
	size_t numPlayers = game.GetPlayers().size();
	while (game.GetCurrentPlayer().IsFinished() && checks < numPlayers) {
		game.NextPlayer();
		checks++;
	}
	return gameWon;
}
