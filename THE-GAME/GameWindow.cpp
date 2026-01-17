#include "GameWindow.h"
#include "ui_GameWindow.h"
#include "UiUtils.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QEasingCurve>
#include <QTimer>
#include <QMouseEvent>
#include <QPainterPath>
#include <QDebug>
#include <QBuffer>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QListWidgetItem>
#include "PeasantDialog.h"

GameWindow::GameWindow(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Form())
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);
    hide();

    ui->formBackground->setStyleSheet("background-color: transparent;");
    
    // Install event filter on chat button
    if (ui->chatButton) {
        ui->chatButton->installEventFilter(this);
        ui->chatButton->setCursor(Qt::PointingHandCursor);
    }
    
    if (ui->chatInput) {
        connect(ui->chatInput, &QLineEdit::returnPressed, this, &GameWindow::sendMessage);
    }
    
    // Ensure chat starts hidden
    if (ui->chatWidget) {
        // Will be positioned off-screen by resizeUI initially if logic is added there
        // or we can hide it. But we want it to slide in, so just position it off screen
        // resizeUI will handle initial placement
    }
    
    loadGameImage();
    
    
    // Attempt to connect end turn button if it exists (user added in Designer)
    QPushButton* endTurnBtn = findChild<QPushButton*>("endTurnButton");
    if (endTurnBtn) {
        connect(endTurnBtn, &QPushButton::clicked, this, &GameWindow::sendEndTurnAction);
    }

    if (ui->abilityButton) {
        connect(ui->abilityButton, &QPushButton::clicked, this, &GameWindow::onAbilityButtonClicked);
    }
    
    // Connect to Turn Label defined in Designer
    m_turnLabel = findChild<QLabel*>("turnLabel");
    if (m_turnLabel) {
        m_turnLabel->setText("Waiting for game start...");
        m_turnLabel->setVisible(true);
    } else {
        qDebug() << "Warning: turnLabel not found in UI!";
    }

    // Chat Setup
    if (ui->chatHistory) {
        ui->chatHistory->hide(); // Hide original ListWidget
    }
    
    // Create new TextEdit for Chat
    if (ui->chatWidget) {
        m_chatDisplay = new QTextEdit(ui->chatWidget);
        m_chatDisplay->setReadOnly(true);
        m_chatDisplay->setFrameShape(QFrame::NoFrame);
        // Style: Transparent background, white text
        m_chatDisplay->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
        m_chatDisplay->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_chatDisplay->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_chatDisplay->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        m_chatDisplay->show();
    }
}

GameWindow::~GameWindow()
{
    delete ui;
    ui = nullptr;
}

