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
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel("HOW TO PLAY", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2C3E50; margin-bottom: 10px;");
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
            background-color: #ECF0F1;
        }
        
        QTextBrowser {
            background-color: white;
            border: 2px solid #BDC3C7;
            border-radius: 8px;
            padding: 15px;
            font-size: 13px;
            color: #2C3E50;
        }
        
        QTextBrowser QScrollBar:vertical {
            border: none;
            background: #ECF0F1;
            width: 10px;
            margin: 0px;
        }
        
        QTextBrowser QScrollBar::handle:vertical {
            background: #95A5A6;
            border-radius: 5px;
            min-height: 20px;
        }
        
        QTextBrowser QScrollBar::handle:vertical:hover {
            background: #7F8C8D;
        }
        
        QPushButton {
            background-color: #3498DB;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
            padding: 10px;
        }
        
        QPushButton:hover {
            background-color: #2980B9;
        }
        
        QPushButton:pressed {
            background-color: #21618C;
        }
    )");
}

QString HelpDialog::getGameRules()
{
    return R"(
        <html>
        <body style='font-family: Arial, sans-serif; line-height: 1.6;'>
            
            <h2 style='color: #E74C3C; margin-top: 0;'>🎯 OBJECTIVE</h2>
            <p>Work together to play all 98 cards from the deck onto four piles before running out of moves!</p>
            
            <h2 style='color: #E74C3C;'>📚 SETUP</h2>
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
            
            <h2 style='color: #E74C3C;'>🎮 HOW TO PLAY</h2>
            
            <h3 style='color: #2980B9;'>On Your Turn:</h3>
            <ol>
                <li><b>Play at least 2 cards</b> from your hand onto any of the four piles</li>
                <li>After playing, draw back to 6 cards (if deck has cards)</li>
                <li>Pass the turn to the next player</li>
            </ol>
            
            <h3 style='color: #2980B9;'>Card Placement Rules:</h3>
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
            
            <h3 style='color: #2980B9;'>Communication Rules:</h3>
            <ul>
                <li>❌ <b>Cannot</b> say specific card numbers</li>
                <li>✅ <b>Can</b> give hints like "I have very high cards" or "I can help with the descending pile"</li>
                <li>✅ <b>Can</b> discuss strategy generally</li>
            </ul>
            
            <h2 style='color: #E74C3C;'>🏆 WINNING & LOSING</h2>
            <ul>
                <li><b style='color: #27AE60;'>You WIN</b> if all 98 cards are played!</li>
                <li><b style='color: #C0392B;'>You LOSE</b> if a player cannot play the minimum 2 cards (or 1 card when deck is empty)</li>
            </ul>
            
            <h2 style='color: #E74C3C;'>💡 TIPS FOR SUCCESS</h2>
            <ul>
                <li>🤝 <b>Communicate constantly</b> - teamwork is essential!</li>
                <li>⏰ <b>Save the "backward trick"</b> for emergencies</li>
                <li>🎯 <b>Spread cards across all piles</b> - don't focus on just one</li>
                <li>📊 <b>Middle-range cards (40-60)</b> are the hardest to play - use them wisely!</li>
                <li>🔄 <b>Plan ahead</b> - think about what cards teammates might need</li>
            </ul>
            
            <p style='text-align: center; margin-top: 20px; font-size: 16px; color: #7F8C8D;'>
                <b>Good luck, and remember - in The Game, you play together or lose together!</b>
            </p>
            
        </body>
        </html>
    )";
}