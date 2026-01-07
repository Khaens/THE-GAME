#include "LobbyRoomDialog.h"
#include "GameWindow.h"
#include <QClipboard>
#include <QApplication>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QFont>
#include <QPointer>

LobbyRoomDialog::LobbyRoomDialog(int userId, const QString& lobbyId,
    const QString& lobbyName, const QString& lobbyCode,
    bool isHost, NetworkManager* networkManager, QWidget* parent)
    : QDialog(parent)
    , m_userId(userId)
    , m_lobbyId(lobbyId)
    , m_lobbyName(lobbyName)
    , m_lobbyCode(lobbyCode)
    , m_isHost(isHost)
    , m_maxPlayers(4)
    , m_contentContainer(nullptr)
    , m_networkManager(networkManager)
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

    // NetworkManager is now passed in, no need to create new one

    setupUI();
    setupStyle();
    setupConnections();
    
    // Connect to lobby WebSocket for real-time updates
    connect(m_networkManager, &NetworkManager::lobbyConnected, this, [this]() {
        qDebug() << "Connected to lobby WebSocket";
        // Fetch initial player list
        fetchLobbyPlayers();
    });
    
    connect(m_networkManager, &NetworkManager::lobbyMessageReceived, this, 
        &LobbyRoomDialog::handleLobbyWebSocketMessage);
    
    // Connect to lobby WebSocket
    m_networkManager->connectToLobby(m_lobbyId.toStdString());
    
    // Also fetch initial status
    fetchLobbyStatus();
    fetchLobbyPlayers();
    
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
    // Use longer interval since we have WebSocket for real-time updates
    m_refreshTimer->start(5000); // Refresh every 5 seconds (just as backup)
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

        // Only auto-start if we have at least 1 players (Temporary)
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
    
    // Disconnect from lobby WebSocket
    m_networkManager->disconnectFromLobby();

    // Send leave request to server
    m_networkManager->leaveLobby(m_userId, m_lobbyId.toStdString());

    reject(); // Close the dialog
}