void GameWindow::showOverlay()
{
    QWidget* parent = parentWidget();
    if (parent) {
        setGeometry(0, 0, parent->width(), parent->height());
    }
    raise();
    show();

    // Ensure layout/scaling is up-to-date to get correct final positions
    if (ui && ui->bannerLabel && ui->beerLabel && ui->cardBack) {
        // Force manual scaling update
        resizeUI();
        
        // Save final positions (where scaling put them)
        QRect finalBannerRect = ui->bannerLabel->geometry();
        QRect finalBeerRect = ui->beerLabel->geometry();
        QRect finalCardBackRect = ui->cardBack->geometry();

        // 1. Banner Animation (Top to Bottom)
        QPropertyAnimation* bannerAnim = new QPropertyAnimation(ui->bannerLabel, "geometry");
        bannerAnim->setDuration(2000); 
        bannerAnim->setStartValue(QRect(finalBannerRect.x(), -finalBannerRect.height(), finalBannerRect.width(), finalBannerRect.height()));
        bannerAnim->setEndValue(finalBannerRect);
        bannerAnim->setEasingCurve(QEasingCurve::OutCubic);

        // 2. Beer Meter Animation (Bottom to Top)
        QPropertyAnimation* beerAnim = new QPropertyAnimation(ui->beerLabel, "geometry");
        beerAnim->setDuration(2000);
        beerAnim->setStartValue(QRect(finalBeerRect.x(), height(), finalBeerRect.width(), finalBeerRect.height()));
        beerAnim->setEndValue(finalBeerRect);
        beerAnim->setEasingCurve(QEasingCurve::OutCubic);

        // Group 1: Parallel animations for Banner and Beer
        QParallelAnimationGroup* initialGroup = new QParallelAnimationGroup();
        initialGroup->addAnimation(bannerAnim);
        initialGroup->addAnimation(beerAnim);

        // 3. Card Back Animation (Appears AFTER banner)
        // Set initial state for card back (hidden/off-screen)
        // We'll animate it sliding in from above, similar to banner but delayed
        ui->cardBack->setGeometry(finalCardBackRect.x(), -finalCardBackRect.height(), finalCardBackRect.width(), finalCardBackRect.height());
        
        QPropertyAnimation* cardBackAnim = new QPropertyAnimation(ui->cardBack, "geometry");
        cardBackAnim->setDuration(1000); // 1 second animation
        cardBackAnim->setStartValue(QRect(finalCardBackRect.x(), -finalCardBackRect.height(), finalCardBackRect.width(), finalCardBackRect.height()));
        cardBackAnim->setEndValue(finalCardBackRect);
        cardBackAnim->setEasingCurve(QEasingCurve::OutBack); // Bouncy effect

        // Main Sequential Group: [Banner+Beer] -> [Card Back]
        QSequentialAnimationGroup* mainSequence = new QSequentialAnimationGroup(this);
        mainSequence->addAnimation(initialGroup);
        mainSequence->addAnimation(cardBackAnim);
        
        mainSequence->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void GameWindow::hideOverlay()
{
    resetGameState();
    hide();
}

void GameWindow::resetGameState()
{
    // Clear Hand
    if (ui) {
        if (ui->hand1) ui->hand1->setVisible(false);
        if (ui->hand2) ui->hand2->setVisible(false);
        if (ui->hand3) ui->hand3->setVisible(false);
        if (ui->hand4) ui->hand4->setVisible(false);
        if (ui->hand5) ui->hand5->setVisible(false);
        if (ui->hand6) ui->hand6->setVisible(false);
        
        // Reset Piles to initial state (hidden images)
        if (ui->ascPile1) ui->ascPile1->setVisible(false);
        if (ui->ascPile2) ui->ascPile2->setVisible(false);
        if (ui->descPile1) ui->descPile1->setVisible(false);
        if (ui->descPile2) ui->descPile2->setVisible(false);
        
        // Reset BeerMeter PFP Circles
        if (ui->pfpCircle1) ui->pfpCircle1->clear();
        if (ui->pfpCircle2) ui->pfpCircle2->clear();
        if (ui->pfpCircle3) ui->pfpCircle3->clear();
        if (ui->pfpCircle4) ui->pfpCircle4->clear();
        if (ui->pfpCircle5) ui->pfpCircle5->clear();

        // Chat
        // Chat
        // if (ui->chatHistory) ui->chatHistory->clear();
        if (m_chatDisplay) m_chatDisplay->clear();
        
        // Ability Button
        if (ui->abilityButton) ui->abilityButton->setText("ABILITY");

        // Turn Label
        if (m_turnLabel) {
             m_turnLabel->setText(""); 
        }
    }

    // Logic State
    m_isMyTurn = false;
    m_isHPMode = false;
    m_wasSoothActive = false;
    m_cardsPlayedThisTurn = 0;
    m_deckCount = 0;
    
    // Reset Pile Values
    m_pileTopValues[0] = 1;
    m_pileTopValues[1] = 1;
    m_pileTopValues[2] = 100;
    m_pileTopValues[3] = 100;
    
    m_lastPileTops[0].clear();
    m_lastPileTops[1].clear();
    m_lastPileTops[2].clear();
    m_lastPileTops[3].clear();

    clearCardSelection();
}

void GameWindow::resizeEvent(QResizeEvent* event)
{
    if (ui && ui->formBackground) {
        ui->formBackground->setGeometry(this->rect());
    }
    
    // Perform manual scaling of UI elements
    resizeUI();
    
    QWidget::resizeEvent(event);
}



QRect GameWindow::scaleRect(int x, int y, int w, int h) const
{
    // Design resolution
    const float designW = 1366.0f;
    const float designH = 768.0f;

    // Current window dimensions
    float currentW = static_cast<float>(this->width());
    float currentH = static_cast<float>(this->height());

    // Calculate scale factors
    float scaleX = currentW / designW;
    float scaleY = currentH / designH;

    return QRect(
        static_cast<int>(x * scaleX),
        static_cast<int>(y * scaleY),
        static_cast<int>(w * scaleX),
        static_cast<int>(h * scaleY)
    );
}

void GameWindow::resizeUI()
{
    if (!ui) return;

    const float designH = 768.0f;
    
    // Member function scaleRect is now available to use directly.

    // Apply scaling to all known widgets
    // Coordinates taken from UI design (approximate based on user's successful windowed view)
    
    // Banner: (985, 0, 150, 435)
    if (ui->bannerLabel) ui->bannerLabel->setGeometry(scaleRect(985, 0, 180, 450));
    
    // Beer: (1185, 9, 141, 750)
    if (ui->beerLabel) ui->beerLabel->setGeometry(scaleRect(1185, 9, 141, 750));
    
    // CardBack: (1000, 70, 125, 187)
    if (ui->cardBack) ui->cardBack->setGeometry(scaleRect(1000, 70, 150, 225));
    
    // Chat Button: (10, 680, 75, 75)
    if (ui->chatButton) ui->chatButton->setGeometry(scaleRect(10, 680, 75, 75));
    
    // Piles
    // Asc1: (425, 50, 125, 187)
    if (ui->underAsc1) ui->underAsc1->setGeometry(scaleRect(275, 50, 150, 225));
    
    // Asc2: (750, 50, 125, 187)
    if (ui->underAsc2) ui->underAsc2->setGeometry(scaleRect(600, 50, 150, 225));
    
    // Desc1: (425, 300, 125, 187)
    if (ui->underDesc1) ui->underDesc1->setGeometry(scaleRect(375, 300, 150, 225));
    
    // Desc2: (750, 300, 150, 225)
    if (ui->underDesc2) ui->underDesc2->setGeometry(scaleRect(700, 300, 150, 225));

    // Hand Cards
    // Base Y is 630, W 150, H 225
    // Spacing 100px (440, 540, 640...)
    if (ui->hand1) ui->hand1->setGeometry(scaleRect(440, 630, 150, 225));
    if (ui->hand2) ui->hand2->setGeometry(scaleRect(540, 630, 150, 225));
    if (ui->hand3) ui->hand3->setGeometry(scaleRect(640, 630, 150, 225));
    if (ui->hand4) ui->hand4->setGeometry(scaleRect(740, 630, 150, 225));
    if (ui->hand5) ui->hand5->setGeometry(scaleRect(840, 630, 150, 225));
    if (ui->hand6) ui->hand6->setGeometry(scaleRect(940, 630, 150, 225));
    
    // PFP Circles Geometry (ensure aspect ratio or correct scale)
    // BeerMeter: (1185, 9, 141, 750)
    // The circles in the design are at X=1220 (offset 35 from 1185)
    // Spacing Y seems to be 150, 250, 350, 450, 550
    // To fix offset, we should ensure they scale exactly with the BeerMeter
    
    auto scalePfp = [&](int startY) {
         return scaleRect(1215, startY, 80, 80);
    };

    if (ui->pfpCircle5) ui->pfpCircle5->setGeometry(scalePfp(170));
    if (ui->pfpCircle4) ui->pfpCircle4->setGeometry(scalePfp(258));
    if (ui->pfpCircle3) ui->pfpCircle3->setGeometry(scalePfp(346));
    if (ui->pfpCircle2) ui->pfpCircle2->setGeometry(scalePfp(434));
    if (ui->pfpCircle1) ui->pfpCircle1->setGeometry(scalePfp(522));
    
    // New Piles (User-defined in .ui)
    // AscPile1: Base 275
    bool hasCard1 = !ui->ascPile1->pixmap().isNull();
    int off1 = hasCard1 ? 75 : 0;
    if (ui->ascPile1) ui->ascPile1->setGeometry(scaleRect(275 + off1, 50, 150, 225));
    
    // AscPile2: Base 600
    bool hasCard2 = !ui->ascPile2->pixmap().isNull();
    int off2 = hasCard2 ? 75 : 0;
    if (ui->ascPile2) ui->ascPile2->setGeometry(scaleRect(600 + off2, 50, 150, 225));
    
    // DescPile1: Base 375
    bool hasCard3 = !ui->descPile1->pixmap().isNull();
    int off3 = hasCard3 ? 75 : 0;
    if (ui->descPile1) ui->descPile1->setGeometry(scaleRect(375 + off3, 300, 150, 225));
    
    // DescPile2: Base 700
    bool hasCard4 = !ui->descPile2->pixmap().isNull();
    int off4 = hasCard4 ? 75 : 0;
    if (ui->descPile2) ui->descPile2->setGeometry(scaleRect(700 + off4, 300, 150, 225));

    // Turn Label (styled as a container)
    // Rect: (20, 280, 220, 100) -> Slightly wider
    if (ui->turnLabel) {
        ui->turnLabel->setGeometry(scaleRect(20, 280, 220, 100));
        ui->turnLabel->setStyleSheet("border-image: url(Resources/TextBox_1-2_Small.png); background-color: transparent; border: none;");
        ui->turnLabel->lower(); // Put behind pfp/username
    }
    
    // Profile Picture in Turn Label
    // Rect relative to turnLabel roughly (30, 310, 40, 40) - Smaller and better centered
    if (ui->pfpLabel) {
        ui->pfpLabel->setGeometry(scaleRect(30, 310, 40, 40));
        ui->pfpLabel->setScaledContents(true);
        // Circular styling (placeholder until image loading)
        ui->pfpLabel->setStyleSheet(R"(
            QLabel {
                background-color: #deaf11;
                border: 2px solid #f3d05a;
                border-radius: 20px; 
            }
        )");
        ui->pfpLabel->raise();
    }
    
    // Username Label in Turn Label
    if (ui->usernameLabel) {
        ui->usernameLabel->setGeometry(scaleRect(80, 295, 150, 70)); // Moved left, wider
        ui->usernameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter); // Align left to not overlap pfp
        ui->usernameLabel->setStyleSheet(R"(
            QLabel {
                font-family: "Jersey 15";
                font-size: 20px;
                color: #f3d05a;
                font-weight: bold;
                background: transparent;
                padding-left: 5px;
            }
        )");
        ui->usernameLabel->raise();
    }
    
    // End Turn Button
    if (ui->endTurnButton) {
        ui->endTurnButton->setGeometry(scaleRect(20, 150, 200, 100));
        ui->endTurnButton->setCursor(Qt::PointingHandCursor);
        ui->endTurnButton->setStyleSheet(R"(
            QPushButton {
                border-image: url(Resources/Button.png);
                font-family: 'Jersey 15';
                font-size: 30px;
                color: white;
                font-weight: bold;
                padding-bottom: 5px;
            }
            QPushButton:pressed {
                border-image: url(Resources/Button_Pressed.png);
                padding-top: 5px;
            }
            QPushButton:disabled {
                opacity: 0.5;
                color: rgba(255, 255, 255, 0.5);
            }
        )");
        // Text is set in UI file ("END TURN")
    }

    // Ability Button Styling
    if (ui->abilityButton) {
        ui->abilityButton->setGeometry(scaleRect(20, 420, 200, 70));
        ui->abilityButton->setCursor(Qt::PointingHandCursor);
        ui->abilityButton->setStyleSheet(R"(
            QPushButton {
                border-image: url(Resources/Button.png);
                font-family: 'Jersey 15';
                font-size: 30px;
                color: white;
                font-weight: bold;
                padding-bottom: 5px;
            }
            QPushButton:pressed {
                border-image: url(Resources/Button_Pressed.png);
                padding-top: 5px;
            }
            QPushButton:disabled {
                opacity: 0.5;
                color: rgba(255, 255, 255, 0.5);
            }
        )");
    }

    // Pile Light Indicators (to the left of each pile)
    if (ui->asc1Light) ui->asc1Light->setGeometry(scaleRect(255, 50, 10, 225));
    if (ui->asc2Light) ui->asc2Light->setGeometry(scaleRect(580, 50, 10, 225));
    if (ui->desc1Light) ui->desc1Light->setGeometry(scaleRect(355, 300, 10, 225));
    if (ui->desc2Light) ui->desc2Light->setGeometry(scaleRect(680, 300, 10, 225));
    
    // ... existing ChatWidget logic ...
    if (ui->chatWidget) {
        int w = 300;
        int h = 200;
        int x = 120; // Specified by user in UI
        
        // Peek amount
        int peekAmount = 40;
        
        // Calculate target Y
        int designY = m_isChatOpen ? (designH - h) : (designH - peekAmount);
        
        // Apply scaling
        QRect finalRect = scaleRect(x, designY, w, h);
        ui->chatWidget->setGeometry(finalRect);
        
        // Ensure it's above other things if needed
        // Ensure it's above other things if needed
        ui->chatWidget->raise();
    }
        
    // 7) Soothsayer Panels
    // Base Design: Panel(935, Y, 250, 80). Circles at X=1215. Panel Right Edge at 1185 (Gap 30).
    // Y positions correspond to pfpCircles.
    // Circle Ys (Design): 522, 434, 346, 258, 170. (Indices 1 to 5)
    // Note: pfpCircle1 is at 522. pfpCircle5 is at 170.
    
    // Design constants needed again since they were local to original resizeUI
    // const float designH = 768.0f; // Already defined at top of function

    int pfpY[5] = { 522, 434, 346, 258, 170 }; // Y for 1, 2, 3, 4, 5
    
    // We want to hide them initially (Off-screen Right).
    // On-screen X: 935 scaled. Off-screen X: 1366 scaled.
    // However, if we move them off-screen, the animation manages the transition.
    // resizeUI's job is to ensure they are scaled correctly.
    // If we just scale them to "Hidden" by default, they will disappear if we resize window during animation?
    // Let's rely on 'm_wasSoothActive' (synced with state).
    bool showSooth = (m_networkManager && m_wasSoothActive); 
    
    // NOTE: individual player visibility is handled in updateSoothsayerView, but here we set global position
    // If showSooth is true, we place them at TARGET. If false, HIDDEN.
    // When strictly resizing, we snap to state.
    
    for(int i=1; i<=5; ++i) {
        QWidget* panel = findChild<QWidget*>(QString("soothsayer%1").arg(i));
        if(!panel) continue;
        
        // Base Design Geometry
        int designX = 935;
        int designY = pfpY[i-1];
        int designW = 250;
        int designH_Panel = 80;
        
        QRect targetRect = scaleRect(designX, designY, designW, designH_Panel);
        QRect hiddenRect = scaleRect(1366, designY, designW, designH_Panel); // Off-screen right
        
        // Set background image for the panel
        panel->setStyleSheet("background-image: url(Resources/TextBox_1-2_Small.png); background-repeat: no-repeat; background-position: center;");
        
        if (showSooth) {
             panel->setGeometry(targetRect);
        } else {
             panel->setGeometry(hiddenRect);
        }
        
        // Scale Inner Children (User Label, Cards) relative to GLOBAL Form coordinates
        // Since they are siblings in XML, we scale them independently relative to Form
        
        // Decide Base X based on Active state to keep them relative to panel
        // If panel is hidden (1366), children should be at 1366 + relative.
        // If panel is target (935), children should be at 935 + relative.
        int baseX = showSooth ? designX : 1366;
        
        QWidget* userLbl = findChild<QWidget*>(QString("ssUser%1").arg(i));
        if(userLbl) userLbl->setGeometry(scaleRect(baseX + 10, designY + 5, 200, 15));
        
        for(int c=1; c<=6; ++c) {
             QWidget* cardLbl = findChild<QWidget*>(QString("ssCard%1_%2").arg(i).arg(c));
             if(cardLbl) cardLbl->setGeometry(scaleRect(baseX + 10 + (c-1)*40, designY + 30, 30, 45));
        }
    }
}


void GameWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    if (!m_backgroundPixmap.isNull()) {
        painter.drawPixmap(rect(), m_backgroundPixmap);
    }
    else {
        painter.fillRect(rect(), QColor("#0d0a47"));
    }

    QWidget::paintEvent(event);
}

