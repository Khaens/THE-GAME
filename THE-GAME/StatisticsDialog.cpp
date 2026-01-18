#include "StatisticsDialog.h"
#include "NetworkManager.h"
#include <QResizeEvent>
#include <QMessageBox>
#include <QDebug>
#include <QGraphicsDropShadowEffect>

StatisticsDialog::StatisticsDialog(QWidget * parent)
    : QWidget(parent)
    , m_contentContainer(nullptr)
    , m_hoursLabel(nullptr)
    , m_performanceScoreLabel(nullptr)
    , m_userId(-1)
{
    setWindowFlags(Qt::Widget);
    setAttribute(Qt::WA_TranslucentBackground);

    setupUI();
    setupStyle();
    hide();
}

void StatisticsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_contentContainer = new QWidget(this);
    m_contentContainer->setObjectName("statisticsContainer");
    m_contentContainer->setFixedSize(600, 420);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(15);
    containerLayout->setContentsMargins(40, 40, 40, 40);

    // Title
    QLabel* titleLabel = new QLabel("STATISTICS", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        font-size: 48px; 
        font-weight: bold; 
        font-family: "Knight Warrior";
        color: #f3d05a; 
        margin-top: 5px;
        margin-bottom: 5px;
    )");

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setOffset(2, 2);
    shadow->setColor(QColor(0, 0, 0, 150));
    titleLabel->setGraphicsEffect(shadow);

    containerLayout->addWidget(titleLabel);

    containerLayout->addSpacing(10);

    QLabel* hoursTitleLabel = new QLabel("TOTAL PLAYTIME", m_contentContainer);
    hoursTitleLabel->setAlignment(Qt::AlignCenter);
    hoursTitleLabel->setStyleSheet(R"(
        font-size: 20px; 
        font-weight: bold; 
        font-family: "Jersey 15";
        color: #e0d0b0;
    )");
    containerLayout->addWidget(hoursTitleLabel);

    m_hoursLabel = new QLabel("0.0", m_contentContainer);
    m_hoursLabel->setAlignment(Qt::AlignCenter);
    m_hoursLabel->setStyleSheet(R"(
        QLabel {
            background-color: rgba(42, 26, 16, 0.8);
            color: #f3d05a;
            border: 2px solid #8c6a3e;
            border-radius: 8px;
            padding: 5px;
            font-size: 28px;
            font-weight: bold; 
            font-family: "Jersey 15";
            min-height: 40px;
        }
    )");
    containerLayout->addWidget(m_hoursLabel);

    containerLayout->addSpacing(15);

    QLabel* scoreTitleLabel = new QLabel("PERFORMANCE SCORE", m_contentContainer);
    scoreTitleLabel->setAlignment(Qt::AlignCenter);
    scoreTitleLabel->setStyleSheet(R"(
        font-size: 20px; 
        font-weight: bold; 
        font-family: "Jersey 15";
        color: #e0d0b0;
    )");
    containerLayout->addWidget(scoreTitleLabel);

    m_performanceScoreLabel = new QLabel("0.0", m_contentContainer);
    m_performanceScoreLabel->setAlignment(Qt::AlignCenter);
    m_performanceScoreLabel->setStyleSheet(R"(
        QLabel {
            background-color: rgba(42, 26, 16, 0.8);
            color: #f3d05a;
            border: 2px solid #8c6a3e;
            border-radius: 8px;
            padding: 5px;
            font-size: 28px;
            font-weight: bold; 
            font-family: "Jersey 15";
            min-height: 40px;
        }
    )");
    containerLayout->addWidget(m_performanceScoreLabel);

    containerLayout->addStretch();

    // Close button
    QPushButton* closeButton = new QPushButton("CLOSE", m_contentContainer);
    closeButton->setFixedSize(110, 55);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(R"(
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
    connect(closeButton, &QPushButton::clicked, this, &StatisticsDialog::hideOverlay);

    QHBoxLayout* closeLayout = new QHBoxLayout();
    closeLayout->addStretch();
    closeLayout->addWidget(closeButton);
    closeLayout->addStretch();
    containerLayout->addLayout(closeLayout);

    containerLayout->addSpacing(10);

    QHBoxLayout* centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(m_contentContainer);
    centerLayout->addStretch();

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();
}

void StatisticsDialog::setupStyle()
{
    m_contentContainer->setStyleSheet(R"(
        #statisticsContainer{
            background-color: transparent;
            border-image: url(Resources/TextBox_1-2.png); 
        }
    )"
    );
}

void StatisticsDialog::showOverlay()
{
    if (m_userId > 0) {
        loadStatistics();
    }
    if (parentWidget()) {
        setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    }
    raise();
    show();
}

void StatisticsDialog::hideOverlay()
{
    hide();
}

void StatisticsDialog::setNetworkManager(std::shared_ptr<NetworkManager> networkManager)
{
    m_networkManager = networkManager;
}

void StatisticsDialog::setUserId(int userId)
{
    m_userId = userId;
}

void StatisticsDialog::loadStatistics()
{
    if (!m_networkManager || m_userId <= 0) {
        m_hoursLabel->setText("0h 0m 0s");
        m_performanceScoreLabel->setText("0.0");
        return;
    }

    // Fetch playtime
    NetworkManager::PlaytimeData playtimeData = m_networkManager->getPlaytime(m_userId);
    if (playtimeData.success) {
        int totalSeconds = static_cast<int>(playtimeData.hours * 3600);
        int h = totalSeconds / 3600;
        int m = (totalSeconds % 3600) / 60;
        int s = totalSeconds % 60;

        m_hoursLabel->setText(QString("%1h %2m %3s").arg(h).arg(m).arg(s));
    }
    else {
        m_hoursLabel->setText("0h 0m 0s");
        qDebug() << "Failed to load playtime:" << QString::fromStdString(playtimeData.error);
    }

    // Fetch statistics (performance score)
    NetworkManager::StatisticsData statsData = m_networkManager->getStatistics(m_userId);
    if (statsData.success) {
        float clampedScore = qBound(1.0f, statsData.performance_score, 5.0f);
        m_performanceScoreLabel->setText(QString::number(clampedScore, 'f', 2));
    }
    else {
        m_performanceScoreLabel->setText("0.0");
        qDebug() << "Failed to load statistics:" << QString::fromStdString(statsData.error);
    }
}

void StatisticsDialog::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}
