#include "HelpDialog.h"
#include <QScrollBar>

HelpDialog::HelpDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
    setupStyle();
}

void HelpDialog::setupUI()
{
    setWindowTitle("The Game - Rules & Help");
    setModal(true);
    setFixedSize(700, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Title
    QLabel* titleLabel = new QLabel("HELP - THE GAME", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ffffff; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Rules text browser
    QTextBrowser* rulesText = new QTextBrowser(this);
    rulesText->setOpenExternalLinks(false);
    rulesText->setHtml(getGameRules());
    mainLayout->addWidget(rulesText);

    // Close button
    QPushButton* closeButton = new QPushButton("Got it!", this);
    closeButton->setFixedSize(150, 40);
    closeButton->setCursor(Qt::PointingHandCursor);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

void HelpDialog::setupStyle()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #8e273b;
        }
        
        QTextBrowser {
            background-color: #deaf11;
            border: 1px solid #ff2424;
            padding: 10px;
            font-size: 13px;
            color: #2C3E50;
        }
        
        QTextBrowser QScrollBar:vertical {
            border: none;
            background: #3d431a;
            width: 10px;
            margin: 0px;
        }
        
        QTextBrowser QScrollBar::handle:vertical {
            background: #654b1f;
            border-radius: 5px;
            min-height: 20px;
        }
        
        QTextBrowser QScrollBar::handle:vertical:hover {
            background: #4a3f1e;
        }
        
        QPushButton {
            background-color: #f3d05a;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
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
}

QString HelpDialog::getGameRules()
{
    return R"(
        <html>
        <body style='font-family: Arial, sans-serif; line-height: 1.25; background-color: #c34158;'>
            
            <h2 style='color: #f3d05a; margin-top: 10px;'>OBJECTIVE</h2>
            <p>Work together to play all cards from the deck and your very own hands onto the four piles before running out of moves!</p>
           
            <h2 style='color: #f3d05a;'>SETUP</h2>
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
            
            <h2 style='color: #f3d05a;'>HOW TO PLAY</h2>
            
            <h3 style='color: #ffffff;'>On Your Turn:</h3>
            <ol>
                <li><b>Play at least 2 cards</b> from your hand onto any of the four piles</li>
                <li>If there are no more cards that can be drawn, only a minimum of 1 card is to be played</li>
                <li>After playing, draw back to 6 cards (if deck has cards)</li>
                <li>Pass the turn to the next player</li>
            </ol>
            
            <h3 style='color: #ffffff;'>Card Placement Rules:</h3>
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
            
            <h3 style='color: #ffffff;'>Communication Rules:</h3>
            <ul>
                <li><b>Cannot</b> say specific card numbers</li>
                <li><b>Can</b> give hints like "I have very high cards" or "I can help with the descending pile"</li>
                <li><b>Can</b> discuss strategy generally</li>
            </ul>
            
            <h2 style='color: #f3d05a;'>WINNING & LOSING</h2>
            <ul>
                <li><b style='color: #27AE60;'>You WIN</b> if all cards from both the players' hands and the main deck are onto the decks</li>
                <li><b style='color: #C0392B;'>You LOSE</b> if a player cannot play the minimum cards required</li>
            </ul>
            
            <h2 style='color: #f3d05a;'>TIPS FOR SUCCESS</h2>
            <ul>
                <li><b>Communicate constantly</b> - teamwork is essential!</li>
                <li><b>Save the "backward trick"</b> for emergencies</li>
                <li><b>Spread cards across all piles</b> - don't focus on just one</li>
                <li><b>Middle-range cards (40-60)</b> are the hardest to play - use them wisely!</li>
                <li><b>Plan ahead</b> - think about what cards teammates might need</li>
            </ul>
            
            <p style='text-align: center; margin-top: 20px; margin-bottom: 10px; font-size: 16px; color: #ffffff;'>
                <b>Good luck, and remember - in The Game, you play together or lose together!</b>
            </p>
            
        </body>
        </html>
    )";
}