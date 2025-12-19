#pragma once
#include "GameServer.h"

class Game;
struct TurnContext;
class IPlayer;
class Pile;
class Card;

#include <string>
#include <vector>

class Round
{
public:
	static void OneRound(Game& game, TurnContext& m_ctx);
	static void FirstRoundDealing(Game& game);
	static bool CanPlaceCard(Game& game, const Card* card, Pile* pile, TurnContext& m_ctx);
	static Pile* GetPile(int pileChoice, std::array<Pile*, PILES_AMOUNT> piles);
	static int NrOfPlayableCardsInHand(Game& game, TurnContext& m_ctx);
	static void UpdateContext(Game& game, TurnContext& m_ctx, IPlayer& currentPlayer);
	static bool IsGameWon(Game& game, IPlayer& currentPlayer);
};

