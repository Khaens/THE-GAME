#include "GameWindow.h"
#include "ui_GameWindow.h"
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
#include <QDebug>

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
    hide();
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

void GameWindow::resizeUI()
{
    if (!ui) return;

    // Design resolution
    const float designW = 1366.0f;
    const float designH = 768.0f;

    // Current window dimensions
    float currentW = static_cast<float>(this->width());
    float currentH = static_cast<float>(this->height());

    // Calculate scale factors
    float scaleX = currentW / designW;
    float scaleY = currentH / designH;

    // Helper to scale rects
    auto scaleRect = [&](int x, int y, int w, int h) -> QRect {
        return QRect(
            static_cast<int>(x * scaleX),
            static_cast<int>(y * scaleY),
            static_cast<int>(w * scaleX),
            static_cast<int>(h * scaleY)
        );
    };

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
    if (ui->ascending1) ui->ascending1->setGeometry(scaleRect(275, 50, 150, 225));
    
    // Asc2: (750, 50, 125, 187)
    if (ui->ascending2) ui->ascending2->setGeometry(scaleRect(600, 50, 150, 225));
    
    // Desc1: (425, 300, 125, 187)
    if (ui->descending1) ui->descending1->setGeometry(scaleRect(375, 300, 150, 225));
    
    // Desc2: (750, 300, 150, 225)
    if (ui->descending2) ui->descending2->setGeometry(scaleRect(700, 300, 150, 225));

    // Hand Cards
    // Base Y is 630, W 150, H 225
    // Spacing 100px (440, 540, 640...)
    if (ui->hand1) ui->hand1->setGeometry(scaleRect(440, 630, 150, 225));
    if (ui->hand2) ui->hand2->setGeometry(scaleRect(540, 630, 150, 225));
    if (ui->hand3) ui->hand3->setGeometry(scaleRect(640, 630, 150, 225));
    if (ui->hand4) ui->hand4->setGeometry(scaleRect(740, 630, 150, 225));
    if (ui->hand5) ui->hand5->setGeometry(scaleRect(840, 630, 150, 225));
    if (ui->hand6) ui->hand6->setGeometry(scaleRect(940, 630, 150, 225));
    
    // New Piles (User-defined in .ui)
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

    // Chat Widget: (140, 768, 300, 200)
    // Design: W=300, H=200.
    // Logic: 
    // - Closed (Peeking): Y = designH - 40 (shows top 40px)
    // - Open: Y = designH - 200 (fully visible)
    
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
        
        // Ensure it's above other things if needed, but sidebar/hands might overlap
        ui->chatWidget->raise();
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
                          obj == ui->descPile1 || obj == ui->descPile2));

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
            // Select Card
            m_selectedCardWidget = static_cast<QWidget*>(obj);
            
            // Store image path depending on which card it is
            // For now, based on setupHandCard we know the paths:
            if (obj == ui->hand1) m_selectedCardImagePath = "Resources/2.png";
            else if (obj == ui->hand2) m_selectedCardImagePath = "Resources/3.png";
            else if (obj == ui->hand3) m_selectedCardImagePath = "Resources/4.png";
            else if (obj == ui->hand4) m_selectedCardImagePath = "Resources/5.png";
            else if (obj == ui->hand5) m_selectedCardImagePath = "Resources/6.png";
            else if (obj == ui->hand6) m_selectedCardImagePath = "Resources/7.png";
            
            qDebug() << "Selected card: " << m_selectedCardImagePath;
            return true;
        }
    }
    else if (isPile) {
        if (event->type() == QEvent::MouseButtonPress) {
            if (m_selectedCardWidget && !m_selectedCardImagePath.isEmpty()) {
                // Place Card
                QLabel* pileLabel = static_cast<QLabel*>(obj);
                
                // Check if it was empty before (state change)
                bool wasEmpty = pileLabel->pixmap().isNull();
                
                QPixmap pix(m_selectedCardImagePath);
                if (!pix.isNull()) {
                    pileLabel->setPixmap(pix);
                    pileLabel->setScaledContents(true);
                    
                    // Hide original card
                    m_selectedCardWidget->hide();
                    
                    // Reset selection
                    m_selectedCardWidget = nullptr;
                    m_selectedCardImagePath.clear();
                    
                    int baseX = 0;
                    int baseY = 0;
                    if (pileLabel == ui->ascPile1) { baseX = 275; baseY = 50; }
                    else if (pileLabel == ui->ascPile2) { baseX = 600; baseY = 50; }
                    else if (pileLabel == ui->descPile1) { baseX = 375; baseY = 300; }
                    else if (pileLabel == ui->descPile2) { baseX = 700; baseY = 300; }
                    
                    if (baseX != 0) {
                        float designW = 1366.0f;
                        float designH = 768.0f;
                        float sX = width() / designW;
                        float sY = height() / designH;
                        
                        int startX = static_cast<int>(baseX * sX); // Start at BASE (275)
                        int startY = static_cast<int>(baseY * sY);
                        
                        int targetX = static_cast<int>((baseX + 75) * sX); // End at OFFSET (350)
                        int targetY = static_cast<int>(baseY * sY);
                        int targetW = static_cast<int>(150 * sX);
                        int targetH = static_cast<int>(225 * sY);
                        
                        QPropertyAnimation* anim = new QPropertyAnimation(pileLabel, "geometry");
                        anim->setDuration(300);
                        // Force start from BASE so we see the slide
                        anim->setStartValue(QRect(startX, startY, targetW, targetH)); 
                        anim->setEndValue(QRect(targetX, targetY, targetW, targetH));
                        anim->setEasingCurve(QEasingCurve::OutBack);
                        anim->start(QAbstractAnimation::DeleteWhenStopped);
                    }
                }
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
    loadPixmap(ui->ascending1, "Resources/1.png", "1.png");
    loadPixmap(ui->ascending2, "Resources/1.png", "1.png");

    // Load Descending Piles (1 and 2) -> 100.png
    // Note: UI has descending2 based on user info
    loadPixmap(ui->descending1, "Resources/100.png", "100.png");
    loadPixmap(ui->descending2, "Resources/100.png", "100.png"); 
    
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

    // Load Chat Button Default
    loadPixmap(ui->chatButton, "Resources/Button_Chat.png", "Button_Chat.png");

    // Load Card Back
    loadPixmap(ui->cardBack, "Resources/Card_Back.png", "Card_Back.png");
    
    // Install filter helper
    auto setupHandCard = [this, &loadPixmap](QLabel* label, const QString& imgPath) {
        if (label) {
            loadPixmap(label, imgPath, imgPath);
            label->installEventFilter(this);
            label->setCursor(Qt::PointingHandCursor);
        }
    };

    // Load Player Hand (Test Images)
    setupHandCard(ui->hand1, "Resources/2.png");
    setupHandCard(ui->hand2, "Resources/3.png");
    setupHandCard(ui->hand3, "Resources/4.png");
    setupHandCard(ui->hand4, "Resources/5.png");
    setupHandCard(ui->hand5, "Resources/6.png");
    setupHandCard(ui->hand6, "Resources/7.png");

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
    int x = 140; 
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

void GameWindow::sendMessage()
{
    if (!ui->chatInput || !ui->chatHistory) return;

    QString text = ui->chatInput->text().trimmed();
    if (text.isEmpty()) return;

    // Create a custom item
    // For now, just text. We can add icon later if we want custom widget items
    QListWidgetItem* item = new QListWidgetItem(text);
    
    // TODO: Add user name/avatar once we have that info available here
    // For now just format: "Me: Message"
    item->setText("Me: " + text);
    
    // Add placeholder avatar
    QIcon avatar("Resources/2.png");
    item->setIcon(avatar);
    
    ui->chatHistory->addItem(item);
    ui->chatInput->clear();
    
    // Scroll to bottom
    ui->chatHistory->scrollToBottom();
}