#include "AccountDialog.h"
#include <QResizeEvent>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>

AccountDialog::AccountDialog(QWidget* parent)
    : QWidget(parent)
    , m_contentContainer(nullptr)
    , m_stackedWidget(nullptr)
    , m_isLoggedIn(false)
{
    setWindowFlags(Qt::Widget);
    setAttribute(Qt::WA_TranslucentBackground);

    setupUI();
    setupStyle();
    hide();
}

void AccountDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_contentContainer = new QWidget(this);
    m_contentContainer->setFixedSize(500, 600);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(20);
    containerLayout->setContentsMargins(30, 30, 30, 30);

    QLabel* titleLabel = new QLabel("ACCOUNT", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #f3d05a; margin-bottom: 10px;");
    containerLayout->addWidget(titleLabel);

    // Stacked Widget pentru pagini diferite
    m_stackedWidget = new QStackedWidget(m_contentContainer);

    // === LOGIN PAGE ===
    m_loginPage = new QWidget();
    QVBoxLayout* loginLayout = new QVBoxLayout(m_loginPage);
    loginLayout->setSpacing(15);

    QLabel* loginTitle = new QLabel("Login to Your Account");
    loginTitle->setAlignment(Qt::AlignCenter);
    loginTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #ffffff;");
    loginLayout->addWidget(loginTitle);

    m_loginUsernameInput = new QLineEdit();
    m_loginUsernameInput->setPlaceholderText("Username");
    m_loginUsernameInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            color: #2C3E50;
        }
        QLineEdit:focus {
            border: 2px solid #f3d05a;
        }
    )");
    loginLayout->addWidget(m_loginUsernameInput);

    m_loginEmailInput = new QLineEdit();
    m_loginEmailInput->setPlaceholderText("Email");
    m_loginEmailInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            color: #2C3E50;
        }
        QLineEdit:focus {
            border: 2px solid #f3d05a;
        }
    )");
    loginLayout->addWidget(m_loginEmailInput);

    m_loginButton = new QPushButton("LOGIN");
    m_loginButton->setFixedHeight(45);
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_loginButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #e3b310;
        }
        QPushButton:pressed {
            background-color: #869e22;
        }
    )");
    connect(m_loginButton, &QPushButton::clicked, this, &AccountDialog::onLoginClicked);
    loginLayout->addWidget(m_loginButton);

    loginLayout->addSpacing(10);

    QLabel* registerPrompt = new QLabel("Don't have an account?");
    registerPrompt->setAlignment(Qt::AlignCenter);
    registerPrompt->setStyleSheet("color: #ffffff; font-size: 13px;");
    loginLayout->addWidget(registerPrompt);

    m_goToRegisterButton = new QPushButton("Create Account");
    m_goToRegisterButton->setFixedHeight(40);
    m_goToRegisterButton->setCursor(Qt::PointingHandCursor);
    m_goToRegisterButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #f3d05a;
            border: 2px solid #f3d05a;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(243, 208, 90, 0.1);
        }
    )");
    connect(m_goToRegisterButton, &QPushButton::clicked, this, &AccountDialog::onGoToRegisterClicked);
    loginLayout->addWidget(m_goToRegisterButton);

    loginLayout->addStretch();
    m_stackedWidget->addWidget(m_loginPage);

    // === REGISTER PAGE ===
    m_registerPage = new QWidget();
    QVBoxLayout* registerLayout = new QVBoxLayout(m_registerPage);
    registerLayout->setSpacing(15);

    QLabel* registerTitle = new QLabel("Create New Account");
    registerTitle->setAlignment(Qt::AlignCenter);
    registerTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #ffffff;");
    registerLayout->addWidget(registerTitle);

    m_registerUsernameInput = new QLineEdit();
    m_registerUsernameInput->setPlaceholderText("Username");
    m_registerUsernameInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            color: #2C3E50;
        }
        QLineEdit:focus {
            border: 2px solid #f3d05a;
        }
    )");
    registerLayout->addWidget(m_registerUsernameInput);

    m_registerEmailInput = new QLineEdit();
    m_registerEmailInput->setPlaceholderText("Email");
    m_registerEmailInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            color: #2C3E50;
        }
        QLineEdit:focus {
            border: 2px solid #f3d05a;
        }
    )");
    registerLayout->addWidget(m_registerEmailInput);

    //m_registerPasswordInput = new QLineEdit();
    //m_registerPasswordInput->setPlaceholderText("Password");
    //m_registerPasswordInput->setEchoMode(QLineEdit::Password);
    //m_registerPasswordInput->setStyleSheet(R"(
    //    QLineEdit {
    //        background-color: #deaf11;
    //        border: 2px solid #654b1f;
    //        border-radius: 8px;
    //        padding: 12px;
    //        font-size: 14px;
    //        color: #2C3E50;
    //    }
    //    QLineEdit:focus {
    //      border: 2px solid #f3d05a;
    //    }
    //)");
    //registerLayout->addWidget(m_registerPasswordInput);

    m_registerButton = new QPushButton("REGISTER");
    m_registerButton->setFixedHeight(45);
    m_registerButton->setCursor(Qt::PointingHandCursor);
    m_registerButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #e3b310;
        }
        QPushButton:pressed {
            background-color: #869e22;
        }
    )");
    connect(m_registerButton, &QPushButton::clicked, this, &AccountDialog::onRegisterClicked);
    registerLayout->addWidget(m_registerButton);

    registerLayout->addSpacing(10);

    QLabel* loginPrompt = new QLabel("Already have an account?");
    loginPrompt->setAlignment(Qt::AlignCenter);
    loginPrompt->setStyleSheet("color: #ffffff; font-size: 13px;");
    registerLayout->addWidget(loginPrompt);

    m_goToLoginButton = new QPushButton("Back to Login");
    m_goToLoginButton->setFixedHeight(40);
    m_goToLoginButton->setCursor(Qt::PointingHandCursor);
    m_goToLoginButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #f3d05a;
            border: 2px solid #f3d05a;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(243, 208, 90, 0.1);
        }
    )");
    connect(m_goToLoginButton, &QPushButton::clicked, this, &AccountDialog::onGoToLoginClicked);
    registerLayout->addWidget(m_goToLoginButton);

    registerLayout->addStretch();
    m_stackedWidget->addWidget(m_registerPage);

    // === PROFILE PAGE ===
    m_profilePage = new QWidget();
    QVBoxLayout* profileLayout = new QVBoxLayout(m_profilePage);
    profileLayout->setSpacing(20);
    profileLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // Profile Picture (circle)
    m_profilePicture = new QLabel();
    m_profilePicture->setFixedSize(120, 120);
    m_profilePicture->setStyleSheet(R"(
        QLabel {
            background-color: #deaf11;
            border: 4px solid #f3d05a;
            border-radius: 60px;
        }
    )");
    profileLayout->addWidget(m_profilePicture, 0, Qt::AlignCenter);

    // Username
    m_profileUsername = new QLabel("Username");
    m_profileUsername->setAlignment(Qt::AlignCenter);
    m_profileUsername->setStyleSheet("font-size: 24px; font-weight: bold; color: #f3d05a;");
    profileLayout->addWidget(m_profileUsername);

    // Email
    m_profileEmail = new QLabel("email@example.com");
    m_profileEmail->setAlignment(Qt::AlignCenter);
    m_profileEmail->setStyleSheet("font-size: 14px; color: #ffffff;");
    profileLayout->addWidget(m_profileEmail);

    profileLayout->addSpacing(30);

    // Stats placeholder
    QLabel* statsLabel = new QLabel("Statistics coming soon...");
    statsLabel->setAlignment(Qt::AlignCenter);
    statsLabel->setStyleSheet("font-size: 14px; color: #aaaaaa; font-style: italic;");
    profileLayout->addWidget(statsLabel);

    profileLayout->addStretch();

    // Logout button
    m_logoutButton = new QPushButton("LOGOUT");
    m_logoutButton->setFixedHeight(45);
    m_logoutButton->setCursor(Qt::PointingHandCursor);
    m_logoutButton->setStyleSheet(R"(
        QPushButton {
            background-color: #c0392b;
            color: #ffffff;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #a93226;
        }
        QPushButton:pressed {
            background-color: #922b21;
        }
    )");
    connect(m_logoutButton, &QPushButton::clicked, this, &AccountDialog::onLogoutClicked);
    profileLayout->addWidget(m_logoutButton);

    m_stackedWidget->addWidget(m_profilePage);

    containerLayout->addWidget(m_stackedWidget);

    // Close button
    QPushButton* closeButton = new QPushButton("CLOSE", m_contentContainer);
    closeButton->setFixedSize(140, 45);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #e3b310;
        }
        QPushButton:pressed {
            background-color: #869e22;
        }
    )");
    connect(closeButton, &QPushButton::clicked, this, &AccountDialog::hideOverlay);

    QHBoxLayout* closeLayout = new QHBoxLayout();
    closeLayout->addStretch();
    closeLayout->addWidget(closeButton);
    closeLayout->addStretch();
    containerLayout->addLayout(closeLayout);

    // Centrare container
    QHBoxLayout* centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(m_contentContainer);
    centerLayout->addStretch();

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();
}

