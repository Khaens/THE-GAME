#include "GameServer.h"
#include "Gambler.h"
#include "HarryPotter.h"
#include "PlayerFactory.h"

Game::Game(std::vector<UserModel>& users) : m_numberOfPlayers{ users.size()}
{
	m_players.reserve(m_numberOfPlayers);
	if (users.size() < 2 || users.size() > 5) {
		throw std::invalid_argument("Number of players must be between 2 and 5.");
	}
	
	std::vector<AbilityType> abilities = PlayerFactory::GetRandomUniqueAbilities(users.size());
	size_t i = 0;
	for (auto& user : users) {
		m_players.push_back(PlayerFactory::CreateFromUser(user, abilities[i]));
		i++;
	}

	for (int i = 2; i < 100; i++) {
		Card* newCard = new Card(std::to_string(i));
		Card* newCard2 = new Card(std::to_string(i));
		m_wholeDeck.InsertCard(newCard);
		m_wholeDeck.InsertCard(newCard2);
	}
}

size_t Game::WhoStartsFirst()
{
	std::cout << "\nTake a look at your cards... Who wants to start the game? You have 1 minute to decide.\n";
	std::cout << "Pick a number 1 - " << m_numberOfPlayers << ": ";

	std::packaged_task<size_t()> task([]() {
		size_t choice;
		std::cin >> choice;
		return choice;
		});

	auto future = task.get_future();
	std::thread(std::move(task)).detach();

	if (future.wait_for(std::chrono::seconds(60)) == std::future_status::ready) {
		size_t choice = future.get();
		if (choice >= 1 && choice <= m_numberOfPlayers) {
			std::cout << "Player " << choice << " will start The Game!\n";
			return choice - 1;
		}
		else {
			std::cout << "Invalid number! We will now pick randomly...\n";
		}
	}
	else {
		std::cout << "Uh oh... Time's up! Picking randomly...\n";
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<size_t> dist(1, m_numberOfPlayers);

	size_t randomChoice = dist(gen);
	std::cout << "Player " << randomChoice << " will start The Game!\n";
	return randomChoice - 1;
}

bool Game::IsGameOver(const IPlayer& currentPlayer)
{
	int playableCards = NumberOfPlayableCardsInHand();


	if(m_currentPlayerIndex == m_ctx.HPplayerIndex && currentPlayer.GetHPFlag()) {
		std::cout << "No one played a +/-10 card until " << currentPlayer.GetUsername() << "'s turn. All players lose!\n";
		return true;
	}
	if (playableCards < m_ctx.currentRequired) {
		std::cout << currentPlayer.GetUsername() << " cannot play the required number of cards (" << m_ctx.currentRequired << ").\n";
		std::cout << "Game Over! " << currentPlayer.GetUsername() << " loses!\n";
		return true;
	}


	
	return false;
}

void Game::OneRound(IPlayer& currentPlayer)
{
	std::cout << "A1: " << m_ascPile1.GetTopCard()->GetCardValue() << " | "
		<< "A2: " << m_ascPile2.GetTopCard()->GetCardValue() << " | "
		<< "D1: " << m_descPile1.GetTopCard()->GetCardValue() << " | "
		<< "D2: " << m_descPile2.GetTopCard()->GetCardValue() << "\n";
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
			if (pileChoice == "A1") chosenPile = &m_ascPile1;
			else if (pileChoice == "A2") chosenPile = &m_ascPile2;
			else if (pileChoice == "D1") chosenPile = &m_descPile1;
			else if (pileChoice == "D2") chosenPile = &m_descPile2;
			if (!chosenPile) std::cout << "That's not a valid Pile! Try again!\n";
		}
		if (CanPlaceCard(chosenCard, *chosenPile)) {
			cardPlaced = true;
			if (m_ctx.HPplayerIndex != -1 && m_players[m_ctx.HPplayerIndex]->GetHPFlag()) {
				if (std::stoi(chosenCard->GetCardValue()) == std::stoi(chosenPile->GetTopCard()->GetCardValue()) + 10 ||
					std::stoi(chosenCard->GetCardValue()) == std::stoi(chosenPile->GetTopCard()->GetCardValue()) - 10) {
					m_players[m_ctx.HPplayerIndex]->SetHPFlag(false);
				}
			}
			chosenPile->PlaceCard(chosenCard);
			currentPlayer.RemoveCardFromHand(chosenCard);
			}
		else {
			std::cout << "Cannot place that card on the chosen pile. Try again.\n";
		}
	}
}


