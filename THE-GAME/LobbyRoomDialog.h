#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTimer>
#include <QProgressBar>
#include "NetworkManager.h"

struct PlayerInfo {
    int userId;
    QString username;
    bool isHost;
};

class LobbyRoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LobbyRoomDialog(int userId, const QString& lobbyId,
        const QString& lobbyName, const QString& lobbyCode,
        bool isHost, QWidget* parent = nullptr);
    ~LobbyRoomDialog() = default;

    void updatePlayerList(const QVector<PlayerInfo>& players);
    void setLobbyInfo(const QString& name, const QString& code);

private slots:
    void onLeaveClicked();
    void onStartGameClicked();
    void onCopyCodeClicked();
    void refreshLobbyStatus();
    void updateCountdown();  // NEW: For the 60-second timer

private:
    void setupUI();
    void setupStyle();
    void setupConnections();
    void startRefreshTimer();
    void stopRefreshTimer();
    void startCountdownTimer();  // NEW: Start the 60-second countdown
    void stopCountdownTimer();   // NEW: Stop the countdown

    void fetchLobbyStatus();
    void handleLobbyUpdate(const LobbyStatus& status);

    // UI Elements
    QWidget* m_contentContainer;
    QLabel* m_titleLabel;
    QLabel* m_lobbyNameLabel;
    QLabel* m_lobbyCodeLabel;
    QPushButton* m_copyCodeButton;
    QListWidget* m_playerListWidget;
    QPushButton* m_startGameButton;
    QPushButton* m_leaveButton;
    QLabel* m_playerCountLabel;
    QLabel* m_countdownLabel;     // NEW: Shows "Starting in 60s"
    QProgressBar* m_countdownBar; // NEW: Visual countdown progress bar

    // Data
    int m_userId;
    QString m_lobbyId;
    QString m_lobbyName;
    QString m_lobbyCode;
    bool m_isHost;

    // Timers
    NetworkManager* m_networkManager;
    QTimer* m_refreshTimer;
    QTimer* m_countdownTimer;  // NEW: 60-second countdown timer
    int m_countdownSeconds;    // NEW: Current countdown value (60 -> 0)

    // Player management
    QVector<PlayerInfo> m_players;
};