bool GameWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->chatButton) {
        if (event->type() == QEvent::MouseButtonPress) {
            QPixmap pressed("Resources/Button_Chat_Pressed.png");
            if (!pressed.isNull()) {
                ui->chatButton->setPixmap(pressed);
                ui->chatButton->setScaledContents(true);
            }
            return true; // Event handled
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            QPixmap normal("Resources/Button_Chat.png");
            if (!normal.isNull()) {
                ui->chatButton->setPixmap(normal);
                ui->chatButton->setScaledContents(true);
            }
            toggleChat();
            return true; // Event handled
        }
    }
    
    
    // Hand Card Hover & Click Handling
    bool isHandCard = (ui && (obj == ui->hand1 || obj == ui->hand2 || obj == ui->hand3 || 
                       obj == ui->hand4 || obj == ui->hand5 || obj == ui->hand6));
    
    // Check if object is a pile
    bool isPile = (ui && (obj == ui->ascPile1 || obj == ui->ascPile2 || 
                          obj == ui->descPile1 || obj == ui->descPile2 ||
                          obj == ui->underAsc1 || obj == ui->underAsc2 ||
                          obj == ui->underDesc1 || obj == ui->underDesc2));

    if (isHandCard) {
        if (event->type() == QEvent::Enter) {
            QWidget* card = static_cast<QWidget*>(obj);
            
            // Bring to front
            card->raise();
            
            // Pop up animation
            float scaleY = static_cast<float>(height()) / 768.0f;
            int offset = static_cast<int>(50 * scaleY);
            
            QRect currentGeom = card->geometry();
            
            QPropertyAnimation* anim = new QPropertyAnimation(card, "geometry");
            anim->setDuration(100);
            anim->setStartValue(currentGeom);
            
            // Recalculate 'baseY' from resizeUI logic for accuracy
            int baseY = static_cast<int>(630 * scaleY);
            int targetY = baseY - offset;
            
            anim->setEndValue(QRect(currentGeom.x(), targetY, currentGeom.width(), currentGeom.height()));
            anim->start(QAbstractAnimation::DeleteWhenStopped);
            
            return true;
        }
        else if (event->type() == QEvent::Leave) {
            QWidget* card = static_cast<QWidget*>(obj);
            
            // Don't animate down if this card is currently selected
            if (card == m_selectedCardWidget) {
                return true;
            }
            
            // Animate back down
            float scaleY = static_cast<float>(height()) / 768.0f;
            int baseY = static_cast<int>(630 * scaleY);
            
            QRect currentGeom = card->geometry();
            
            QPropertyAnimation* anim = new QPropertyAnimation(card, "geometry");
            anim->setDuration(100);
            anim->setStartValue(currentGeom);
            anim->setEndValue(QRect(currentGeom.x(), baseY, currentGeom.width(), currentGeom.height()));
            anim->start(QAbstractAnimation::DeleteWhenStopped);
            
            return true;
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            QWidget* clickedCard = static_cast<QWidget*>(obj);
            int cardValue = clickedCard->property("cardValue").toInt();
            
            if (cardValue <= 0) {
                qDebug() << "Clicked card has invalid value";
                return true;
            }
            
            // Toggle selection: if clicking the same card, deselect it
            if (m_selectedCardWidget == clickedCard) {
                clearCardSelection(); 
                qDebug() << "Deselected card";
            } else {
                // Clear previous selection first
                clearCardSelection();
                
                // Select new card
                m_selectedCardWidget = clickedCard;
                m_selectedCardImagePath = "Resources/" + QString::number(cardValue) + ".png";
                
                // Ensure card is animated UP (in case it was down from a deselect or otherwise)
                float scaleY = static_cast<float>(height()) / 768.0f;
                int baseY = static_cast<int>(630 * scaleY);
                int offset = static_cast<int>(50 * scaleY);
                int targetY = baseY - offset;
                
                QRect currentGeom = m_selectedCardWidget->geometry();
                
                // Only animate if not already up (tolerance of 1-2 pixels)
                if (abs(currentGeom.y() - targetY) > 2) {
                    QPropertyAnimation* anim = new QPropertyAnimation(m_selectedCardWidget, "geometry");
                    anim->setDuration(100);
                    anim->setStartValue(currentGeom);
                    anim->setEndValue(QRect(currentGeom.x(), targetY, currentGeom.width(), currentGeom.height()));
                    anim->start(QAbstractAnimation::DeleteWhenStopped);
                }
                
                // Card stays elevated (Leave event won't animate it down)
                
                qDebug() << "Selected card:" << cardValue;
                
                // Update pile light indicators based on selected card
                updatePileClickability();
            }
            
            return true;
        }
    }
    else if (isPile) {
        if (event->type() == QEvent::MouseButtonPress) {
            // Block if not player's turn
            if (!m_isMyTurn) {
                qDebug() << "Cannot place card - not your turn";
                return true;
            }
            
            // Block if no card selected
            if (!m_selectedCardWidget || m_selectedCardImagePath.isEmpty()) {
                qDebug() << "No card selected";
                return true;
            }
            
            // Place Card
            QLabel* pileLabel = static_cast<QLabel*>(obj);
            
            // Identify Pile Index
            int pileIndex = -1;
            // Check both Top and Base piles
            if (pileLabel == ui->ascPile1 || pileLabel == ui->underAsc1) pileIndex = 0;
            else if (pileLabel == ui->ascPile2 || pileLabel == ui->underAsc2) pileIndex = 1;
            else if (pileLabel == ui->descPile1 || pileLabel == ui->underDesc1) pileIndex = 2;
            else if (pileLabel == ui->descPile2 || pileLabel == ui->underDesc2) pileIndex = 3;

            // Get Card Value
            int cardValue = m_selectedCardWidget->property("cardValue").toInt();
            
            // Validate card placement
            if (pileIndex == -1 || cardValue <= 0) {
                qDebug() << "Invalid pile index or card value";
                return true;
            }

            if (!canPlaceCardOnPile(cardValue, pileIndex)) {
                qDebug() << "Illegal move - card" << cardValue << "cannot be placed on pile" << pileIndex;
                return true;
            }
            
            qDebug() << "Placing card. PileIndex:" << pileIndex 
                     << " CardValue:" << cardValue;
            
            if (m_networkManager) {
                QJsonObject action;
                action["type"] = "game_action";
                action["action"] = "play_card";
                action["card_value"] = cardValue;
                action["pile_index"] = pileIndex + 1; // Server expects 1-based index (1-4)
                action["lobby_id"] = QString::fromStdString(m_lobbyId);
                action["user_id"] = m_userId;
                
                m_networkManager->sendGameAction(action);
                qDebug() << "Sent play_card action to server";
                
                // Increment cards played logic handled here client-side for immediate feedback, 
                // but strictly should be on server confirmation. 
                // For now assuming success to update UI immediately.
                if (m_isMyTurn) {
                    m_cardsPlayedThisTurn++;
                    updateEndTurnButtonState();
                }

                // Reset selection
                clearCardSelection();
            }
            return true; 
        }
    }
    return QWidget::eventFilter(obj, event);
}


