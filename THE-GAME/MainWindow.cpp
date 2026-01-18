#include "MainWindow.h"
#include "NetworkConfig.h"
#include <QPixmap>
#include <QMessageBox>
#include <QShortcut>
#include <QKeySequence>
#include <QResizeEvent>
#include <QPainter>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass())
    , m_helpDialog(nullptr)
    , m_settingsDialog(nullptr)
    , m_accountDialog(nullptr)
    , m_lobbyDialog(nullptr)
	, m_gameWindow(nullptr)
{
    ui->setupUi(this);

    loadBackgroundImage();

    ui->centralWidget->setStyleSheet("background-color: transparent;");

    setupMenuStyle();

	m_networkManager = std::make_shared<NetworkManager>(SERVER_URL);

    connect(m_networkManager.get(), &NetworkManager::serverStatusChanged, this, [this](bool isOnline) {
        if (isOnline) {
            ui->connectionStatus->setText("CONNECTED");
            ui->connectionStatus->setStyleSheet("color: #00FF00; font-weight: bold; font-size: 16px;");
        } else {
            ui->connectionStatus->setText("DISCONNECTED");
            ui->connectionStatus->setStyleSheet("color: #FF0000; font-weight: bold; font-size: 16px;");
        }
    });

    m_helpDialog = new HelpDialog(this);
    m_settingsDialog = new SettingsDialog(this);
    m_accountDialog = new AccountDialog(this);
    m_lobbyDialog = new LobbyDialog(this);
    m_gameWindow = new GameWindow(this);

    m_accountDialog->setNetworkManager(m_networkManager);
    m_lobbyDialog->setNetworkManager(m_networkManager.get());

	connect(m_lobbyDialog, &LobbyDialog::gameStartedFromLobby, this, &MainWindow::showGameOverlay);
    connect(ui->newGameButton, &QPushButton::clicked, this, &MainWindow::onNewGameClicked);
    connect(ui->exitGameButton, &QPushButton::clicked, this, &MainWindow::onExitClicked);
    connect(ui->helpButton, &QPushButton::clicked, this, &MainWindow::onHelpClicked);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(ui->accountButton, &QPushButton::clicked, this, &MainWindow::onAccountClicked);

    connect(m_gameWindow, &GameWindow::backToMenuRequested, this, &MainWindow::onGameBackToMenu);

    QShortcut* fsShortcut = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(fsShortcut, &QShortcut::activated, this, &MainWindow::toggleFullScreen);

    QShortcut* escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, [this]() {
        if (isFullScreen()) {
            showNormal();
        }
    });

    QPixmap titlePixmap("Resources/TitleCard.png");
    if (!titlePixmap.isNull()) {
        m_titlePixmap = titlePixmap;
        QPixmap scaled = m_titlePixmap.scaled(ui->titleLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->titleLabel->setPixmap(scaled);
        ui->titleLabel->setAlignment(Qt::AlignCenter);
    }
    else {
        qWarning() << "Failed to load TitleCard.png";
    }
}

MainWindow::~MainWindow()
{
    m_helpDialog = nullptr;
    m_settingsDialog = nullptr;
    m_accountDialog = nullptr;
    m_lobbyDialog = nullptr;
    m_gameWindow = nullptr;
    
    delete ui;
    ui = nullptr;
}

void MainWindow::loadBackgroundImage()
{
    m_backgroundPixmap = QPixmap("Resources/TableMC.png");

    if (m_backgroundPixmap.isNull()) {
        qWarning() << "Failed to load TableMC.png - using fallback color";
        m_backgroundPixmap = QPixmap(1, 1);
        m_backgroundPixmap.fill(QColor("#0d0a47"));
    }
    else {
        qDebug() << "Background loaded successfully:" << m_backgroundPixmap.size();
    }
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    if (!m_backgroundPixmap.isNull()) {
        painter.drawPixmap(rect(), m_backgroundPixmap);
    }
    else {
        painter.fillRect(rect(), QColor("#0d0a47"));
    }

    QMainWindow::paintEvent(event);
}

