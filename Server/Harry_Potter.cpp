#include "Harry_Potter.h"
#include "GameServer.h"

void Harry_Potter::UseAbility(Game* game) // special round rules for said ability
{
	if (m_canUse && game->GetDeckSize() == 0) {
		m_canUse = false;
		game->SetHPplayerIndex();
		game->HPused = true;
		std::cout << "Ability used! You get to play any card this round.\n";
		std::cout << "Careful! If no one plays a +/- 10 card until your next turn you all lose!\n";
		std::cout << "Your hand:\n";
		game->GetCurrentPlayer().ShowHand();
		std::cout << "Pick a card to play from your hand.\n";
		std::string chosenCardValue;
		std::cin >> chosenCardValue;
		Card* chosenCard = game->GetCurrentPlayer().ChooseCard(chosenCardValue);
		std::cout << "Choose a pile to place the card on (A1/A2 for Ascending, D1/D2 for Descending): ";
		std::string pileChoice;
		std::cin >> pileChoice;
		game->GetPile(pileChoice)->PlaceCard(chosenCard);
		game->GetCurrentPlayer().RemoveCardFromHand(chosenCard);
		Card* drawnCard = game->DrawCard();
		if (drawnCard) {
			game->GetCurrentPlayer().AddCardToHand(drawnCard);
		}
		game->NextPlayer();
	}
	else if (!m_canUse) {
		std::cout << "Ability already used!\n";
	}
	if(game->Minimum2CardsPlayable()) {
		game->OneRound(game->GetCurrentPlayer());
	}
	game->OneRound(game->GetCurrentPlayer());
}
