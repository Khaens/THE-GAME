#include "LobbyRoomDialog.h"
#include <QClipboard>
#include <QApplication>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QFont>

LobbyRoomDialog::LobbyRoomDialog(int userId, const QString& lobbyId,
    const QString& lobbyName, const QString& lobbyCode,
    bool isHost, QWidget* parent)
    : QDialog(parent)
    , m_userId(userId)
    , m_lobbyId(lobbyId)
    , m_lobbyName(lobbyName)
    , m_lobbyCode(lobbyCode)
    , m_isHost(isHost)
    , m_contentContainer(nullptr)
    , m_networkManager(nullptr)
    , m_refreshTimer(nullptr)
    , m_countdownTimer(nullptr)
    , m_countdownSeconds(60)  // Start at 60 seconds
    , m_playerCountLabel(nullptr)
    , m_countdownLabel(nullptr)
    , m_countdownBar(nullptr)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    // Initialize network manager
    m_networkManager = new NetworkManager("http://localhost:18080");

    setupUI();
    setupStyle();
    setupConnections();
    startRefreshTimer();
    startCountdownTimer();  // Start the 60-second countdown
}

void LobbyRoomDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 15, 30, 15);

    // Content container
    m_contentContainer = new QWidget(this);
    m_contentContainer->setObjectName("lobbyRoomContainer");
    m_contentContainer->setFixedSize(700, 650);  // Increased height for countdown

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(15);
    containerLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    m_titleLabel = new QLabel("LOBBY ROOM", m_contentContainer);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #f3d05a; margin-bottom: 10px;");
    containerLayout->addWidget(m_titleLabel);

    // Lobby Info Section
    QWidget* infoWidget = new QWidget(m_contentContainer);
    infoWidget->setObjectName("infoWidget");
    QGridLayout* infoLayout = new QGridLayout(infoWidget);
    infoLayout->setContentsMargins(20, 15, 20, 15);
    infoLayout->setVerticalSpacing(10);
    infoLayout->setHorizontalSpacing(20);

    // Lobby Name
    QLabel* nameTitle = new QLabel("Lobby Name:", infoWidget);
    nameTitle->setStyleSheet("font-size: 16px; color: #ffffff; font-weight: bold;");

    m_lobbyNameLabel = new QLabel(m_lobbyName, infoWidget);
    m_lobbyNameLabel->setStyleSheet(R"(
        QLabel {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 12px;
            font-size: 16px;
            font-weight: bold;
            color: #2C3E50;
            min-height: 25px;
        }
    )");

    // Lobby Code
    QLabel* codeTitle = new QLabel("Lobby Code:", infoWidget);
    codeTitle->setStyleSheet("font-size: 16px; color: #ffffff; font-weight: bold;");

    m_lobbyCodeLabel = new QLabel(m_lobbyCode, infoWidget);
    m_lobbyCodeLabel->setAlignment(Qt::AlignCenter);
    m_lobbyCodeLabel->setStyleSheet(R"(
        QLabel {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 12px;
            font-size: 24px;
            font-weight: bold;
            letter-spacing: 8px;
            color: #2C3E50;
            min-width: 200px;
            min-height: 25px;
        }
    )");

    m_copyCodeButton = new QPushButton("Copy", infoWidget);
    m_copyCodeButton->setFixedSize(100, 45);
    m_copyCodeButton->setCursor(Qt::PointingHandCursor);
    m_copyCodeButton->setStyleSheet(R"(
        QPushButton {
            background-color: #654b1f;
            color: #f3d05a;
            border: 2px solid #8e273b;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4a3f1e;
            border: 2px solid #f3d05a;
        }
        QPushButton:pressed {
            background-color: #3d431a;
        }
    )");

    infoLayout->addWidget(nameTitle, 0, 0);
    infoLayout->addWidget(m_lobbyNameLabel, 0, 1);
    infoLayout->addWidget(codeTitle, 1, 0);

    QHBoxLayout* codeLayout = new QHBoxLayout();
    codeLayout->setSpacing(10);
    codeLayout->addWidget(m_lobbyCodeLabel);
    codeLayout->addWidget(m_copyCodeButton);
    infoLayout->addLayout(codeLayout, 1, 1);

    containerLayout->addWidget(infoWidget);

    // NEW: Countdown Section
    m_countdownLabel = new QLabel("Game starts in: 60s", m_contentContainer);
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    m_countdownLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #f3d05a;");
    containerLayout->addWidget(m_countdownLabel);

    m_countdownBar = new QProgressBar(m_contentContainer);
    m_countdownBar->setRange(0, 60);
    m_countdownBar->setValue(60);
    m_countdownBar->setTextVisible(false);
    m_countdownBar->setFixedHeight(20);
    m_countdownBar->setStyleSheet(R"(
        QProgressBar {
            border: 2px solid #654b1f;
            border-radius: 10px;
            background-color: rgba(101, 75, 31, 0.3);
        }
        QProgressBar::chunk {
            background-color: #f3d05a;
            border-radius: 8px;
        }
    )");
    containerLayout->addWidget(m_countdownBar);

    // Players List Section
    QLabel* playersTitle = new QLabel("PLAYERS", m_contentContainer);
    playersTitle->setAlignment(Qt::AlignCenter);
    playersTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #f3d05a; margin-top: 10px;");
    containerLayout->addWidget(playersTitle);

    m_playerListWidget = new QListWidget(m_contentContainer);
    m_playerListWidget->setStyleSheet(R"(
        QListWidget {
            background-color: rgba(222, 175, 17, 0.1);
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 10px;
            color: #ffffff;
            font-size: 16px;
            font-weight: bold;
        }
        QListWidget::item {
            background-color: rgba(101, 75, 31, 0.7);
            border-radius: 6px;
            margin: 3px;
            padding: 12px;
            border: 1px solid #deaf11;
        }
        QListWidget::item:hover {
            background-color: rgba(101, 75, 31, 0.9);
            border: 1px solid #f3d05a;
        }
        QListWidget::item:selected {
            background-color: #f3d05a;
            color: #2C3E50;
            border: 1px solid #f3d05a;
        }
    )");
    m_playerListWidget->setMinimumHeight(180);
    containerLayout->addWidget(m_playerListWidget);

    // Player Count
    m_playerCountLabel = new QLabel("Players: 1/4", m_contentContainer);
    m_playerCountLabel->setAlignment(Qt::AlignCenter);
    m_playerCountLabel->setStyleSheet("font-size: 14px; color: #f3d05a; font-style: italic; margin-top: 5px;");
    containerLayout->addWidget(m_playerCountLabel);

    containerLayout->addStretch();

    // Buttons Section - ONLY LEAVE and START GAME (for host)
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    m_startGameButton = new QPushButton("START GAME", m_contentContainer);
    m_startGameButton->setFixedSize(200, 45);
    m_startGameButton->setCursor(Qt::PointingHandCursor);
    m_startGameButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #e3b310;
        }
        QPushButton:pressed {
            background-color: #869e22;
        }
        QPushButton:disabled {
            background-color: #654b1f;
            color: #8e7b4a;
        }
    )");
    m_startGameButton->setVisible(m_isHost);
    m_startGameButton->setEnabled(false); // Disabled until we have at least 2 players

    m_leaveButton = new QPushButton("LEAVE LOBBY", m_contentContainer);
    m_leaveButton->setFixedSize(200, 45);
    m_leaveButton->setCursor(Qt::PointingHandCursor);
    m_leaveButton->setStyleSheet(R"(
        QPushButton {
            background-color: #e03434;
            color: #ffffff;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #b82a2a;
        }
        QPushButton:pressed {
            background-color: #a12525;
        }
    )");

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_startGameButton);
    buttonLayout->addWidget(m_leaveButton);
    buttonLayout->addStretch();

    containerLayout->addLayout(buttonLayout);

    // Center the container
    QVBoxLayout* centerLayout = new QVBoxLayout();
    QHBoxLayout* horizontalCenter = new QHBoxLayout();
    horizontalCenter->addStretch();
    horizontalCenter->addWidget(m_contentContainer);
    horizontalCenter->addStretch();

    centerLayout->addStretch();
    centerLayout->addLayout(horizontalCenter);
    centerLayout->addStretch();

    mainLayout->addLayout(centerLayout);
}

