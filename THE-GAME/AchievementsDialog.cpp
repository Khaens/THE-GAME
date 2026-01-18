#include "AchievementsDialog.h"
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

AchievementsDialog::AchievementsDialog(std::shared_ptr<NetworkManager> networkManager, int userId, QWidget* parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog)
    , m_networkManager(networkManager)
    , m_userId(userId)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setupUI();
    loadAchievements();

    if (parent) {
        move(parent->mapToGlobal(parent->rect().center()) - rect().center());
    }
}

AchievementsDialog::~AchievementsDialog()
{
}

void AchievementsDialog::onBackClicked()
{
    close();
}

void AchievementsDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    if (m_background) {
        m_background->setGeometry(0, 0, width(), height());
    }
}

void AchievementsDialog::setupUI()
{
    resize(500, 750); 

    m_background = new QWidget(this);
    m_background->setObjectName("achievementsBackground");
    m_background->setStyleSheet(R"(
        #achievementsBackground { 
           border-image: url(Resources/TextBox_2-1.png);
           background-color: transparent;
        }
        )"
    );

    QVBoxLayout* mainLayout = new QVBoxLayout(m_background);
    mainLayout->setContentsMargins(50, 80, 50, 80); 
    mainLayout->setSpacing(20);

    m_titleLabel = new QLabel("ACHIEVEMENTS", m_background);
    m_titleLabel->setStyleSheet("font-family: 'Jersey 15'; font-size: 40px; color: #f3d05a; font-weight: bold; background: transparent;");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_titleLabel);

    m_scrollArea = new QScrollArea(m_background);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(R"(
        QScrollArea { background: transparent; border: none; }
        QScrollBar:vertical { background: #2c2c2c; width: 10px; margin: 0px; }
        QScrollBar::handle:vertical { background: #f3d05a; min-height: 20px; border-radius: 5px; }
        QScrollBar::add-line:vertical { height: 0px; }
        QScrollBar::sub-line:vertical { height: 0px; }
        )"
    );
    
    m_scrollContent = new QWidget();
    m_scrollContent->setStyleSheet("background: transparent;");
    m_scrollLayout = new QVBoxLayout(m_scrollContent);
    m_scrollLayout->setContentsMargins(10, 0, 10, 0);
    m_scrollLayout->setSpacing(15);
    m_scrollLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    
    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea);

    m_backButton = new QPushButton("BACK", m_background);
    m_backButton->setFixedSize(110, 55);
    m_backButton->setCursor(Qt::PointingHandCursor);
    m_backButton->setStyleSheet(R"(
        QPushButton {
            border-image: url(Resources/Button.png);
            font-family: 'Jersey 15';
            font-size: 40px;
            color: white;
            letter-spacing: 1px;
            font-weight: bold; 
            padding-bottom: 2px; 
        }
        QPushButton:pressed {
            border-image: url(Resources/Button_Pressed.png);
            padding-top: 2px;
            padding-left: 2px;
        }
    )");
    connect(m_backButton, &QPushButton::clicked, this, &AchievementsDialog::onBackClicked);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(m_backButton);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);
}

void AchievementsDialog::addAchievementItem(const QString& title, const QString& description, bool unlocked)
{
    QFrame* item = new QFrame(m_scrollContent);
    item->setMinimumHeight(80);
    
    QString border = unlocked ? "2px solid #f3d05a" : "2px solid #555";
    QString color = unlocked ? "#000000" : "#222222";
    QString textColor = unlocked ? "#f3d05a" : "#777777";
    
    item->setStyleSheet(QString(
        "QFrame { background-color: %1; border: %2; border-radius: 10px; border-image: none; }"
    ).arg(color).arg(border));

    QVBoxLayout* layout = new QVBoxLayout(item);
    layout->setContentsMargins(15, 5, 15, 5);
    layout->setSpacing(2);

    QLabel* titleLbl = new QLabel(title, item);
    titleLbl->setStyleSheet(QString("font-family: 'Jersey 15'; font-size: 24px; color: %1; font-weight: bold; border: none; background: transparent;").arg(textColor));
    titleLbl->setAlignment(Qt::AlignCenter);
    
    QLabel* descLbl = new QLabel(description, item);
    descLbl->setStyleSheet(QString("font-family: 'Jersey 15'; font-size: 16px; color: %1; border: none; background: transparent;").arg(textColor));
    descLbl->setAlignment(Qt::AlignCenter);
    descLbl->setWordWrap(true);

    layout->addWidget(titleLbl);
    layout->addWidget(descLbl);

    m_scrollLayout->addWidget(item);
}

void AchievementsDialog::loadAchievements()
{
    if (!m_networkManager) return;

    auto result = m_networkManager->getAchievements(m_userId);
    
    if (result.success) {
        QJsonObject ach = result.achievements;
        
        auto isUnlocked = [&](const QString& key) { return ach[key].toBool(); };

        addAchievementItem("All on Red", "You played as the Gambler and always placed at least 2 cards in every endgame round.", isUnlocked("all_on_red"));
        addAchievementItem("Harry Potter", "Played a game with the Harry Potter ability.", isUnlocked("harry_potter"));
        addAchievementItem("Soothsayer", "Played a game with the Soothsayer ability", isUnlocked("soothsayer"));
        addAchievementItem("Tax Evader", "Played a game with the Tax Evader ability", isUnlocked("tax_evader"));
        addAchievementItem("Gambler", "Played a game with the Gambler ability", isUnlocked("gambler"));
        addAchievementItem("Peasant", "Played a game as a Peasant (trolled)", isUnlocked("peasant"));
        addAchievementItem("Serious Player", "You've won 5 games. Keep it up!", isUnlocked("serious_player"));
        addAchievementItem("Talented Player", "You've won 8 or more games in your first ever 10 games. Keep it up!", isUnlocked("talented_player"));
        addAchievementItem("Jack of All Trades", "Played at least one game with all abilities. You are a truly versatile player!", isUnlocked("jack"));
        addAchievementItem("Zero Effort", "Won after using the Tax Evader ability at least 5 times. Laziness is the key to success.", isUnlocked("zero_effort"));
        addAchievementItem("Vanilla Victory", "Your team won without using any special abilities. Pure skill!", isUnlocked("vanilla_w"));
        addAchievementItem("High Risk High Reward", "You played as the Gambler and utilised all your ability uses.", isUnlocked("high_risk"));
        addAchievementItem("Perfect Game", "Won the game and always played cards with a maximum difference of 3 points between them throughout the entire match.", isUnlocked("perfect_game"));
        addAchievementItem("Six-Seven", "You placed both 6 and 7 in a single round.", isUnlocked("six_seven"));
    } else {
        QLabel* err = new QLabel("Failed to load achievements", m_scrollContent);
        err->setStyleSheet("color: red; font-size: 20px; font-family: 'Jersey 15';");
        err->setAlignment(Qt::AlignCenter);
        m_scrollLayout->addWidget(err);
    }
}

