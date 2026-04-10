#pragma once
// import PileServer;
#include "CardServer.h"
#include <array>
#include <memory>
#include <vector>

class Game;
class Pile;

struct CardChoice {
  int cardNum;
  int pileNum;
};

class AlgorithmCardSelection {
public:
  static CardChoice
  ChooseCardAndPile(Game &game, const std::array<Pile *, 4> &piles,
                    const std::vector<std::unique_ptr<Card>> &m_hand);
};