void LobbyRoomDialog::setupStyle()
{
    // Match the same style as CreateLobbyDialog
    m_contentContainer->setStyleSheet(R"(
        #lobbyRoomContainer {
            background-color: transparent;
            border-image: url(Resources/TextBox_1-2.png);
        }
        #infoWidget {
            background-color: rgba(101, 75, 31, 0.3);
            border: 2px solid #654b1f;
            border-radius: 10px;
        }
    )");
}

void LobbyRoomDialog::setupConnections()
{
    connect(m_startGameButton, &QPushButton::clicked, this, &LobbyRoomDialog::onStartGameClicked);
    connect(m_leaveButton, &QPushButton::clicked, this, &LobbyRoomDialog::onLeaveClicked);
    connect(m_copyCodeButton, &QPushButton::clicked, this, &LobbyRoomDialog::onCopyCodeClicked);
}

void LobbyRoomDialog::startRefreshTimer()
{
    if (!m_refreshTimer) {
        m_refreshTimer = new QTimer(this);
        connect(m_refreshTimer, &QTimer::timeout, this, &LobbyRoomDialog::refreshLobbyStatus);
    }
    m_refreshTimer->start(2000); // Refresh every 2 seconds
}

void LobbyRoomDialog::stopRefreshTimer()
{
    if (m_refreshTimer && m_refreshTimer->isActive()) {
        m_refreshTimer->stop();
    }
}

