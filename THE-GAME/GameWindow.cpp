#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QPainter>

GameWindow::GameWindow(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Form())
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);  // Important: Makes it a child widget
    hide();  // Start hidden

    ui->formBackground->setStyleSheet("background-color: transparent;");
    
    loadGameImage();
}

void GameWindow::showOverlay()
{
    QWidget* parent = parentWidget();
    if (parent) {
        setGeometry(0, 0, parent->width(), parent->height());
    }
    raise();  // Bring to front
    show();
}

void GameWindow::hideOverlay()
{
    hide();
}

void GameWindow::resizeEvent(QResizeEvent* event)
{
    if (ui->formBackground) {
        ui->formBackground->setGeometry(this->rect());
    }
}

void GameWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    if (!m_backgroundPixmap.isNull()) {
        // Draw the background filling the entire window
        painter.drawPixmap(rect(), m_backgroundPixmap);
    }
    else {
        // Fallback color
        painter.fillRect(rect(), QColor("#0d0a47"));
    }

    QWidget::paintEvent(event);
}

void GameWindow::loadGameImage()
{
    // Load background image ONCE and cache it
    m_backgroundPixmap = QPixmap("Resources/FullTableMC.png");

    if (m_backgroundPixmap.isNull()) {
        qWarning() << "Failed to load FullTableMC.png - using fallback color";
        // Fallback to solid color
        m_backgroundPixmap = QPixmap(1, 1);
        m_backgroundPixmap.fill(QColor("#0d0a47"));
    }
    else {
        qDebug() << "Background loaded successfully:" << m_backgroundPixmap.size();
    }
}

void GameWindow::onBackButtonClicked()
{
    hideOverlay();
    emit backToMenuRequested();
}