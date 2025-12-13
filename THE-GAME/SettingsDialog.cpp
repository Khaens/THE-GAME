#include "SettingsDialog.h"
#include <QScreen>
#include <QApplication>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QResizeEvent>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QWidget(parent)
    , m_contentContainer(nullptr)
    , m_resolutionLabel(nullptr)
    , m_leftArrow(nullptr)
    , m_rightArrow(nullptr)
    , m_currentResolutionIndex(0)
{
    setWindowFlags(Qt::Widget);
    setAttribute(Qt::WA_TranslucentBackground);

    populateResolutions();
    setupUI();
    setupStyle();
    updateCurrentResolution();
    hide(); 
}

void SettingsDialog::setupUI()
{
    // Layout principal
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Container central
    m_contentContainer = new QWidget(this);
    m_contentContainer->setObjectName("settingsContainer");
    m_contentContainer->setFixedSize(600, 300);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(20);
    containerLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    QLabel* titleLabel = new QLabel("SETTINGS", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        font-size: 35px; 
        font-weight: bold; 
        font-family: "Knight Warrior";
        color: #f3d05a; 
        margin-top: 10px;
    )"
    );
    containerLayout->addWidget(titleLabel);

    // Resolution section label
    QLabel* resolutionSectionLabel = new QLabel("Resolution:", m_contentContainer);
    resolutionSectionLabel->setAlignment(Qt::AlignCenter);
    resolutionSectionLabel->setStyleSheet(R"(
        font-size: 22px; 
        font-weight: bold; 
        font-family: "Jersey 15";
        color: #ffffff;
    )"
    );
    containerLayout->addWidget(resolutionSectionLabel);

    // Resolution selector (arrow buttons + label)
    QHBoxLayout* resolutionLayout = new QHBoxLayout();
    resolutionLayout->setSpacing(20);
    resolutionLayout->setAlignment(Qt::AlignCenter);

    // Left arrow button
    m_leftArrow = new QPushButton("<", m_contentContainer);
    m_leftArrow->setFixedSize(50, 50);
    m_leftArrow->setCursor(Qt::PointingHandCursor);
    m_leftArrow->setStyleSheet(R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 25px;
            font-size: 20px;
            font-weight: bold;
        }
        
        QPushButton:hover:enabled {
            background-color: #e3b310;
        }
        
        QPushButton:pressed:enabled {
            background-color: #869e22;
        }
        
        QPushButton:disabled {
            background-color: #808080;
            color: #4a4a4a;
        }
    )");
    connect(m_leftArrow, &QPushButton::clicked, this, &SettingsDialog::onPreviousResolution);

    // Resolution display label
    m_resolutionLabel = new QLabel(m_contentContainer);
    m_resolutionLabel->setAlignment(Qt::AlignCenter);
    m_resolutionLabel->setFixedSize(280, 50);
    m_resolutionLabel->setStyleSheet(R"(
        QLabel {
            background-color: #deaf11;
            color: #2C3E50;
            border: 2px solid #654b1f;
            border-radius: 10px;
            padding: 10px;
            font-size: 25px;
            font-weight: bold; 
            font-family: "Jersey 15";
        }
    )");

    // Right arrow button
    m_rightArrow = new QPushButton(">", m_contentContainer);
    m_rightArrow->setFixedSize(50, 50);
    m_rightArrow->setCursor(Qt::PointingHandCursor);
    m_rightArrow->setStyleSheet(R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 25px;
            font-size: 20px;
            font-weight: bold;
        }
        
        QPushButton:hover:enabled {
            background-color: #e3b310;
        }
        
        QPushButton:pressed:enabled {
            background-color: #869e22;
        }
        
        QPushButton:disabled {
            background-color: #808080;
            color: #4a4a4a;
        }
    )");
    connect(m_rightArrow, &QPushButton::clicked, this, &SettingsDialog::onNextResolution);

    resolutionLayout->addWidget(m_leftArrow);
    resolutionLayout->addWidget(m_resolutionLabel);
    resolutionLayout->addWidget(m_rightArrow);

    containerLayout->addLayout(resolutionLayout);

    // Spacer
    containerLayout->addStretch();

    // Buttons layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    QPushButton* applyButton = new QPushButton("APPLY", m_contentContainer);
    applyButton->setFixedSize(110, 55);
    applyButton->setCursor(Qt::PointingHandCursor);
    applyButton->setStyleSheet(R"(
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
    connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyResolution);

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
    connect(closeButton, &QPushButton::clicked, this, &SettingsDialog::hideOverlay);

    buttonLayout->addStretch();
    buttonLayout->addWidget(applyButton);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    containerLayout->addLayout(buttonLayout);

    QHBoxLayout* centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(m_contentContainer);
    centerLayout->addStretch();

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();

    // Update initial display
    updateResolutionDisplay();
}