// NEW: Start the 60-second countdown
void LobbyRoomDialog::startCountdownTimer()
{
    if (!m_countdownTimer) {
        m_countdownTimer = new QTimer(this);
        connect(m_countdownTimer, &QTimer::timeout, this, &LobbyRoomDialog::updateCountdown);
    }
    m_countdownTimer->start(1000); // Update every second
}

// NEW: Stop the countdown
void LobbyRoomDialog::stopCountdownTimer()
{
    if (m_countdownTimer && m_countdownTimer->isActive()) {
        m_countdownTimer->stop();
    }
}

// NEW: Update countdown every second
void LobbyRoomDialog::updateCountdown()
{
    m_countdownSeconds--;

    // Update label
    m_countdownLabel->setText(QString("Game starts in: %1s").arg(m_countdownSeconds));

    // Update progress bar
    m_countdownBar->setValue(m_countdownSeconds);

    // Change color when getting close to 0
    if (m_countdownSeconds <= 10) {
        m_countdownLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #e03434;");
        m_countdownBar->setStyleSheet(R"(
            QProgressBar {
                border: 2px solid #654b1f;
                border-radius: 10px;
                background-color: rgba(101, 75, 31, 0.3);
            }
            QProgressBar::chunk {
                background-color: #e03434;
                border-radius: 8px;
            }
        )");
    }
    else if (m_countdownSeconds <= 30) {
        m_countdownLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #e67e22;");
        m_countdownBar->setStyleSheet(R"(
            QProgressBar {
                border: 2px solid #654b1f;
                border-radius: 10px;
                background-color: rgba(101, 75, 31, 0.3);
            }
            QProgressBar::chunk {
                background-color: #e67e22;
                border-radius: 8px;
            }
        )");
    }

    // When countdown reaches 0, start game automatically
    if (m_countdownSeconds <= 0) {
        stopCountdownTimer();

        // Only auto-start if we have at least 2 players
        if (m_players.size() >= 2) {
            onStartGameClicked();
        }
        else {
            // Reset countdown if not enough players
            m_countdownSeconds = 60;
            m_countdownLabel->setText("Game starts in: 60s (Need more players)");
            m_countdownLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #f3d05a;");
            m_countdownBar->setValue(60);
            m_countdownBar->setStyleSheet(R"(
                QProgressBar {
                    border: 2px solid #654b1f;
                    border-radius: 10px;
                    background-color: rgba(101, 75, 31, 0.3);
                }
                QProgressBar::chunk {
                    background-color: #f3d05a;
                    border-radius: 8px;
                }
            )");
            startCountdownTimer(); // Restart the countdown
        }
    }
}

void LobbyRoomDialog::onLeaveClicked()
{
    stopRefreshTimer();
    stopCountdownTimer();

    // TODO: Send leave request to server
    // m_networkManager->leaveLobby(m_userId, m_lobbyId.toStdString());

    reject(); // Close the dialog
}

