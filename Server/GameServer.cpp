#include "GameServer.h"
#include "Gambler.h"
#include "Harry_Potter.h"


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
	if(HPused && m_currentPlayerIndex == HPplayerIndex) {
		std::cout << "No one played a +/-10 card until " << currentPlayer.GetUsername() << "'s turn. All players lose!\n";
		return true;
	}
	Card* firstCard = nullptr;
	Card* secondCard = nullptr;
	std::vector<Card*> playerHand(currentPlayer.GetHand().begin(), currentPlayer.GetHand().end());
	for (Card* card : playerHand) {
		if (CanPlaceCard(card, m_ascPile1) || CanPlaceCard(card,m_ascPile2) ||
			CanPlaceCard(card, m_descPile1) || CanPlaceCard(card, m_descPile2)) {
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
	std::cout << "A1: " << m_ascPile1.GetTopCard()->GetCardValue() << " | "
		<< "A2: " << m_ascPile2.GetTopCard()->GetCardValue() << " | "
		<< "D1: " << m_descPile1.GetTopCard()->GetCardValue() << " | "
		<< "D2: " << m_descPile2.GetTopCard()->GetCardValue() << "\n";
	std::cout << "Your hand:\n";
	currentPlayer.ShowHand();
	bool cardPlaced = false;
	while (!cardPlaced) {
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
		if (std::abs(std::stoi(chosenCard->GetCardValue()) -
			std::stoi(chosenPile->GetTopCard()->GetCardValue())) == 10) {
			HPused = false;
		}
		if(CanPlaceCard(chosenCard, *chosenPile)) {
			cardPlaced = true;
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
	size_t minimumCardsNeeded;
	while (!IsGameOver(GetCurrentPlayer())) {
		IPlayer& currentPlayer = GetCurrentPlayer();
		std::cout << "\nIt's " << currentPlayer.GetUsername() << "'s turn.\n";
		
		int playedCards = 0;
		bool abilityUsed = false;
		std::cout << "\n" << currentPlayer.GetUsername() << ", do you want to use your ability this turn? (y/n): ";
		char useAbility;
		std::cin >> useAbility;
		if (std::tolower(useAbility) == 'y') {
			currentPlayer.UseAbility(this);
			abilityUsed = true;
		}
		else {
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
	for (size_t i = 0; i < m_numberOfPlayers - 1; i++) {

		m_players.emplace_back(
			std::make_unique<Player<Gambler>>("Player" + std::to_string(i + 1), "password")
		);
	}
	m_players.push_back(
		std::make_unique<Player<Harry_Potter>>(
			"Player" + std::to_string(m_numberOfPlayers), "password"
		)
	);
	
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
		if (CanPlaceCard(card, m_ascPile1) || CanPlaceCard(card, m_ascPile2) ||
			CanPlaceCard(card, m_descPile1) || CanPlaceCard(card, m_descPile2)) {
			count++;
		}
	}
	return count;
}

bool Game::CanPlaceCard(const Card* card, Pile& pile)
{
	int top = std::stoi(pile.GetTopCard()->GetCardValue());
	int value = std::stoi(card->GetCardValue());

	if (pile.GetPileType()  == PileType::ASCENDING)
		return (value > top) || (value == top - 10);
	else
		return (value < top) || (value == top + 10);
}

bool Game::Minimum2CardsPlayable()
{
	if (m_wholeDeck.IsEmpty() || m_wholeDeck.GetSize() == 1) {
		return false;
	}
	return true;
}

Pile* Game::GetPile(const std::string& pileChoice)
{
	if(pileChoice == "A1") return &m_ascPile1;
	else if (pileChoice == "A2") return &m_ascPile2;
	else if (pileChoice == "D1") return &m_descPile1;
	else if (pileChoice == "D2") return &m_descPile2;
	return nullptr;
}

void Game::SetHPplayerIndex()
{
	HPplayerIndex = m_currentPlayerIndex;
}

Card* Game::DrawCard()
{
	return m_wholeDeck.DrawCard();
}

size_t Game::GetDeckSize() const
{
	return m_wholeDeck.GetSize();
}
