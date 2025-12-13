#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>

GameWindow::GameWindow(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Form())
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);
    hide();

    ui->formBackground->setStyleSheet("background-color: transparent;");
    
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

    // Ensure layout is up-to-date to get correct final positions
    if (ui && ui->bannerLabel && ui->beerLabel) {
        // Force layout update
        ui->formBackground->updateGeometry();
        
        // Save final positions (where layout put them)
        QRect finalBannerRect = ui->bannerLabel->geometry();
        QRect finalBeerRect = ui->beerLabel->geometry();

        // Banner Animation (Top to Bottom)
        QPropertyAnimation* bannerAnim = new QPropertyAnimation(ui->bannerLabel, "geometry");
        bannerAnim->setDuration(2000); 
        bannerAnim->setStartValue(QRect(finalBannerRect.x(), -finalBannerRect.height(), finalBannerRect.width(), finalBannerRect.height()));
        bannerAnim->setEndValue(finalBannerRect);
        bannerAnim->setEasingCurve(QEasingCurve::OutCubic);

        // Beer Meter Animation (Bottom to Top)
        QPropertyAnimation* beerAnim = new QPropertyAnimation(ui->beerLabel, "geometry");
        beerAnim->setDuration(2000);
        beerAnim->setStartValue(QRect(finalBeerRect.x(), height(), finalBeerRect.width(), finalBeerRect.height()));
        beerAnim->setEndValue(finalBeerRect);
        beerAnim->setEasingCurve(QEasingCurve::OutCubic);

        // Group Animation
        QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
        group->addAnimation(bannerAnim);
        group->addAnimation(beerAnim);
        group->start(QAbstractAnimation::DeleteWhenStopped);
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
    QWidget::resizeEvent(event);
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

void GameWindow::loadGameImage()
{
    // Load background image ONCE and cache it
    m_backgroundPixmap = QPixmap("Resources/TableMC.png");

    if (m_backgroundPixmap.isNull()) {
        qWarning() << "Failed to load TableMC.png - using fallback color";
        m_backgroundPixmap = QPixmap(1, 1);
        m_backgroundPixmap.fill(QColor("#0d0a47"));
    }

    // Load banner and beer meter
    QPixmap banner("Resources/BannerTableMC.png");
    if (!banner.isNull()) {
        if (ui && ui->bannerLabel) {
            ui->bannerLabel->setPixmap(banner);
            ui->bannerLabel->setScaledContents(true);
        }
    }
    else {
        qWarning() << "Failed to load BannerTableMC.png";
    }

    QPixmap beer("Resources/BeerMeter1TableMC.png");
    if (!beer.isNull()) {
        if (ui && ui->beerLabel) {
            ui->beerLabel->setPixmap(beer);
            ui->beerLabel->setScaledContents(true);
        }
    }
    else {
        qWarning() << "Failed to load BeerMeter1TableMC.png";
    }
}

void GameWindow::onBackButtonClicked()
{
    hideOverlay();
    emit backToMenuRequested();
}