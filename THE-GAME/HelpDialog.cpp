#include "HelpDialog.h"
#include <QResizeEvent>

HelpDialog::HelpDialog(QWidget* parent)
    : QWidget(parent)
    , m_contentContainer(nullptr)
{
    setWindowFlags(Qt::Widget);
    setAttribute(Qt::WA_TranslucentBackground);

    setupUI();
    setupStyle();
    hide(); 
}

void HelpDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_contentContainer = new QWidget(this);
    m_contentContainer->setFixedSize(800, 600);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(15);
    containerLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel("HELP - THE GAME", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #f3d05a; margin-bottom: 5px;");
    containerLayout->addWidget(titleLabel);

    // Rules text browser
    QTextBrowser* rulesText = new QTextBrowser(m_contentContainer);
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
    QPushButton* backButton = new QPushButton("BACK TO MENU", m_contentContainer);
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

    connect(backButton, &QPushButton::clicked, this, &HelpDialog::hideOverlay);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    containerLayout->addLayout(buttonLayout);

    // Centrarea containerului
    QHBoxLayout* centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(m_contentContainer);
    centerLayout->addStretch();

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();
}

void HelpDialog::setupStyle()
{
    setStyleSheet("background-color: rgba(0, 0, 0, 150);");

    m_contentContainer->setStyleSheet(
        "background-color: #8e273b; "
        "border: 3px solid #f3d05a; "
        "border-radius: 15px;"
    );
}

QString HelpDialog::getGameRules()
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

void HelpDialog::showOverlay()
{
    if (parentWidget()) {
        setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    }
    raise();
    show();
}

void HelpDialog::hideOverlay()
{
    hide();
}

void HelpDialog::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}