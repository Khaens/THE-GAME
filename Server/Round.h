#pragma once
#include "GameServer.h"
#include <string>
#include <vector>

constexpr size_t STARTING_HAND_SIZE = 6;

class Game;
struct TurnContext;
class IPlayer;
class Pile;
class Card;


class Round
{
public:
	static void FirstRoundDealing(Game& game);
	static bool CanPlaceCard(Game& game, const Card* card, Pile* pile, TurnContext& m_ctx);
	static Pile* GetPile(int pileChoice, const std::array<std::unique_ptr<Pile>, PILES_AMOUNT>& piles);
	static int GetNrOfPlayableCardsInHand(Game& game, TurnContext& m_ctx);
	static void UpdateContext(Game& game, TurnContext& m_ctx, IPlayer& currentPlayer);
	static bool IsGameWon(Game& game, IPlayer& currentPlayer);
};