void Game::StartGame()
{
	m_wholeDeck.ShuffleDeck();
	FirstRoundDealing();
	m_currentPlayerIndex = WhoStartsFirst();

	while (true) {
		IPlayer& currentPlayer = GetCurrentPlayer();
		if (GetDeckSize() == 0) m_ctx.endgame = true;
		if (m_ctx.endgame) m_ctx.baseRequired = 1;
		else m_ctx.baseRequired = 2;
		if (m_ctx.endgame && m_currentPlayerIndex == m_ctx.GamblerPlayerIndex) {
			if (currentPlayer.GetHand().size() > 1 &&
				currentPlayer.GetGamblerUses() > 0) {
				m_ctx.currentRequired = 2;
			}
		}
		else m_ctx.currentRequired = m_ctx.baseRequired;
		if (currentPlayer.CanUseAbility(m_ctx)) {
			std::cout << "\n" << currentPlayer.GetUsername() << ", do you want to use your ability this turn? (y/n): ";
			char useAbility;
			std::cin >> useAbility;
			if (std::tolower(useAbility) == 'y') {
				currentPlayer.UseAbility(m_ctx, m_currentPlayerIndex);
			}
		}
		if (IsGameOver(currentPlayer)) {
			break;
		}
		ShowCtx();
		int playedCards = 0;
		for (int i = 0; i < m_ctx.currentRequired; i++) {
			OneRound(currentPlayer);
			if (IsGameOver(currentPlayer)) break;
			playedCards++;
		}
		if (m_ctx.GamblerPlayerIndex != -1 && currentPlayer.GActive()) {
			currentPlayer.SetGActive(false);
		}
		else if (m_ctx.TaxEvPlayerIndex != -1 &&
			m_currentPlayerIndex == m_ctx.TaxEvPlayerIndex &&
			currentPlayer.IsTaxActive()) {
			std::cout << "You don't have to play any cards this round!\n";			
		}
		int nrOfPlayableCards = NumberOfPlayableCardsInHand();
		if (nrOfPlayableCards == 0) {
			std::cout << "Nu mai poti pune nici o carte. Trecem la urmatorul jucator.\n";
			for (size_t i = 0; i < playedCards; i++) {
				Card* drawnCard = m_wholeDeck.DrawCard();
				if (drawnCard) currentPlayer.AddCardToHand(drawnCard);
			}
		}
		else {
			while (nrOfPlayableCards > 0) {
				std::cout << "Mai poti pune " << nrOfPlayableCards << " carti.\n";
				std::cout << "Doresti sa continui? (y/n): ";
				char optiune;
				std::cin >> optiune;
				if (optiune == 'n') {
					break;
				}
				else if (optiune == 'y') {
					OneRound(currentPlayer);
					playedCards++;
				}
				nrOfPlayableCards--;
			}
			for (size_t i = 0; i < playedCards; i++) {
				Card* drawnCard = m_wholeDeck.DrawCard();
				if (drawnCard) currentPlayer.AddCardToHand(drawnCard);
			}
			/*currentPlayer.ShowHand();
			m_wholeDeck.ShowDeck();*/
			NextPlayer();
		}
	}
}


void Game::NextPlayer()
{
	m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_numberOfPlayers;
}

IPlayer& Game::GetCurrentPlayer()
{
	return *m_players[m_currentPlayerIndex];
}

void Game::FirstRoundDealing()
{
	
	for (size_t i = 0; i < m_players.size(); i++) {
		for (size_t j = 0; j < 6; j++) {
			Card* dealtCard = m_wholeDeck.DrawCard();
			m_players[i]->AddCardToHand(dealtCard);
		}
	}
}

int Game::NumberOfPlayableCardsInHand()
{
	IPlayer& currentPlayer = GetCurrentPlayer();
	int count = 0;
	for (Card* card : currentPlayer.GetHand()) {
		if ((CanPlaceCard(card, m_ascPile1) || CanPlaceCard(card, m_ascPile2) ||
			CanPlaceCard(card, m_descPile1) || CanPlaceCard(card, m_descPile2)) ||
			(m_ctx.HPplayerIndex != -1 && m_players[m_ctx.HPplayerIndex]->HPActive())) {
			count++;
		}
	}
	return count;
}

bool Game::CanPlaceCard(const Card* card, Pile& pile)
{
	int top = std::stoi(pile.GetTopCard()->GetCardValue());
	int value = std::stoi(card->GetCardValue());
	if (m_ctx.HPplayerIndex != -1
		&& m_players[m_ctx.HPplayerIndex]->HPActive()) return true;

	if (pile.GetPileType()  == PileType::ASCENDING)
		return (value > top) || (value == top - 10);
	else
		return (value < top) || (value == top + 10);
}




Pile* Game::GetPile(const std::string& pileChoice)
{
	if(pileChoice == "A1") return &m_ascPile1;
	else if (pileChoice == "A2") return &m_ascPile2;
	else if (pileChoice == "D1") return &m_descPile1;
	else if (pileChoice == "D2") return &m_descPile2;
	return nullptr;
}


Card* Game::DrawCard()
{
	return m_wholeDeck.DrawCard();
}

size_t Game::GetDeckSize() const
{
	return m_wholeDeck.GetSize();
}

void Game::ShowCtx()
{
	std::cout << "baseReq " << m_ctx.baseRequired << "\n";
	std::cout << "currReq " << m_ctx.currentRequired << "\n";
	std::cout << "HPPIndex " << m_ctx.HPplayerIndex << "\n";
	std::cout << "GambPInd " << m_ctx.GamblerPlayerIndex << "\n";
	std::cout << "TaxEvPInd" << m_ctx.TaxEvPlayerIndex << "\n";
}
