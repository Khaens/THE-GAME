#pragma once

#include "ui_GameWindow.h"
#include "NetworkManager.h"
#include <QLabel> // Added for QLabel
#include <QTextEdit>
#include "WinDialog.h"
#include "LossDialog.h"

class GameWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget* parent = nullptr);
    ~GameWindow();
    void showOverlay();
    void hideOverlay();

    // Game Logic
    void initialize(NetworkManager* networkManager, int userId, const std::string& lobbyId);

signals:
    void backToMenuRequested();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onBackButtonClicked();
    void onAbilityButtonClicked();
    void onGameConnected();
    void onGameMessageReceived(const QJsonObject& message);

private:
    Ui::Form* ui;
    void loadGameImage();

    QPixmap m_backgroundPixmap;

    void resizeUI();

    // Chat Functionality
    void toggleChat();
    void sendMessage();
    bool m_isChatOpen = false;
    QTextEdit* m_chatDisplay = nullptr;

    // Card Selection State
    QWidget* m_selectedCardWidget = nullptr;
    QString m_selectedCardImagePath;

    void sendEndTurnAction();

    // Network & Game State
    NetworkManager* m_networkManager = nullptr;
    int m_userId = -1;
    std::string m_lobbyId;
    std::string m_username;
    bool m_wasSoothActive = false;

    void handleGameState(const QJsonObject& state);
    void updateHand(const QJsonArray& hand);
    void updatePiles(const QJsonArray& piles);
    void updateOpponents(const QJsonArray& players);
    void updateSoothsayerView(const QJsonArray& players);
    void hideSoothsayerPanels();

    // Helper for UI scaling
    QRect scaleRect(int x, int y, int w, int h) const;

    QLabel* m_turnLabel = nullptr; // New label for turn indication
    int m_lastTurnUserId = -1;
    QString m_lastPileTops[4]; // Track pile state to avoid redundant animations

    // Turn and Pile State for Client-Side Validation
    bool m_isMyTurn = false;
    bool m_isHPMode = false;
    int m_cardsPlayedThisTurn = 0; // Track number of cards played in current turn
    int m_pileTopValues[4] = { 1, 1, 100, 100 }; // Current top card values [asc1, asc2, desc1, desc2]
    int m_deckCount = 0; // Updated from server state. Default 0 safe for start.
    int m_requiredCards = 2; // Server dictates this (2 normally, 1 for Gambler, 0 for TaxEvader, etc)
    bool m_canUseAbility = true; // Server tells us if ability can be used
    QString m_myAbilityName; // Track our ability type
    int m_gamblerUsesLeft = 3; // Gambler uses remaining
    int m_taxEvaderUsesLeft = 2; // TaxEvader uses remaining
    bool m_abilityUsedThisTurn = false; // Client-side: block after one use per turn

    bool canPlaceCardOnPile(int cardValue, int pileIndex) const;
    void updatePileClickability();
    void clearCardSelection();
    void updateEndTurnButtonState();
    void updateAbilityButtonState();
    
    // Helper to reset state
    void resetGameState();
};