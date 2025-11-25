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
    m_contentContainer->setFixedSize(800, 600);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(15);
    containerLayout->setContentsMargins(20, 20, 20, 20);

    QLabel* titleLabel = new QLabel("HELP - THE GAME", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #f3d05a; margin-bottom: 5px;");
    containerLayout->addWidget(titleLabel);

    m_rulesText = new QTextBrowser(m_contentContainer);
    m_rulesText->setOpenExternalLinks(false);

    m_rulesText->setStyleSheet(R"(
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
    containerLayout->addWidget(m_rulesText);

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