void MainWindow::setupMenuStyle()
{
    QString playButtonStyle = R"(
    QPushButton {
        border-image: url(Resources/Button_Play.png);
    }
    QPushButton:pressed {
        border-image: url(Resources/Button_Play_Pressed.png);
    }
    )";

    QString settingsButtonStyle = R"(
    QPushButton {
        border-image: url(Resources/Button_Settings.png);
    }
    QPushButton:pressed {
        border-image: url(Resources/Button_Settings_Pressed.png);
    }
    )";

    QString exitButtonStyle = R"(
    QPushButton {
        border-image: url(Resources/Button_Exit.png);
    }
    QPushButton:pressed {
        border-image: url(Resources/Button_Exit_Pressed.png);
    }
    )";

    QString helpButtonStyle = R"(
    QPushButton {
        border-image: url(Resources/Button_Help.png);
    }
    QPushButton:pressed {
        border-image: url(Resources/Button_Help_Pressed.png);
    }
    )";

    QString accountButtonStyle = R"(
    QPushButton {
        border-image: url(Resources/Button_Account.png);
    }
    QPushButton:pressed {
        border-image: url(Resources/Button_Account_Pressed.png);
    }
    )";

    ui->newGameButton->setStyleSheet(playButtonStyle);
    ui->settingsButton->setStyleSheet(settingsButtonStyle);
    ui->exitGameButton->setStyleSheet(exitButtonStyle);
    ui->helpButton->setStyleSheet(helpButtonStyle);
    ui->accountButton->setStyleSheet(accountButtonStyle);
}

void MainWindow::onNewGameClicked()
{
    if (!m_accountDialog->isUserLoggedIn()) {
        QMessageBox::warning(this, "Login Required",
            "You need to be logged in to play!\n\nPlease click the Account button to login or register.");
        return;
    }

    m_lobbyDialog->setUserId(m_accountDialog->getCurrentUserId());
    m_lobbyDialog->showOverlay();
}

void MainWindow::onExitClicked()
{
    close();
}

void MainWindow::onHelpClicked()
{
    m_helpDialog->showOverlay();
}

void MainWindow::onSettingsClicked()
{
    m_settingsDialog->showOverlay();
}

void MainWindow::onAccountClicked()
{
    m_accountDialog->showOverlay();
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    if (!m_titlePixmap.isNull() && ui->titleLabel) {
        QSize labelSize = ui->titleLabel->size();
        if (labelSize.width() > 0 && labelSize.height() > 0) {
            QPixmap scaled = m_titlePixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->titleLabel->setPixmap(scaled);
            ui->titleLabel->setAlignment(Qt::AlignCenter);
        }
    }
    if (m_helpDialog && m_helpDialog->isVisible()) {
        m_helpDialog->setGeometry(0, 0, width(), height());
    }

    if (m_settingsDialog && m_settingsDialog->isVisible()) {
        m_settingsDialog->setGeometry(0, 0, width(), height());
    }

    if (m_accountDialog && m_accountDialog->isVisible()) {
        m_accountDialog->setGeometry(0, 0, width(), height());
    }

    if (m_lobbyDialog && m_lobbyDialog->isVisible()) {
        m_lobbyDialog->setGeometry(0, 0, width(), height());
    }

    if (m_gameWindow && m_gameWindow->isVisible()) {
        m_gameWindow->setGeometry(0, 0, width(), height());
    }

    QMainWindow::resizeEvent(event);
}

void MainWindow::showGameOverlay(const QString& lobbyId)
{
    if (m_accountDialog && m_gameWindow) {
        int userId = m_accountDialog->getCurrentUserId();
        m_gameWindow->initialize(m_networkManager.get(), userId, lobbyId.toStdString());
    }
    
    m_gameWindow->showOverlay();
}

void MainWindow::onGameBackToMenu()
{
    if (m_gameWindow) {
        m_gameWindow->hideOverlay();
    }
    this->show();
    
    if (m_lobbyDialog) {
        m_lobbyDialog->hideOverlay();
    }
}