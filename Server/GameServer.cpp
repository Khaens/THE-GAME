#include "GameServer.h"

Game::Game(size_t numberOfPlayers) : m_numberOfPlayers{ numberOfPlayers }
{
	m_players.reserve(m_numberOfPlayers);
	if (numberOfPlayers < 2 || numberOfPlayers > 5) {
		throw std::invalid_argument("Number of players must be between 2 and 5.");
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
	Card* firstCard = nullptr;
	Card* secondCard = nullptr;
	std::vector<Card*> playerHand(currentPlayer.GetHand().begin(), currentPlayer.GetHand().end());
	for (Card* card : playerHand) {
		if (m_ascPile1.CanPlaceCard(card) || m_ascPile2.CanPlaceCard(card) ||
			m_descPile1.CanPlaceCard(card) || m_descPile2.CanPlaceCard(card)) {
			if (!firstCard) {
				firstCard = card;
			}
			else {
				secondCard = card;
				break;
			}
		}
	}
	if (m_wholeDeck.IsEmpty() && firstCard) return false;
	else if (!m_wholeDeck.IsEmpty() && firstCard != secondCard) return false;
	return true;
}

void Game::OneRound(IPlayer& currentPlayer)
{

	std::cout << "\nIt's " << currentPlayer.GetUsername() << "'s turn.\n";
	std::cout << "Your hand:\n";
	currentPlayer.ShowHand();
	std::cout << "Pick a card to play from your hand.\n";
	std::string chosenCardValue;
	std::cin >> chosenCardValue;
	Card* chosenCard = currentPlayer.ChooseCard(chosenCardValue);
	std::cout << "Choose a pile to place the card on (A1/A2 for Ascending, D1/D2 for Descending): ";
	std::string pileChoice;
	std::cin >> pileChoice;
	Pile* chosenPile = nullptr;
	if (pileChoice == "A1") chosenPile = &m_ascPile1;
	else if (pileChoice == "A2") chosenPile = &m_ascPile2;
	else if (pileChoice == "D1") chosenPile = &m_descPile1;
	else if (pileChoice == "D2") chosenPile = &m_descPile2;
	chosenPile->PlaceCard(chosenCard);
	currentPlayer.RemoveCardFromHand(chosenCard);
}

void Game::StartGame()
{
	m_wholeDeck.ShuffleDeck();
	FirstRoundDealing();
	m_currentPlayerIndex = WhoStartsFirst();
	size_t minimumCardsNeeded;
	while (!IsGameOver(GetCurrentPlayer())) {
		IPlayer& currentPlayer = GetCurrentPlayer();
		int playedCards = 0;
		if (!m_wholeDeck.IsEmpty()) {
			minimumCardsNeeded = 2;
			OneRound(currentPlayer);
			OneRound(currentPlayer);
			playedCards = 2;
		}
		else if (m_wholeDeck.IsEmpty() || m_wholeDeck.GetSize() == 1) {
			minimumCardsNeeded = 1;
			OneRound(currentPlayer);
			playedCards = 1;
		}


		int nrOfPlayableCards = NumberOfPlayableCardsInHand();
		if (nrOfPlayableCards == 0) {
			std::cout << "Nu mai poti pune nici o carte. Trecem la urmatorul jucator.\n";
			for (size_t i = 0; i < playedCards; i++) {
				Card* drawnCard = m_wholeDeck.DrawCard();
				currentPlayer.AddCardToHand(drawnCard);
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
				currentPlayer.AddCardToHand(drawnCard);
			}
			currentPlayer.ShowHand();
			m_wholeDeck.ShowDeck();
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
	for (size_t i = 0; i < m_numberOfPlayers; i++) {

		m_players.emplace_back(
			std::make_unique<Player<Gambler>>("Player" + std::to_string(i + 1), "password")
		);
		IPlayer& currentPlayer = *m_players[i];
		for (size_t j = 0; j < 6; j++) {
			// Assuming Deck has a method to deal cards
			Card* dealtCard = m_wholeDeck.DrawCard();
			currentPlayer.AddCardToHand(dealtCard);
		}
	}
}

int Game::NumberOfPlayableCardsInHand()
{
	IPlayer& currentPlayer = GetCurrentPlayer();
	int count = 0;
	for (Card* card : currentPlayer.GetHand()) {
		if (m_ascPile1.CanPlaceCard(card) || m_ascPile2.CanPlaceCard(card) ||
			m_descPile1.CanPlaceCard(card) || m_descPile2.CanPlaceCard(card)) {
			count++;
		}
	}
	return count;
}