void LobbyRoomDialog::onStartGameClicked()
{
    if (!m_isHost && m_countdownSeconds > 0) {
        return;
    }

    // TEMPORARY: Allow 1 player for testing
    if (m_players.size() < 2) {
        QMessageBox::warning(this, "Not Enough Players",
            "Need at least 2 players to start the game."); 
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::Yes;

    if (m_isHost && m_countdownSeconds > 0) {
        reply = QMessageBox::question(this, "Start Game",
            "Are you sure you want to start the game now?",
            QMessageBox::Yes | QMessageBox::No);
    }

    if (reply == QMessageBox::Yes) {
        stopRefreshTimer();
        stopCountdownTimer();
        
        // Start game on server
        // Try to start on server, but if it fails (e.g. not enough players), 
        // proceed client-side anyway for testing purposes.
        /* ORIGINAL:
        if (m_networkManager->startGame(m_lobbyId.toStdString())) {
            m_networkManager->disconnectFromLobby();
            emit gameStarted(); // EMIT SIGNAL INSTEAD
            accept(); // Close lobby dialog
        } else {
            QMessageBox::warning(this, "Error", "Failed to start game. Please try again.");
        }
        */
        
        if (m_networkManager->startGame(m_lobbyId.toStdString())) {
             m_networkManager->disconnectFromLobby();
             emit gameStarted(m_lobbyId);
             accept(); // Close lobby dialog
        } else {
             QMessageBox::warning(this, "Error", "Failed to start game. Please try again.");
        }
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

    // Use QPointer to safely check if dialog still exists
    QPointer<LobbyRoomDialog> dialogPtr(this);
    QPointer<QPushButton> buttonPtr(m_copyCodeButton);
    
    QTimer::singleShot(1500, [dialogPtr, buttonPtr, originalText]() {
        // Check if dialog and button still exist before accessing
        if (dialogPtr && buttonPtr) {
            buttonPtr->setText(originalText);
            buttonPtr->setStyleSheet(R"(
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
        }
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

void LobbyRoomDialog::fetchLobbyPlayers()
{
    auto players = m_networkManager->getLobbyPlayers(m_lobbyId.toStdString());
    
    m_players.clear();
    for (const auto& player : players) {
        PlayerInfo info;
        info.userId = player.user_id;
        info.username = QString::fromStdString(player.username);
        info.isHost = player.is_host;
        m_players.append(info);
    }
    
    // Update player count label
    if (m_playerCountLabel) {
        m_playerCountLabel->setText(QString("Players: %1/%2")
            .arg(m_players.size())
            .arg(m_maxPlayers));
    }
    
    // Update START GAME button state
    if (m_isHost) {
        m_startGameButton->setEnabled(m_players.size() >= 2);
    }
    
    updatePlayerList(m_players);
}

void LobbyRoomDialog::handleLobbyUpdate(const LobbyStatus& status)
{
    // Store max_players for later use
    m_maxPlayers = status.max_players;

    // Update player count label
    if (m_playerCountLabel) {
        m_playerCountLabel->setText(QString("Players: %1/%2")
            .arg(status.current_players)
            .arg(status.max_players));
    }

    // Fetch updated player list when status changes
    fetchLobbyPlayers();

    // Check if game has started
    // Check if game has started
    if (status.game_started) {
        stopRefreshTimer();
        stopCountdownTimer();
        m_networkManager->disconnectFromLobby();
        // Removed intrusive popup as per user request
        emit gameStarted(m_lobbyId);
        accept(); 
    }
}

void LobbyRoomDialog::handleLobbyWebSocketMessage(const QJsonObject& message)
{
    QString type = message["type"].toString();
    
    if (type == "player_joined") {
        qDebug() << "Player joined via WebSocket";
        int user_id = message["user_id"].toInt();
        QString username = message["username"].toString();
        int current_players = message["current_players"].toInt();
        int max_players = message["max_players"].toInt();
        
        // Update player count
        if (m_playerCountLabel) {
            m_playerCountLabel->setText(QString("Players: %1/%2")
                .arg(current_players)
                .arg(max_players));
        }
        
        // Fetch updated player list to get all players with correct info
        fetchLobbyPlayers();
        
        // Also update status
        fetchLobbyStatus();
    }
    else if (type == "lobby_state") {
        // Initial state received
        int current_players = message["current_players"].toInt();
        int max_players = message["max_players"].toInt();
        bool game_started = message["game_started"].toBool();
        
        if (m_playerCountLabel) {
            m_playerCountLabel->setText(QString("Players: %1/%2")
                .arg(current_players)
                .arg(max_players));
        }
        
        if (game_started) {
            stopRefreshTimer();
            stopCountdownTimer();
            m_networkManager->disconnectFromLobby();
            QMessageBox::information(this, "Game Starting",
                "Game is starting! Transitioning to game screen...");
            emit gameStarted(m_lobbyId);
            accept();
        } else {
            fetchLobbyPlayers();
        }
    }
    else if (type == "game_started") {
        stopRefreshTimer();
        stopCountdownTimer();
        m_networkManager->disconnectFromLobby();
        QMessageBox::information(this, "Game Starting",
            "Game is starting! Transitioning to game screen...");
        emit gameStarted(m_lobbyId);
        accept();
    }
    else if (type == "player_left") {
        qDebug() << "Player left via WebSocket";
        QString username = message["username"].toString();
        int current_players = message["current_players"].toInt();
        int max_players = message["max_players"].toInt();
        
        // Update player count
        if (m_playerCountLabel) {
            m_playerCountLabel->setText(QString("Players: %1/%2")
                .arg(current_players)
                .arg(max_players));
        }
        
        // Fetch updated player list
        fetchLobbyPlayers();
    }
    else if (type == "lobby_closed") {
        stopRefreshTimer();
        stopCountdownTimer();
        m_networkManager->disconnectFromLobby();
        QString reason = message["reason"].toString();
        QMessageBox::warning(this, "Lobby Closed",
            QString("Lobby has been closed: %1").arg(reason));
        reject();
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