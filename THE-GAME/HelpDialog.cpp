#include "HelpDialog.h"
#include <QResizeEvent>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTimer>

QString HelpDialog::s_cachedRules;

HelpDialog::HelpDialog(QWidget* parent)
    : QWidget(parent)
    , m_contentContainer(nullptr)
    , m_rulesText(nullptr)
{
    setWindowFlags(Qt::Widget);
    setAttribute(Qt::WA_TranslucentBackground);

    setupUI();
    setupStyle();
    hide();

    QTimer::singleShot(0, this, &HelpDialog::finishLoadingContent);
}

void HelpDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_contentContainer = new QWidget(this);
    m_contentContainer->setObjectName("helpContainer");
    m_contentContainer->setFixedSize(1125, 750);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(10);  // Reduced spacing between elements
    containerLayout->setContentsMargins(40, 40, 40, 40);  // More balanced margins

    QLabel* titleLabel = new QLabel("HELP - THE GAME", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        font-size: 50px; 
        font-weight: bold; 
        font-family: "Knight Warrior";
        color: #edc40c; 
        margin-top: 0px;
        margin-bottom: 10px;
    )"
    );
    containerLayout->addWidget(titleLabel);

    m_rulesText = new QTextBrowser(m_contentContainer);
    m_rulesText->setOpenExternalLinks(false);
    m_rulesText->setFixedSize(750, 500);  // Increased width to use more space

    m_rulesText->setStyleSheet(R"(
        QTextBrowser {
            border-image: url(Resources/TextBox_1-2.png);
            padding: 50px;
            color: white;
            font-family: "Jersey 15";
            font-size: 20px;
        }
        QTextBrowser QScrollBar:vertical {
            border: none;
            border-radius: 6px;
            background: #edc40c;
            width: 12px;
            margin: 0px;
        }
        
        QTextBrowser QScrollBar::handle:vertical {
            background: #d1ad08;
            border-radius: 6px;
            min-height: 20px;
        }
        
        QTextBrowser QScrollBar::handle:vertical:hover {
            background: #e0ba09;
        }
    )");
    containerLayout->addWidget(m_rulesText, 0, Qt::AlignCenter);

    containerLayout->addSpacing(10);  // Small fixed space before button

    QPushButton* backButton = new QPushButton("BACK", m_contentContainer);
    backButton->setFixedSize(110, 55);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet(R"(
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

    connect(backButton, &QPushButton::clicked, this, &HelpDialog::hideOverlay);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    containerLayout->addLayout(buttonLayout);

    // Center the container
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
    m_contentContainer->setStyleSheet(R"(
        #helpContainer{
            background-color: transparent;
            border-image: url(Resources/TextBox_1-2.png);
        }
    )"    
    );
}

QString HelpDialog::loadGameRules()
{
    if (!s_cachedRules.isEmpty()) {
        return s_cachedRules;
    }

    QFile file("help.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        s_cachedRules = in.readAll();
        file.close();
        return s_cachedRules;
    }

    qWarning() << "Failed to load help.html from resources";
    return "<html><body><h2 style='color: #8e273b;'>Error: Could not load help content</h2></body></html>";
}

void HelpDialog::finishLoadingContent()
{
    if (m_rulesText && m_rulesText->document()->isEmpty()) {
        m_rulesText->setHtml(loadGameRules());
        m_rulesText->repaint();
    }
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