#pragma once

#include "ui_GameWindow.h"
#include "NetworkManager.h"
#include <QLabel>
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

    void toggleChat();
    void sendMessage();
    bool m_isChatOpen = false;
    QTextEdit* m_chatDisplay = nullptr;

    QWidget* m_selectedCardWidget = nullptr;
    QString m_selectedCardImagePath;

    void sendEndTurnAction();

    NetworkManager* m_networkManager = nullptr;
    int m_userId = -1;
    std::string m_lobbyId;
    std::string m_username;
    bool m_wasSoothActive = false;

    void handleGameState(const QJsonObject& state);
    void updateHand(const QJsonArray& hand);
    void updatePiles(const QJsonArray& piles);
    void updateSoothsayerView(const QJsonArray& players);
    void hideSoothsayerPanels();

    QRect scaleRect(int x, int y, int w, int h) const;

    QLabel* m_turnLabel = nullptr;
    int m_lastTurnUserId = -1;
    QString m_lastPileTops[4];

    bool m_isMyTurn = false;
    bool m_isHPMode = false;
    int m_cardsPlayedThisTurn = 0; 
    int m_pileTopValues[4] = { 1, 1, 100, 100 }; 
    int m_deckCount = 0;
    int m_requiredCards = 2; 
    bool m_canUseAbility = true; 
    QString m_myAbilityName;
    int m_gamblerUsesLeft = 3; 
    int m_taxEvaderUsesLeft = 2; 
    bool m_abilityUsedThisTurn = false; 

    bool canPlaceCardOnPile(int cardValue, int pileIndex) const;
    void updatePileClickability();
    void clearCardSelection();
    void updateEndTurnButtonState();
    void updateAbilityButtonState();
    
    void resetGameState();
};