#include "LobbyDialog.h"
#include "CreateLobbyDialog.h" 
#include "JoinLobbyDialog.h"
#include "LobbyRoomDialog.h"
#include <QResizeEvent>
#include <QMessageBox>
#include <QDebug>
#include <QRandomGenerator>
#include <QMap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QCheckBox>

LobbyDialog::LobbyDialog(QWidget* parent)
    : QWidget(parent)
    , m_contentContainer(nullptr)
    , m_createLobbyButton(nullptr)
    , m_joinLobbyButton(nullptr)
    , m_userId(-1)
    , m_networkManager(nullptr)
{
    setWindowFlags(Qt::Widget);
    setAttribute(Qt::WA_TranslucentBackground);

    m_networkManager = new NetworkManager("http://localhost:18080");

    setupUI();
    setupStyle();
    hide();
}

void LobbyDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_contentContainer = new QWidget(this);
    m_contentContainer->setObjectName("lobbyContainer");
    m_contentContainer->setFixedSize(700, 300);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(15);
    containerLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    QLabel* titleLabel = new QLabel("GAME LOBBIES", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #f3d05a; margin-bottom: 10px;");
    containerLayout->addWidget(titleLabel);

    // Layout pentru butoane (vertical, unul sub altul)
    QVBoxLayout* buttonsLayout = new QVBoxLayout();
    buttonsLayout->setSpacing(15);

    // 1. CREATE LOBBY Button
    m_createLobbyButton = new QPushButton("CREATE LOBBY", m_contentContainer);
    m_createLobbyButton->setFixedSize(300, 55);
    m_createLobbyButton->setCursor(Qt::PointingHandCursor);

    // 2. JOIN LOBBY Button
    m_joinLobbyButton = new QPushButton("JOIN LOBBY", m_contentContainer);
    m_joinLobbyButton->setFixedSize(300, 55);
    m_joinLobbyButton->setCursor(Qt::PointingHandCursor);

    buttonsLayout->addWidget(m_createLobbyButton, 0, Qt::AlignCenter);
    buttonsLayout->addWidget(m_joinLobbyButton, 0, Qt::AlignCenter);

    containerLayout->addLayout(buttonsLayout);

    // Buton Inapoi
    QPushButton* backButton = new QPushButton("BACK", m_contentContainer);
    backButton->setObjectName("BACK");
    backButton->setFixedSize(150, 40);
    backButton->setCursor(Qt::PointingHandCursor);
    connect(backButton, &QPushButton::clicked, this, &LobbyDialog::hideOverlay);

    QHBoxLayout* backButtonLayout = new QHBoxLayout();
    backButtonLayout->addStretch();
    backButtonLayout->addWidget(backButton);
    backButtonLayout->addStretch();
    containerLayout->addLayout(backButtonLayout);

    // Centrare
    QHBoxLayout* centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(m_contentContainer);
    centerLayout->addStretch();

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();

    // Conectam sloturile
    connect(m_createLobbyButton, &QPushButton::clicked, this, &LobbyDialog::onCreateLobbyClicked);
    connect(m_joinLobbyButton, &QPushButton::clicked, this, &LobbyDialog::onJoinLobbyClicked);
}

void LobbyDialog::setupStyle()
{
    m_contentContainer->setStyleSheet(R"(
        #lobbyContainer{
            background-color: transparent;
            border-image: url(Resources/TextBox_1-2_Small.png); 
        }       
    )"
    );

    // Stiluri pentru butoanele de lobby
    QString lobbyButtonStyle = R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 10px;
            font-size: 18px;
            font-weight: bold;
            padding: 15px;
        }
        QPushButton:hover {
            background-color: #e3b310;
        }
        QPushButton:pressed {
            background-color: #869e22;
        }
    )";
    m_createLobbyButton->setStyleSheet(lobbyButtonStyle);
    m_joinLobbyButton->setStyleSheet(lobbyButtonStyle);

    // Stil pentru butonul "BACK"
    m_contentContainer->findChild<QPushButton*>("BACK")->setStyleSheet(R"(
        QPushButton {
            background-color: #654b1f;
            color: #f3d05a;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            padding: 8px 15px;
        }
        QPushButton:hover {
            background-color: #4a3f1e;
        }
    )");
}

void LobbyDialog::showOverlay()
{
    if (parentWidget()) {
        setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    }
    show();
}

void LobbyDialog::hideOverlay()
{
    hide();
}

void LobbyDialog::onCreateLobbyClicked()
{
    if (m_userId == -1) {
        QMessageBox::warning(this, "Authentication Required", "Please log in to create a lobby.");
        return;
    }

    CreateLobbyDialog dialog(m_userId, this);

    if (dialog.exec() == QDialog::Accepted && dialog.wasLobbyCreated()) {
        QString lobbyName = dialog.getLobbyName();
        int maxPlayers = dialog.getMaxPlayers();
        QString generatedPassword = dialog.getGeneratedPassword();

        LobbyResponse lobbyResponse = m_networkManager->createLobby(
            m_userId,
            lobbyName.toStdString(),
            maxPlayers,
            generatedPassword.toStdString()
        );

        if (lobbyResponse.success) {
            hideOverlay();

            // NEW: Show LobbyRoomDialog
            QString lobbyId = QString::fromStdString(lobbyResponse.lobby_id);
            LobbyRoomDialog* lobbyRoom = new LobbyRoomDialog(
                m_userId,
                lobbyId,
                lobbyName,
                generatedPassword,
                true,  // isHost = true for creator
                parentWidget()
            );

            lobbyRoom->exec();
            delete lobbyRoom;
        }
        else {
            QString errorMessage = QString::fromStdString(
                lobbyResponse.error_message.empty() ? "Unknown error creating lobby." : lobbyResponse.error_message
            );
            QMessageBox::warning(this, "Error", errorMessage);
        }
    }
}

void LobbyDialog::onJoinLobbyClicked()
{
    if (m_userId == -1) {
        QMessageBox::warning(this, "Authentication Required", "Please log in to join a lobby.");
        return;
    }

    JoinLobbyDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QString lobbyCode = dialog.getLobbyCode();
        std::string code_to_join = lobbyCode.toStdString();

        bool success = m_networkManager->joinLobby(m_userId, code_to_join);

        if (success) {
            QMessageBox::information(this, "Success",
                "Successfully joined lobby with code: " + lobbyCode);
            hideOverlay();

            // NEW: Show LobbyRoomDialog
            // Note: We need to get lobby name from server response
            // For now, we'll use the code as name
            LobbyRoomDialog* lobbyRoom = new LobbyRoomDialog(
                m_userId,
                QString::fromStdString(code_to_join), // Use code as ID temporarily
                "Joined Lobby", // TODO: Get actual lobby name from server
                lobbyCode,
                false,  // isHost = false for joiner
                parentWidget()
            );

            lobbyRoom->exec();
            delete lobbyRoom;
        }
        else {
            QMessageBox::warning(this, "Error",
                "Failed to join lobby. Invalid code or lobby is full.");
        }
    }
}

void LobbyDialog::resizeEvent(QResizeEvent* event)
{
    if (parentWidget() && isVisible()) {
        setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    }
    QWidget::resizeEvent(event);
}