void LobbyRoomDialog::onStartGameClicked()
{
    if (!m_isHost && m_countdownSeconds > 0) {
        return; // Only host can start manually before countdown ends
    }

    // Check if we have at least 2 players
    if (m_players.size() < 2) {
        QMessageBox::warning(this, "Not Enough Players",
            "Need at least 2 players to start the game.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::Yes;

    // Only show confirmation if host is starting manually
    if (m_isHost && m_countdownSeconds > 0) {
        reply = QMessageBox::question(this, "Start Game",
            "Are you sure you want to start the game now?",
            QMessageBox::Yes | QMessageBox::No);
    }

    if (reply == QMessageBox::Yes) {
        stopRefreshTimer();
        stopCountdownTimer();

        // TODO: Send start game request to server
        // bool success = m_networkManager->startGame(m_lobbyId.toStdString());

        accept(); // Close dialog and start game
    }
}

void LobbyRoomDialog::onCopyCodeClicked()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(m_lobbyCode);

    // Visual feedback with matching colors
    QString originalText = m_copyCodeButton->text();
    m_copyCodeButton->setText("Copied!");
    m_copyCodeButton->setStyleSheet(R"(
        QPushButton {
            background-color: #27AE60;
            color: #ffffff;
            border: 2px solid #27AE60;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
        }
    )");

    QTimer::singleShot(1500, [this, originalText]() {
        m_copyCodeButton->setText(originalText);
        m_copyCodeButton->setStyleSheet(R"(
            QPushButton {
                background-color: #654b1f;
                color: #f3d05a;
                border: 2px solid #8e273b;
                border-radius: 8px;
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #4a3f1e;
                border: 2px solid #f3d05a;
            }
            QPushButton:pressed {
                background-color: #3d431a;
            }
        )");
        });
}

void LobbyRoomDialog::refreshLobbyStatus()
{
    fetchLobbyStatus();
}

void LobbyRoomDialog::fetchLobbyStatus()
{
    auto statusOpt = m_networkManager->getLobbyStatus(m_lobbyId.toStdString());

    if (statusOpt.has_value()) {
        handleLobbyUpdate(statusOpt.value());
    }
}

void LobbyRoomDialog::handleLobbyUpdate(const LobbyStatus& status)
{
    // Update player count label
    if (m_playerCountLabel) {
        m_playerCountLabel->setText(QString("Players: %1/%2")
            .arg(status.current_players)
            .arg(status.max_players));
    }

    // Update player list (for demo, create dummy data)
    if (m_players.isEmpty() || m_players.size() != status.current_players) {
        m_players.clear();

        // Add current user
        PlayerInfo user;
        user.userId = m_userId;
        user.username = QString("Player_%1").arg(m_userId);
        user.isHost = m_isHost;
        m_players.append(user);

        // Add other players
        for (int i = 1; i < status.current_players; i++) {
            PlayerInfo player;
            player.userId = m_userId + i;
            player.username = QString("Player_%1").arg(m_userId + i);
            player.isHost = false;
            m_players.append(player);
        }

        // Update START GAME button state
        if (m_isHost) {
            m_startGameButton->setEnabled(m_players.size() >= 2);
        }
    }

    updatePlayerList(m_players);

    // Check if game has started
    if (status.game_started) {
        stopRefreshTimer();
        stopCountdownTimer();
        QMessageBox::information(this, "Game Starting",
            "Game is starting! Transitioning to game screen...");
        accept();
    }
}

void LobbyRoomDialog::updatePlayerList(const QVector<PlayerInfo>& players)
{
    m_playerListWidget->clear();

    for (const auto& player : players) {
        QString playerText = player.username;

        // Add host indicator
        if (player.isHost) {
            playerText += " [HOST]";
        }

        QListWidgetItem* item = new QListWidgetItem(playerText, m_playerListWidget);
        item->setTextAlignment(Qt::AlignCenter);

        // Set different colors based on status
        if (player.userId == m_userId) {
            // Current user - highlighted
            item->setBackground(QColor(243, 208, 90, 50));
            item->setForeground(QColor(255, 255, 255));
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
        }
        else if (player.isHost) {
            // Host - gold color
            item->setForeground(QColor(243, 208, 90));
        }
        else {
            // Other players - white
            item->setForeground(QColor(255, 255, 255));
        }
    }
}

void LobbyRoomDialog::setLobbyInfo(const QString& name, const QString& code)
{
    m_lobbyName = name;
    m_lobbyCode = code;
    m_lobbyNameLabel->setText(name);
    m_lobbyCodeLabel->setText(code);
}