void AccountDialog::setupStyle()
{
    setStyleSheet("background-color: rgba(0, 0, 0, 150);");

    m_contentContainer->setStyleSheet(
        "background-color: #8e273b; "
        "border: 3px solid #f3d05a; "
        "border-radius: 15px;"
    );
}

void AccountDialog::showOverlay()
{
    if (m_isLoggedIn) {
        showProfilePage();
    }
    else {
        showLoginPage();
    }

    if (parentWidget()) {
        setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    }
    raise();
    show();
}

void AccountDialog::hideOverlay()
{
    hide();
}

void AccountDialog::showLoginPage()
{
    m_stackedWidget->setCurrentWidget(m_loginPage);
}

void AccountDialog::showRegisterPage()
{
    m_stackedWidget->setCurrentWidget(m_registerPage);
}

void AccountDialog::showProfilePage()
{
    m_profileUsername->setText(m_currentUsername);
    m_profileEmail->setText(m_currentEmail);
    m_stackedWidget->setCurrentWidget(m_profilePage);
}

void AccountDialog::onLoginClicked()
{
    QString username = m_loginUsernameInput->text().trimmed();
    QString email = m_loginEmailInput->text().trimmed();

    if (username.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Login Failed", "Please fill in all fields.");
        return;
    }

	// Verificare in database (simulata)
    m_isLoggedIn = true;
    m_currentUsername = username;
    m_currentEmail = email;

    QMessageBox::information(this, "Login Successful", "Welcome back, " + username + "!");
    showProfilePage();
}