void GameWindow::loadGameImage()
{
    // Load background image ONCE and cache it
    m_backgroundPixmap = QPixmap("Resources/TableMC.png");

    if (m_backgroundPixmap.isNull()) {
        qWarning() << "Failed to load TableMC.png - using fallback color";
        m_backgroundPixmap = QPixmap(1, 1);
        m_backgroundPixmap.fill(QColor("#0d0a47"));
    }

    // Helper lambda to load pixmap into label safely
    auto loadPixmap = [](QLabel* label, const QString& path, const QString& name) {
        if (label) {
            QPixmap pix(path);
            if (!pix.isNull()) {
                label->setPixmap(pix);
                label->setScaledContents(true);
            } else {
                qWarning() << "Failed to load" << name << "from" << path;
            }
        }
    };

    // Load Banner
    loadPixmap(ui->bannerLabel, "Resources/BannerTableMC.png", "BannerTableMC.png");

    // Load Beer Meter
    loadPixmap(ui->beerLabel, "Resources/BeerMeter1TableMC.png", "BeerMeter1TableMC.png");

    // Load Ascending Piles (1 and 2) -> 1.png
    loadPixmap(ui->underAsc1, "Resources/1.png", "1.png");
    loadPixmap(ui->underAsc2, "Resources/1.png", "1.png");

    // Load Descending Piles (1 and 2) -> 100.png
    // Note: UI has descending2 based on user info
    loadPixmap(ui->underDesc1, "Resources/100.png", "100.png");
    loadPixmap(ui->underDesc2, "Resources/100.png", "100.png"); 
    
    // Setup Placeable Piles (Clickable)
    auto setupPile = [this, &loadPixmap](QLabel* label) {
        if (label) {
            label->installEventFilter(this);
            label->setCursor(Qt::PointingHandCursor);
            // Optional: visual indicator or just transparent initially
        }
    };
    setupPile(ui->ascPile1);
    setupPile(ui->ascPile2);
    setupPile(ui->descPile1);
    setupPile(ui->descPile2);
    
    // Also setup base piles to capture clicks when top pile is hidden
    setupPile(ui->underAsc1);
    setupPile(ui->underAsc2);
    setupPile(ui->underDesc1);
    setupPile(ui->underDesc2);

    // Load Chat Button Default
    loadPixmap(ui->chatButton, "Resources/Button_Chat.png", "Button_Chat.png");

    // Load Card Back
    loadPixmap(ui->cardBack, "Resources/Card_Back.png", "Card_Back.png");
    
    // Initialize Hand Cards (Hidden initially)
    auto initHandCard = [this](QLabel* label) {
        if (label) {
            label->installEventFilter(this);
            label->setCursor(Qt::PointingHandCursor);
            label->setVisible(false); // Hide until we get cards
        }
    };

    initHandCard(ui->hand1);
    initHandCard(ui->hand2);
    initHandCard(ui->hand3);
    initHandCard(ui->hand4);
    initHandCard(ui->hand5);
    initHandCard(ui->hand6);

    // Load Chat Background
    loadPixmap(ui->chatBackground, "Resources/TextBox_1-2.png", "TextBox_1-2.png");
}

void GameWindow::onBackButtonClicked()
{
    hideOverlay();
    emit backToMenuRequested();
}

