#include "MainWindow.h"
#include <QPixmap>
#include <QMessageBox>
#include <QShortcut>
#include <QKeySequence>
#include <QResizeEvent>
#include <QDebug>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass())
    , m_helpWidget(nullptr)
{
    ui->setupUi(this);
    setupMenuStyle();

    // Widget-ul de help va fi un overlay
    m_helpWidget = createHelpWidget();
    m_helpWidget->setParent(this);
    m_helpWidget->hide(); 

    // Connect buttons to slots
    connect(ui->newGameButton, &QPushButton::clicked, this, &MainWindow::onNewGameClicked);
    connect(ui->exitGameButton, &QPushButton::clicked, this, &MainWindow::onExitClicked);
    connect(ui->helpButton, &QPushButton::clicked, this, &MainWindow::onHelpClicked);

    // Shortcut for fullscreen toggle (F11)
    QShortcut* fsShortcut = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(fsShortcut, &QShortcut::activated, this, &MainWindow::toggleFullScreen);

    // Load the title image
    QPixmap titlePixmap("Assets/TitleCard.png");
    if (!titlePixmap.isNull()) {
        m_titlePixmap = titlePixmap;
        QPixmap scaled = m_titlePixmap.scaled(ui->titleLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->titleLabel->setPixmap(scaled);
        ui->titleLabel->setAlignment(Qt::AlignCenter);
    }
    else {
        qWarning() << "Failed to load Assets/TitleCard.png";
    }

    // background to the central widget
    ui->centralWidget->setStyleSheet("background-color: #001433;");
}

MainWindow::~MainWindow()
{
    delete ui;
}

QWidget* MainWindow::createHelpWidget()
{
    // Widget semi-transparent pentru background overlay
    QWidget* helpWidget = new QWidget();
    helpWidget->setStyleSheet("background-color: rgba(0, 0, 0, 150);"); // semi-transparent black

    QWidget* helpContainer = new QWidget(helpWidget);
    helpContainer->setStyleSheet("background-color: #8e273b; border: 3px solid #f3d05a; border-radius: 15px;");
    helpContainer->setFixedSize(800, 600); 

    QVBoxLayout* containerLayout = new QVBoxLayout(helpContainer);
    containerLayout->setSpacing(15);
    containerLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel("HELP - THE GAME", helpContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #f3d05a; margin-bottom: 5px;");
    containerLayout->addWidget(titleLabel);

    // Rules text browser
    QTextBrowser* rulesText = new QTextBrowser(helpContainer);
    rulesText->setOpenExternalLinks(false);
    rulesText->setHtml(getGameRules());
    rulesText->setStyleSheet(R"(
        QTextBrowser {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 15px;
            font-size: 13px;
            color: #2C3E50;
        }
        
        QTextBrowser QScrollBar:vertical {
            border: none;
            background: #3d431a;
            width: 12px;
            margin: 0px;
        }
        
        QTextBrowser QScrollBar::handle:vertical {
            background: #654b1f;
            border-radius: 6px;
            min-height: 20px;
        }
        
        QTextBrowser QScrollBar::handle:vertical:hover {
            background: #4a3f1e;
        }
    )");
    containerLayout->addWidget(rulesText);

    // Back button
    QPushButton* backButton = new QPushButton("BACK TO MENU", helpContainer);
    backButton->setFixedSize(200, 45);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
            padding: 10px;
        }
        
        QPushButton:hover {
            background-color: #e3b310;
        }
        
        QPushButton:pressed {
            background-color: #869e22;
        }
    )");

    connect(backButton, &QPushButton::clicked, this, &MainWindow::onBackToMenuClicked);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    containerLayout->addLayout(buttonLayout);

    // Layout principal pentru centrarea containerului
    QVBoxLayout* mainLayout = new QVBoxLayout(helpWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(helpContainer);
    centerLayout->addStretch();

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();

    return helpWidget;
}

QString MainWindow::getGameRules()
{
    return R"(
        <html>
        <body style='font-family: Arial, sans-serif; line-height: 1.6; background-color: #deaf11;'>
            
            <h2 style='color: #8e273b; margin-top: 10px; border-bottom: 2px solid #654b1f; padding-bottom: 5px;'>🎯 OBJECTIVE</h2>
            <p>Work together to play all cards from the deck and your very own hands onto the four piles before running out of moves!</p>
           
            <h2 style='color: #8e273b; border-bottom: 2px solid #654b1f; padding-bottom: 5px;'>⚙️ SETUP</h2>
            <ul>
                <li><b>Players:</b> 2-5 players (cooperative game)</li>
                <li><b>Starting Hand:</b> Each player receives 6 cards</li>
                <li><b>Four Piles:</b>
                    <ul>
                        <li>2 <b style='color: #27AE60;'>Ascending Piles</b> (start at 1, go up)</li>
                        <li>2 <b style='color: #E67E22;'>Descending Piles</b> (start at 100, go down)</li>
                    </ul>
                </li>
            </ul>
            
            <h2 style='color: #8e273b; border-bottom: 2px solid #654b1f; padding-bottom: 5px;'>🎮 HOW TO PLAY</h2>
            
            <h3 style='color: #654b1f;'>On Your Turn:</h3>
            <ol>
                <li><b>Play at least 2 cards</b> from your hand onto any of the four piles</li>
                <li>If there are no more cards that can be drawn, only a minimum of 1 card is to be played</li>
                <li>After playing, draw back to 6 cards (if deck has cards)</li>
                <li>Pass the turn to the next player</li>
            </ol>
            
            <h3 style='color: #654b1f;'>Card Placement Rules:</h3>
            <ul>
                <li><b style='color: #27AE60;'>Ascending Piles:</b> Play higher numbers (e.g., 1 → 15 → 28 → 45...)</li>
                <li><b style='color: #E67E22;'>Descending Piles:</b> Play lower numbers (e.g., 100 → 82 → 67 → 43...)</li>
                <li><b style='color: #8E44AD;'>Special Rule - Backward Trick:</b>
                    <ul>
                        <li>You can go <i>backwards by exactly 10</i> on ascending piles</li>
                        <li>You can go <i>forwards by exactly 10</i> on descending piles</li>
                        <li>Example: 45 → <b>35</b> on ascending or 55 → <b>65</b> on descending</li>
                    </ul>
                </li>
            </ul>
            
            <h3 style='color: #654b1f;'>💬 Communication Rules:</h3>
            <ul>
                <li><b>Cannot</b> say specific card numbers</li>
                <li><b>Can</b> give hints like "I have very high cards" or "I can help with the descending pile"</li>
                <li><b>Can</b> discuss strategy generally</li>
            </ul>
            
            <h2 style='color: #8e273b; border-bottom: 2px solid #654b1f; padding-bottom: 5px;'>🏆 WINNING & LOSING</h2>
            <ul>
                <li><b style='color: #27AE60;'>You WIN</b> if all cards from both the players' hands and the main deck are onto the piles</li>
                <li><b style='color: #C0392B;'>You LOSE</b> if a player cannot play the minimum cards required</li>
            </ul>
            
            <h2 style='color: #8e273b; border-bottom: 2px solid #654b1f; padding-bottom: 5px;'>💡 TIPS FOR SUCCESS</h2>
            <ul>
                <li><b>Communicate constantly</b> - teamwork is essential!</li>
                <li><b>Save the "backward trick"</b> for emergencies</li>
                <li><b>Spread cards across all piles</b> - don't focus on just one</li>
                <li><b>Middle-range cards (40-60)</b> are the hardest to play - use them wisely!</li>
                <li><b>Plan ahead</b> - think about what cards teammates might need</li>
            </ul>
            
            <p style='text-align: center; margin-top: 25px; font-size: 16px; color: #8e273b; font-weight: bold;'>
                Good luck, and remember - in The Game, you play together or lose together! 🎲
            </p>
            
        </body>
        </html>
    )";
}