void AccountDialog::onRegisterClicked()
{
    QString username = m_registerUsernameInput->text().trimmed();
    QString email = m_registerEmailInput->text().trimmed();
    //QString password = m_registerPasswordInput->text();

    if (username.isEmpty() || email.isEmpty() /* || password.isEmpty() */ ) {
        QMessageBox::warning(this, "Registration Failed", "Please fill in all fields.");
        return;
    }

    //if (password.length() < 6) {
    //    QMessageBox::warning(this, "Registration Failed", "Password must be at least 6 characters.");
    //    return;
    //}

    // TODO: Salvare în database
    m_isLoggedIn = true;
    m_currentUsername = username;
    m_currentEmail = email;

    QMessageBox::information(this, "Registration Successful", "Account created for " + username + "!");
    showProfilePage();
}

void AccountDialog::onLogoutClicked()
{
    m_isLoggedIn = false;
    m_currentUsername.clear();
    m_currentEmail.clear();

    m_loginUsernameInput->clear();
    m_loginEmailInput->clear();
    m_registerUsernameInput->clear();
    m_registerEmailInput->clear();
    //m_registerPasswordInput->clear();

    QMessageBox::information(this, "Logged Out", "You have been logged out successfully.");
    showLoginPage();
}

void AccountDialog::onGoToRegisterClicked()
{
    showRegisterPage();
}

void AccountDialog::onGoToLoginClicked()
{
    showLoginPage();
}

void AccountDialog::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}