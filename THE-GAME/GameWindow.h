#pragma once

#include "ui_GameWindow.h"
#include "NetworkManager.h"
#include <QLabel> // Added for QLabel

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
    void onGameConnected();
    void onGameMessageReceived(const QJsonObject& message);

private:
    Ui::Form* ui;
    void loadGameImage();

    QPixmap m_backgroundPixmap;
    QPixmap m_scaledBackground;

    void resizeUI();

    // Chat Functionality
    void toggleChat();
    void sendMessage();
    bool m_isChatOpen = false;

    // Card Selection State
    QWidget* m_selectedCardWidget = nullptr;
    QString m_selectedCardImagePath;

    void sendEndTurnAction();

    // Network & Game State
    NetworkManager* m_networkManager = nullptr;
    int m_userId = -1;
    std::string m_lobbyId;
    std::string m_username;

    void handleGameState(const QJsonObject& state);
    void updateHand(const QJsonArray& hand);
    void updatePiles(const QJsonArray& piles);
    void updateOpponents(const QJsonArray& players);

    QLabel* m_turnLabel = nullptr; // New label for turn indication
    QString m_lastPileTops[4]; // Track pile state to avoid redundant animations
};