void MainWindow::setupMenuStyle()
{
    // Button styles (image-based)
    QString gameButtonStyle = R"(
    QPushButton {
        border-image: url(Assets/Button_Play.png);
    }
    QPushButton:pressed {
        border-image: url(Assets/Button_Play_Pressed.png);
    }
    )";

    QString exitButtonStyle = R"(
    QPushButton {
        border-image: url(Assets/Button_Exit.png);
    }
    QPushButton:pressed {
        border-image: url(Assets/Button_Exit_Pressed.png);
    }
    )";

    QString helpButtonStyle = R"(
    QPushButton {
        border-image: url(Assets/Button_Help.png);
    }
    QPushButton:pressed {
        border-image: url(Assets/Button_Help_Pressed.png);
    }
    )";

    ui->newGameButton->setStyleSheet(gameButtonStyle);
    ui->exitGameButton->setStyleSheet(exitButtonStyle);
    ui->helpButton->setStyleSheet(helpButtonStyle);
}

void MainWindow::onNewGameClicked()
{
    qDebug() << "New Game clicked!";
}

void MainWindow::onExitClicked()
{
    close();
}

void MainWindow::onHelpClicked()
{
    // Arată overlay-ul
    m_helpWidget->setGeometry(0, 0, this->width(), this->height());
    m_helpWidget->raise(); 
    m_helpWidget->show();
}

void MainWindow::onBackToMenuClicked()
{
    m_helpWidget->hide();
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
    // Rescale the cached title pixmap
    if (!m_titlePixmap.isNull() && ui->titleLabel) {
        QSize labelSize = ui->titleLabel->size();
        if (labelSize.width() > 0 && labelSize.height() > 0) {
            QPixmap scaled = m_titlePixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->titleLabel->setPixmap(scaled);
            ui->titleLabel->setAlignment(Qt::AlignCenter);
        }
    }

    if (m_helpWidget && m_helpWidget->isVisible()) {
        m_helpWidget->setGeometry(0, 0, this->width(), this->height());
    }

    QMainWindow::resizeEvent(event);
}