void SettingsDialog::setupStyle()
{
    //setStyleSheet("background-color: rgba(0, 0, 0, 150);");

    m_contentContainer->setStyleSheet(R"(
        #settingsContainer{
            background-color: transparent;
            border-image: url(Resources/TextBox_1-2_Small.png); 
        }
    )"
    );
}

void SettingsDialog::populateResolutions()
{
    QScreen* screen = QApplication::primaryScreen();
    QSize screenSize = screen->geometry().size();

    m_resolutions.clear();

    // Add standard resolutions
    m_resolutions.append({ "1024 x 768", 1024, 768, false });
    m_resolutions.append({ "1280 x 720", 1280, 720, false });
    m_resolutions.append({ "1366 x 768", 1366, 768, false });

    // Add fullscreen option
    m_resolutions.append({
        QString("Fullscreen (%1 x %2)").arg(screenSize.width()).arg(screenSize.height()),
        screenSize.width(),
        screenSize.height(),
        true
        });

    // Set fullscreen as default
    m_currentResolutionIndex = m_resolutions.size() - 1;
}

void SettingsDialog::updateResolutionDisplay()
{
    if (m_currentResolutionIndex >= 0 && m_currentResolutionIndex < m_resolutions.size()) {
        m_resolutionLabel->setText(m_resolutions[m_currentResolutionIndex].displayName);
    }

    // Enable/disable arrows based on position
    m_leftArrow->setEnabled(m_currentResolutionIndex > 0);
    m_rightArrow->setEnabled(m_currentResolutionIndex < m_resolutions.size() - 1);
}

void SettingsDialog::updateCurrentResolution()
{
    QMainWindow* mainWindow = qobject_cast<QMainWindow*>(parentWidget());
    if (!mainWindow) return;

    if (mainWindow->isFullScreen()) {
        // Find fullscreen option
        for (int i = 0; i < m_resolutions.size(); ++i) {
            if (m_resolutions[i].isFullscreen) {
                m_currentResolutionIndex = i;
                break;
            }
        }
    }
    else {
        // Find matching resolution
        int currentWidth = mainWindow->width();
        int currentHeight = mainWindow->height();

        for (int i = 0; i < m_resolutions.size(); ++i) {
            if (m_resolutions[i].width == currentWidth &&
                m_resolutions[i].height == currentHeight) {
                m_currentResolutionIndex = i;
                break;
            }
        }
    }

    updateResolutionDisplay();
}

void SettingsDialog::onPreviousResolution()
{
    if (m_currentResolutionIndex > 0) {
        m_currentResolutionIndex--;
        updateResolutionDisplay();
    }
}

void SettingsDialog::onNextResolution()
{
    if (m_currentResolutionIndex < m_resolutions.size() - 1) {
        m_currentResolutionIndex++;
        updateResolutionDisplay();
    }
}

void SettingsDialog::onApplyResolution()
{
    QMainWindow* mainWindow = qobject_cast<QMainWindow*>(parentWidget());
    if (!mainWindow) return;

    const Resolution& selectedRes = m_resolutions[m_currentResolutionIndex];

    if (selectedRes.isFullscreen) {
        mainWindow->showFullScreen();
    }
    else {
        mainWindow->showNormal();
        mainWindow->resize(selectedRes.width, selectedRes.height);
    }

    if (isVisible()) {
        setGeometry(0, 0, mainWindow->width(), mainWindow->height());
    }
}

void SettingsDialog::showOverlay()
{
    updateCurrentResolution(); 
    if (parentWidget()) {
        setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    }
    raise();
    show();
}

void SettingsDialog::hideOverlay()
{
    hide();
}

void SettingsDialog::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}