void GameWindow::toggleChat()
{
    if (!ui || !ui->chatWidget) return;

    m_isChatOpen = !m_isChatOpen;

    const float designH = 768.0f;
    const float designW = 1366.0f;
    
    // Scale factors
    float scaleY = static_cast<float>(height()) / designH;
    float scaleX = static_cast<float>(width()) / designW;

    // Design dimensions
    int h = 200;
    int x = 120; // Corrected to match initial geometry
    int w = 300;
    int peekAmount = 40;

    // Calculate Y in DESIGN coordinates first
    int targetDesignY = m_isChatOpen ? (768 - h) : (768 - peekAmount);
    
    // Convert to SCALED coordinates
    int scaledX = static_cast<int>(x * scaleX);
    int scaledY = static_cast<int>(targetDesignY * scaleY);
    int scaledW = static_cast<int>(w * scaleX);
    int scaledH = static_cast<int>(h * scaleY);

    QRect endRect(scaledX, scaledY, scaledW, scaledH);
    
    QPropertyAnimation* anim = new QPropertyAnimation(ui->chatWidget, "geometry");
    anim->setDuration(300);
    anim->setStartValue(ui->chatWidget->geometry());
    anim->setEndValue(endRect);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void GameWindow::initialize(NetworkManager* networkManager, int userId, const std::string& lobbyId)
{
    m_networkManager = networkManager;
    m_userId = userId;
    m_lobbyId = lobbyId;

    if (m_networkManager) {
        // Disconnect previous connections if any (safety check)
        disconnect(m_networkManager, &NetworkManager::gameConnected, this, &GameWindow::onGameConnected);
        disconnect(m_networkManager, &NetworkManager::gameMessageReceived, this, &GameWindow::onGameMessageReceived);

        connect(m_networkManager, &NetworkManager::gameConnected, this, &GameWindow::onGameConnected);
        connect(m_networkManager, &NetworkManager::gameMessageReceived, this, &GameWindow::onGameMessageReceived);

        m_networkManager->connectToGame(m_lobbyId, m_userId);
    }
    
    // Ensure state is clean before starting new game
    resetGameState();
}

void GameWindow::onGameConnected()
{
    qDebug() << "GameWindow: Connected to Game WebSocket";
}

void GameWindow::onGameMessageReceived(const QJsonObject& message)
{
    QString type = message["type"].toString();

    if (type == "game_state") {
        handleGameState(message);
    }
    else if (type == "chat") {
        QString username = message["username"].toString();
        QString msg = message["message"].toString();
        
        if (m_chatDisplay) {
            // Bold username, normal message
            // We can use HTML
            QString html = QString("<b>%1:</b><br>%2").arg(username).arg(msg);
            m_chatDisplay->append(html);
            // Auto scroll is built-in usually, but to be sure:
            m_chatDisplay->moveCursor(QTextCursor::End);
        }
    }
    else if (type == "game_over") {
        QString result = message["result"].toString(); // "won" or "lost"
        
        if (result == "won") {
            WinDialog* dialog = new WinDialog(this);
            connect(dialog, &WinDialog::backToMenuRequested, this, &GameWindow::backToMenuRequested, Qt::QueuedConnection);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->open();
        } else {
            LossDialog* dialog = new LossDialog(this);
            connect(dialog, &LossDialog::backToMenuRequested, this, &GameWindow::backToMenuRequested, Qt::QueuedConnection);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->open();
        }
    }
    else if (type == "error") {
        QString errorMsg = message["message"].toString();
        qDebug() << "Game Error:" << errorMsg;
    }
}

void GameWindow::handleGameState(const QJsonObject& state)
{
    // Update Piles
    if (state.contains("piles")) {
        updatePiles(state["piles"].toArray());
    }

    // Update Deck Count (for Endgame Logic)
    if (state.contains("deck_count")) {
        int oldDeckCount = m_deckCount;
        m_deckCount = state["deck_count"].toInt();
        
        // If deck count changed, re-evaluate button state
        if (oldDeckCount != m_deckCount) {
             updateEndTurnButtonState();
             
             // Trigger Deck Exit Animation if deck becomes empty
             if (m_deckCount == 0 && oldDeckCount > 0) {
                 if (ui && ui->cardBack) {
                     QRect currentGeom = ui->cardBack->geometry();
                     QPropertyAnimation* anim = new QPropertyAnimation(ui->cardBack, "geometry");
                     anim->setDuration(1500);
                     anim->setStartValue(currentGeom);
                     anim->setEndValue(QRect(currentGeom.x(), -currentGeom.height(), currentGeom.width(), currentGeom.height()));
                     anim->setEasingCurve(QEasingCurve::InBack);
                     anim->start(QAbstractAnimation::DeleteWhenStopped);
                 }
             }
        }
    }
    
    // Update Required Cards (from Server)
    if (state.contains("current_required")) {
        int oldReq = m_requiredCards;
        m_requiredCards = state["current_required"].toInt();
        if (oldReq != m_requiredCards) {
            updateEndTurnButtonState();
        }
    }

    // Update My Hand and Opponents
    if (state.contains("players")) {
        QJsonArray players = state["players"].toArray();
        QJsonArray myHand;
        
        // --- BeerMeter and PFP Logic ---
        int playerCount = players.size();
        
        // 1. Update BeerMeter Image
        if (ui->beerLabel) {
            QString beerImg = "Resources/BeerMeter2TableMC.png"; // Default fallback
            
            if (playerCount == 5) {
                beerImg = "Resources/BeerMeter1TableMC.png";
            } else if (playerCount == 4) {
                beerImg = "Resources/BeerMeter2TableMC.png";
            } else if (playerCount == 3) {
                beerImg = "Resources/BeerMeter3TableMC.png";
            } else if (playerCount == 2) {
                beerImg = "Resources/BeerMeter4TableMC.png";
            }
            
            QPixmap pm(beerImg);
            if (!pm.isNull()) {
                ui->beerLabel->setPixmap(pm);
                ui->beerLabel->setScaledContents(true);
            }
        }
        
        // 2. Update PFP Circles
        // Helper to get circle widget by index 0-4 -> pfpCircle1-5
        auto getCircle = [this](int idx) -> QLabel* {
            switch(idx) {
                case 0: return ui->pfpCircle1;
                case 1: return ui->pfpCircle2;
                case 2: return ui->pfpCircle3;
                case 3: return ui->pfpCircle4;
                case 4: return ui->pfpCircle5;
                default: return nullptr;
            }
        };

        // Clear all circles first
        for (int i = 0; i < 5; i++) {
            if (QLabel* c = getCircle(i)) c->clear();
        }

        // Fill circles
        for (int i = 0; i < playerCount && i < 5; ++i) {
             QJsonObject p = players[i].toObject();
             int pid = p["user_id"].toInt();
             
             if (QLabel* circle = getCircle(i)) {
                 QPixmap avatar = UiUtils::GetAvatar(pid, m_networkManager);
                 if (!avatar.isNull()) {
                     // Make it circular? UiUtils::GetAvatar usually returns a square.
                     // We can just set it for now or mask it.
                     // Ideally we mask it to be a circle.
                     QPixmap circularAvatar(75, 75);
                     circularAvatar.fill(Qt::transparent);
                     QPainter painter(&circularAvatar);
                     painter.setRenderHint(QPainter::Antialiasing);
                     painter.setRenderHint(QPainter::SmoothPixmapTransform);
                     QPainterPath path;
                     path.addEllipse(0, 0, 75, 75);
                     painter.setClipPath(path);
                     painter.drawPixmap(0, 0, 75, 75, avatar);
                     
                     circle->setPixmap(circularAvatar);
                     circle->setScaledContents(true);
                     circle->setVisible(true);
                 }
             }
        }
        // -------------------------------

        for (const auto& p : players) {
            QJsonObject player = p.toObject();
            if (player["user_id"].toInt() == m_userId) {
                if (player.contains("hand")) {
                    updateHand(player["hand"].toArray());
                }
                // --- Ability Name Update ---
                if (player.contains("ability")) {
                    QString abilityName = player["ability"].toString();
                    m_myAbilityName = abilityName; // Store for Peasant check
                    
                    // For Gambler, also get uses left
                    if (abilityName == "Gambler" && player.contains("gambler_uses_left")) {
                        m_gamblerUsesLeft = player["gambler_uses_left"].toInt();
                        if (ui->abilityButton) {
                            ui->abilityButton->setText(QString("Gambler\n(%1 left)").arg(m_gamblerUsesLeft));
                        }
                    } 
                    // For Soothsayer, also get uses left
                    else if (abilityName == "Soothsayer" && player.contains("soothsayer_uses_left")) {
                        int soothUsesLeft = player["soothsayer_uses_left"].toInt();
                        if (ui->abilityButton) {
                            ui->abilityButton->setText(QString("Soothsayer\n(%1 left)").arg(soothUsesLeft));
                        }
                    } else {
                        if (ui->abilityButton) {
                            ui->abilityButton->setText(abilityName);
                        }
                    }
                }
                
                // Ability Flags
                if (player.contains("is_hp_active")) {
                    m_isHPMode = player["is_hp_active"].toBool();
                }
                

                
                // Ability usability flag from server (default to false if not present)
                m_canUseAbility = player.value("can_use_ability").toBool(false);
                // ---------------------------
            }
        }
        updateOpponents(players);

        // Check if ANY player has Soothsayer active
        bool anySoothActive = false;
        for (const auto& p : players) {
            if (p.toObject().value("is_sooth_active").toBool()) {
                anySoothActive = true;
                break;
            }
        }
        
        // Update view if active, or hide if it WAS active but now isn't
        if (anySoothActive) {
            updateSoothsayerView(players);
        } else if (m_wasSoothActive) {
            m_wasSoothActive = false;
            hideSoothsayerPanels();
        }
    }

    // Turn indication
    if (state.contains("current_turn_username")) {
        QString currentTurn = state["current_turn_username"].toString();
        int currentTurnId = state["current_turn_player_id"].toInt();
        
        // Track if it's this player's turn
        bool wasMyTurn = m_isMyTurn;
        m_isMyTurn = (currentTurnId == m_userId);
        
        // If turn changed, reset counter
        if (m_isMyTurn && !wasMyTurn) {
            m_cardsPlayedThisTurn = 0;
            m_abilityUsedThisTurn = false;
        }

        if (ui->usernameLabel) {
             ui->usernameLabel->setVisible(false); // Hide potentially unused label
        }
        if (ui->pfpLabel) {
             ui->pfpLabel->setVisible(false); // Hide potentially unused label
        }

        // Update Turn Label with PFP and Text using HTML
        if (m_turnLabel) {
            QString turnText = m_isMyTurn ? "YOUR TURN" : (currentTurn + "'s TURN");
            
            // Get Avatar
            QPixmap avatar = UiUtils::GetAvatar(currentTurnId, m_networkManager);
            if (avatar.isNull()) {
                // Return placeholder logical color or handling
                // For now just empty image
                 m_turnLabel->setText(QString("<div style='display: flex; align-items: center; justify-content: center;'><span style='font-size: 20px; font-weight: bold; color: #f3d05a;'>%1</span></div>").arg(turnText));
            } else {
                QByteArray bArray;
                QBuffer buffer(&bArray);
                buffer.open(QIODevice::WriteOnly);
                avatar.save(&buffer, "PNG");
                QString base64 = QString::fromLatin1(bArray.toBase64().data());
                
                // HTML Layout: Image left, Text right, vertically centered
                // Note: QLabel rich text support is limited. Tables are robust.
                QString html = QString(
                    "<table border='0' cellspacing='5' cellpadding='0'>"
                    "<tr>"
                    "<td valign='middle'><img src='data:image/png;base64,%1' width='40' height='40'></td>"
                    "<td valign='middle'><span style='font-size: 20px; font-weight: bold; color: #f3d05a;'>%2</span></td>"
                    "</tr>"
                    "</table>"
                ).arg(base64).arg(turnText);
                
                m_turnLabel->setText(html);
                // Ensure text format allows rich text
                m_turnLabel->setTextFormat(Qt::RichText);
                m_turnLabel->setAlignment(Qt::AlignCenter);
            }
        }
        
        m_lastTurnUserId = currentTurnId;
        
        // Update End Turn Button State based on new rules
        updateEndTurnButtonState();
        
        // Update Ability Button State (disabled when not turn or used)
        updateAbilityButtonState();

        // Update pile clickability when turn changes
        if (wasMyTurn != m_isMyTurn) {
            updatePileClickability();
            // Clear selection when turn ends
            if (!m_isMyTurn) {
                clearCardSelection();
                // Also hide Soothsayer panels when our turn ends
                if (m_wasSoothActive) {
                    m_wasSoothActive = false;
                    hideSoothsayerPanels();
                }
            }
        }
        
        qDebug() << "Current Turn:" << currentTurn << "(My turn:" << m_isMyTurn << ")";
    }
    
    // ...
}

void GameWindow::updateEndTurnButtonState()
{
    if (!ui->endTurnButton) return;

    // Logic: Enabled ONLY if it's my turn AND I've played at least the REQUIRED cards
    // The server tells us 'current_required', which accounts for Deck size AND Abilities (Gambler/TaxEvader)
    
    bool canEnd = m_isMyTurn && (m_cardsPlayedThisTurn >= m_requiredCards);
    
    // Visual transparency update
    if (canEnd) {
        ui->endTurnButton->setEnabled(true);
        ui->endTurnButton->setStyleSheet(R"(
            QPushButton {
                border-image: url(Resources/Button.png);
                font-family: 'Jersey 15';
                font-size: 30px;
                color: white;
                font-weight: bold;
                padding-bottom: 5px;
                opacity: 1.0;
            }
            QPushButton:pressed {
                border-image: url(Resources/Button_Pressed.png);
                padding-top: 5px;
            }
        )");
    } else {
        ui->endTurnButton->setEnabled(false); // Make it unclickable
        ui->endTurnButton->setStyleSheet(R"(
            QPushButton {
                border-image: url(Resources/Button.png);
                font-family: 'Jersey 15';
                font-size: 30px;
                color: rgba(255, 255, 255, 0.5); 
                font-weight: bold;
                padding-bottom: 5px;
            }
        )");
         // Using QGraphicsOpacityEffect would be better for full widget opacity, 
         // but stylesheet color alpha works for text/border-image content usually if supported. 
         // 'opacity' property in stylesheet might not work on all widgets directly.
         // Let's rely on setEnabled(false) visual + dim text.
    }
}    

void GameWindow::updateAbilityButtonState()
{
    if (!ui->abilityButton) return;
    
    // Ability is usable ONLY if: 1) It's my turn, 2) Server says we can use it, 3) Not already used this turn
    bool canUse = m_isMyTurn && m_canUseAbility && !m_abilityUsedThisTurn;
    
    if (canUse) {
        ui->abilityButton->setEnabled(true);
        ui->abilityButton->setCursor(Qt::PointingHandCursor);
        ui->abilityButton->setStyleSheet(R"(
            QPushButton {
                border-image: url(Resources/Button.png);
                font-family: 'Jersey 15';
                font-size: 30px;
                color: white;
                font-weight: bold;
                padding-bottom: 5px;
            }
            QPushButton:pressed {
                border-image: url(Resources/Button_Pressed.png);
                padding-top: 5px;
            }
        )");
    } else {
        ui->abilityButton->setEnabled(false);
        ui->abilityButton->setCursor(Qt::ForbiddenCursor);
        ui->abilityButton->setStyleSheet(R"(
            QPushButton {
                border-image: url(Resources/Button.png);
                font-family: 'Jersey 15';
                font-size: 30px;
                color: rgba(255, 255, 255, 0.5);
                font-weight: bold;
                padding-bottom: 5px;
            }
        )");
    }
}


void GameWindow::updatePiles(const QJsonArray& piles)
{
    // Protocol: [asc1, asc2, desc1, desc2]
    // Piles: {"top_card": "s", "count": i}
    
    auto updatePileLabel = [this](int index, QLabel* pileLabel, const QJsonObject& pileData, QLabel* baseLabel) {
        if (!pileLabel || !baseLabel || index < 0 || index >= 4) return;
        
        int count = pileData["count"].toInt();
        QString cardVal = pileData["top_card"].toString();
        
        bool changed = (m_lastPileTops[index] != cardVal);
        m_lastPileTops[index] = cardVal; // Update state
        
        // Store pile top value for client-side validation
        bool ok;
        int topValue = cardVal.toInt(&ok);
        if (ok) {
            m_pileTopValues[index] = topValue;
        }

        if (count <= 1) {
            // Only 1 card (the base card), hide the played pile widget
            pileLabel->setVisible(false);
        } else {
            // Show new card
            QString path = "Resources/" + cardVal + ".png";
            QPixmap pix(path);
            if (!pix.isNull()) {
                pileLabel->setPixmap(pix);
                pileLabel->setScaledContents(true);
                pileLabel->setVisible(true);
                
                // Animation Logic - ONLY if changed
                if (changed) {
                    // Base Geometry comes from baseLabel
                    QRect baseRect = baseLabel->geometry();
                    
                    // Offset Geometry (Base X + 75)
                    QRect targetRect = baseRect;
                    targetRect.moveLeft(baseRect.x() + 75);
                    
                    QPropertyAnimation* anim = new QPropertyAnimation(pileLabel, "geometry");
                    anim->setDuration(300);
                    anim->setStartValue(baseRect);
                    anim->setEndValue(targetRect);
                    anim->setEasingCurve(QEasingCurve::OutBack);
                    anim->start(QAbstractAnimation::DeleteWhenStopped);
                }
            }
        }
    };

    if (piles.size() >= 4) {
        if (ui->ascPile1 && ui->underAsc1) updatePileLabel(0, ui->ascPile1, piles[0].toObject(), ui->underAsc1);
        if (ui->ascPile2 && ui->underAsc2) updatePileLabel(1, ui->ascPile2, piles[1].toObject(), ui->underAsc2);
        if (ui->descPile1 && ui->underDesc1) updatePileLabel(2, ui->descPile1, piles[2].toObject(), ui->underDesc1);
        if (ui->descPile2 && ui->underDesc2) updatePileLabel(3, ui->descPile2, piles[3].toObject(), ui->underDesc2);
    }
}

void GameWindow::updateHand(const QJsonArray& hand)
{
    // Clear existing hand widgets or update them
    // Logic: We have 6 static slots: hand1...hand6
    // We bind visible cards to slots. Hide unused slots.

    QVector<QLabel*> handSlots = { ui->hand1, ui->hand2, ui->hand3, ui->hand4, ui->hand5, ui->hand6 };
    
    for (int i = 0; i < handSlots.size(); ++i) {
        if (i < hand.size()) {
            QString cardVal = hand[i].toString();
            QString path = "Resources/" + cardVal + ".png";
            
            QPixmap pix(path);
            if (!pix.isNull()) {
                handSlots[i]->setPixmap(pix);
                handSlots[i]->setScaledContents(true);
                handSlots[i]->setVisible(true);
                // Store card value for logic? We used m_selectedCardImagePath path logic before
                // We'll rely on the existing click handler getting the pixmap or object
                // But better to store the value as property
                handSlots[i]->setProperty("cardValue", cardVal.toInt());
            }
        } else {
            handSlots[i]->setVisible(false);
        }
    }
}

void GameWindow::updateOpponents(const QJsonArray& players)
{
    // TODO: Visualize other players (hand counts)
    // For now we don't have UI slots for opponents in the provided UI design description
    // existing GameWindow code didn't have opponent slots visible in setupUI
}

void GameWindow::sendEndTurnAction()
{
    if (m_networkManager) {
        QJsonObject action;
        action["type"] = "game_action";
        action["action"] = "end_turn";
        action["lobby_id"] = QString::fromStdString(m_lobbyId);
        action["user_id"] = m_userId;
        
        m_networkManager->sendGameAction(action);
    }
}

void GameWindow::sendMessage()
{
    if (!ui->chatInput || !ui->chatHistory) return;

    QString text = ui->chatInput->text().trimmed();
    if (text.isEmpty()) return;

    if (m_networkManager) {
        QJsonObject chatMsg;
        chatMsg["type"] = "chat";
        chatMsg["lobby_id"] = QString::fromStdString(m_lobbyId);
        chatMsg["user_id"] = m_userId;
        // chatMsg["username"] = ...; Server knows username from user_id or we can send it
        chatMsg["message"] = text;
        
        m_networkManager->sendGameAction(chatMsg);
    }

    ui->chatInput->clear();
}

bool GameWindow::canPlaceCardOnPile(int cardValue, int pileIndex) const
{
    if (pileIndex < 0 || pileIndex >= 4) return false;
    
    int topValue = m_pileTopValues[pileIndex];
    
    // Piles 0 and 1 are ascending (start at 1, go up OR exactly -10)
    // Piles 2 and 3 are descending (start at 100, go down OR exactly +10)
    bool isAscending = (pileIndex < 2);
    
    if (m_isHPMode) return true; // Harry Potter can place anywhere

    if (isAscending) {
        // Ascending: card must be higher OR exactly 10 less
        return (cardValue > topValue) || (cardValue == topValue - 10);
    } else {
        // Descending: card must be lower OR exactly 10 more
        return (cardValue < topValue) || (cardValue == topValue + 10);
    }
}

void GameWindow::updatePileClickability()
{
    // Get light labels for each pile
    QVector<QLabel*> lightLabels = { ui->asc1Light, ui->asc2Light, ui->desc1Light, ui->desc2Light };
    QVector<QLabel*> pileLabels = { ui->ascPile1, ui->ascPile2, ui->descPile1, ui->descPile2 };
    QVector<QLabel*> basePileLabels = { ui->underAsc1, ui->underAsc2, ui->underDesc1, ui->underDesc2 };
    
    int selectedCardValue = 0;
    if (m_selectedCardWidget) {
        selectedCardValue = m_selectedCardWidget->property("cardValue").toInt();
    }
    
    for (int i = 0; i < 4; ++i) {
        if (!lightLabels[i]) continue;
        
        QString lightColor = "";
        
        if (!m_isMyTurn || selectedCardValue <= 0) {
            // No card selected or not player's turn - hide lights (transparent)
            lightColor = "background-color: transparent;";
            if (pileLabels[i]) pileLabels[i]->setCursor(m_isMyTurn ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
            if (basePileLabels[i]) basePileLabels[i]->setCursor(m_isMyTurn ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
        } else {
            int topValue = m_pileTopValues[i];
            bool isAscending = (i < 2);
            
            // Check for ±10 special move
            bool isTenMove = false;
            if (isAscending) {
                isTenMove = (selectedCardValue == topValue - 10);
            } else {
                isTenMove = (selectedCardValue == topValue + 10);
            }
            
            bool canPlace = canPlaceCardOnPile(selectedCardValue, i);
            
            if (isTenMove) {
                // Purple for ±10 special move
                lightColor = "background-color: #9B30FF; border-radius: 3px;";
                if (pileLabels[i]) pileLabels[i]->setCursor(Qt::PointingHandCursor);
                if (basePileLabels[i]) basePileLabels[i]->setCursor(Qt::PointingHandCursor);
            } else if (m_isHPMode) {
                 // Purple for Harry Potter Mode (Everything valid)
                lightColor = "background-color: #9B30FF; border-radius: 3px;";
                if (pileLabels[i]) pileLabels[i]->setCursor(Qt::PointingHandCursor);
                if (basePileLabels[i]) basePileLabels[i]->setCursor(Qt::PointingHandCursor);
            } else if (canPlace) {
                // Green for valid regular move
                lightColor = "background-color: #00FF00; border-radius: 3px;";
                if (pileLabels[i]) pileLabels[i]->setCursor(Qt::PointingHandCursor);
                if (basePileLabels[i]) basePileLabels[i]->setCursor(Qt::PointingHandCursor);
            } else {
                // Red for invalid move
                lightColor = "background-color: #FF0000; border-radius: 3px;";
                if (pileLabels[i]) pileLabels[i]->setCursor(Qt::ForbiddenCursor);
                if (basePileLabels[i]) basePileLabels[i]->setCursor(Qt::ForbiddenCursor);
            }
        }
        
        lightLabels[i]->setStyleSheet(lightColor);
    }
}

void GameWindow::clearCardSelection()
{
    if (m_selectedCardWidget) {
        // Animate previously selected card back down
        float scaleY = static_cast<float>(height()) / 768.0f;
        int baseY = static_cast<int>(630 * scaleY);
        
        QRect currentGeom = m_selectedCardWidget->geometry();
        
        QPropertyAnimation* anim = new QPropertyAnimation(m_selectedCardWidget, "geometry");
        anim->setDuration(100);
        anim->setStartValue(currentGeom);
        anim->setEndValue(QRect(currentGeom.x(), baseY, currentGeom.width(), currentGeom.height()));
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
    m_selectedCardWidget = nullptr;
    m_selectedCardImagePath.clear();
    
    // Reset pile light indicators
    updatePileClickability();
}

void GameWindow::onAbilityButtonClicked()
{
    // Prevent ability usage if not my turn
    if (!m_isMyTurn) {
         return;
    }
    
    // Check for Peasant - show dialog instead of sending to server
    if (m_myAbilityName == "Peasant") {
        PeasantDialog dialog(this);
        dialog.exec();
        return;
    }

    if (m_networkManager) {
        QJsonObject action;
        action["type"] = "game_action";
        action["action"] = "use_ability";
        action["lobby_id"] = QString::fromStdString(m_lobbyId);
        action["user_id"] = m_userId;
        
        m_networkManager->sendGameAction(action);
        qDebug() << "Sent use_ability action to server";
        
        // For Gambler and Soothsayer, block further uses this turn
        if (m_myAbilityName == "Gambler" || m_myAbilityName == "Soothsayer") {
            m_abilityUsedThisTurn = true;
            updateAbilityButtonState(); // Disable button immediately
        }
    }
}

void GameWindow::updateSoothsayerView(const QJsonArray& players)
{
    // Determine active state change
    bool isActive = false;
    for(const auto& p : players) {
         // Check if ANY player has Soothsayer active
         if (p.toObject().value("is_sooth_active").toBool()) {
             isActive = true;
             break;
         }
    }
    
    if (isActive == m_wasSoothActive) {
        // No state change
        // If Active, we just update the data (hands) without re-animating
        if (isActive) {
             for (int i = 0; i < players.size() && i < 5; ++i) {
                QJsonObject player = players[i].toObject();
                int pid = player["user_id"].toInt();
                if (pid == m_userId) continue;

                int uiIndex = i + 1;
                // Just update text/cards
                QWidget* userLbl = findChild<QWidget*>(QString("ssUser%1").arg(uiIndex));
                if (userLbl) dynamic_cast<QLabel*>(userLbl)->setText(player["username"].toString());
                
                QJsonArray hand = player["hand"].toArray();
                for(int c=1; c<=6; ++c) {
                    QLabel* cardLbl = findChild<QLabel*>(QString("ssCard%1_%2").arg(uiIndex).arg(c));
                    if(cardLbl) {
                        if (c <= hand.size()) {
                            QString val = hand[c-1].toString();
                            // In Qt resource system, assuming prefix is :/cards/Resources or similar
                            // Actually, based on previous code, load logic:
                            QPixmap px(QString("Resources/%1.png").arg(val));
                            if(px.isNull()) px = QPixmap("Resources/Card_Back.png");
                            cardLbl->setPixmap(px);
                            cardLbl->setScaledContents(true);
                            cardLbl->show();
                        } else {
                            cardLbl->hide();
                        }
                    }
                }
             }
        }
        return; 
    }
    
    m_wasSoothActive = isActive;
    
    // Animation Duration
    int dur = 600;

    for (int i = 0; i < players.size() && i < 5; ++i) {
        QJsonObject player = players[i].toObject();
        int pid = player["user_id"].toInt();
        
        // Mapping: players[i] corresponds to pfpCircle(i+1) -> soothsayer(i+1)
        int uiIndex = i + 1;
        
        QWidget* panel = findChild<QWidget*>(QString("soothsayer%1").arg(uiIndex));
        QWidget* userLbl = findChild<QWidget*>(QString("ssUser%1").arg(uiIndex));
        if (!panel || !userLbl) continue;
        
        // Group all widgets for this panel (Panel + User + Cards)
        QList<QWidget*> widgets;
        widgets << panel << userLbl;
        for(int c=1; c<=6; ++c) widgets << findChild<QWidget*>(QString("ssCard%1_%2").arg(uiIndex).arg(c));

        // Start/End Geometry Logic
        // Design: X=935. Hidden: X=1366.
        int designY = 522 - (i)*88; // 522, 434, etc. matches resizeUI logic
        QRect panelTarget = scaleRect(935, designY, 250, 80);
        QRect panelHidden = scaleRect(1366, designY, 250, 80);
        
        if (isActive && pid != m_userId) {
            // SHOW
            // Populate Data
            dynamic_cast<QLabel*>(userLbl)->setText(player["username"].toString());
            QJsonArray hand = player["hand"].toArray();
            for(int c=1; c<=6; ++c) {
                QLabel* cardLbl = findChild<QLabel*>(QString("ssCard%1_%2").arg(uiIndex).arg(c));
                if(cardLbl) {
                    if (c <= hand.size()) {
                        QString val = hand[c-1].toString();
                        QString path = QString(":/cards/Resources/%1.png").arg(val); // Standard path assumption
                        QPixmap pm(path);
                        if(pm.isNull()) pm.load(QString("Resources/%1.png").arg(val)); // Fallback
                        cardLbl->setPixmap(pm.scaled(cardLbl->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                        cardLbl->show();
                    } else {
                        cardLbl->clear();
                    }
                }
            }
            
            // Animate In: Move from Hidden to Target
            // We animate the "Relative Shift" or absolute? Absolute is safer.
            // Problem: User label and cards have DIFFERENT target rects.
            // We need to calculate the "Shift" (Delta).
            // Delta = TargetX - HiddenX = 935 - 1366 = -431 (scaled).
            
            // To animate all sibling widgets together, we can use a QParallelAnimationGroup.
            QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
            
            for(QWidget* w : widgets) {
                if(!w) continue;
                QRect endGeo = w->geometry(); 
                // Wait, if it's already on screen, endGeo is current.
                // We want to force it to Target position.
                // Re-calculate target for this specific widget.
                // Relative to PanelTarget:
                int relX = 0; // relative to 935
                if (w == panel) relX = 0;
                else if (w == userLbl) relX = 10;
                else {
                    // Check valid card
                     QString name = w->objectName();
                     int cIdx = name.right(1).toInt();
                     relX = 10 + (cIdx-1)*40;
                }
                
                int finalX = scaleRect(935 + relX, 0, 0, 0).x();
                int finalY = w->geometry().y(); // Trust current Y from resizeUI? Yes.
                
                QRect startG(scaleRect(1366 + relX, 0, 0, 0).x(), finalY, w->width(), w->height());
                QRect endG(finalX, finalY, w->width(), w->height());
                
                w->show(); // Ensure visible
                
                QPropertyAnimation* anim = new QPropertyAnimation(w, "geometry");
                anim->setDuration(dur);
                anim->setStartValue(startG);
                anim->setEndValue(endG);
                anim->setEasingCurve(QEasingCurve::OutCubic);
                group->addAnimation(anim);
            }
            group->start(QAbstractAnimation::DeleteWhenStopped);
            
        } else {
            // HIDE
            // Animate Out: Move from Current to Hidden
             QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
             for(QWidget* w : widgets) {
                if(!w) continue;
                QRect startG = w->geometry();
                // Calc Hidden X
                 int relX = 0;
                 if (w == panel) relX = 0;
                 else if (w == userLbl) relX = 10;
                 else {
                     QString name = w->objectName();
                     int cIdx = name.right(1).toInt();
                     relX = 10 + (cIdx-1)*40;
                 }
                int hiddenX = scaleRect(1366 + relX, 0, 0, 0).x();
                
                QRect endG(hiddenX, startG.y(), w->width(), w->height());
                
                QPropertyAnimation* anim = new QPropertyAnimation(w, "geometry");
                anim->setDuration(dur);
                anim->setStartValue(startG);
                anim->setEndValue(endG);
                anim->setEasingCurve(QEasingCurve::InCubic);
                group->addAnimation(anim);
             }
              group->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }
}

void GameWindow::hideSoothsayerPanels()
{
    // Quick hide animation for all Soothsayer panels
    int dur = 400;
    
    for (int i = 1; i <= 5; ++i) {
        QWidget* panel = findChild<QWidget*>(QString("soothsayer%1").arg(i));
        QWidget* userLbl = findChild<QWidget*>(QString("ssUser%1").arg(i));
        if (!panel) continue;
        
        QList<QWidget*> widgets;
        widgets << panel << userLbl;
        for(int c=1; c<=6; ++c) widgets << findChild<QWidget*>(QString("ssCard%1_%2").arg(i).arg(c));
        
        QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
        for(QWidget* w : widgets) {
            if(!w) continue;
            QRect startG = w->geometry();
            int hiddenX = scaleRect(1366, 0, 0, 0).x();
            QRect endG(hiddenX, startG.y(), w->width(), w->height());
            
            QPropertyAnimation* anim = new QPropertyAnimation(w, "geometry");
            anim->setDuration(dur);
            anim->setStartValue(startG);
            anim->setEndValue(endG);
            anim->setEasingCurve(QEasingCurve::InCubic);
            group->addAnimation(anim);
        }
        group->start(QAbstractAnimation::DeleteWhenStopped);
    }
}
