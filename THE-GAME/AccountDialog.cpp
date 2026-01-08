#include "AccountDialog.h"
#include "NetworkManager.h"
#include <QResizeEvent>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>

AccountDialog::AccountDialog(QWidget* parent)
    : QWidget(parent)
    , m_contentContainer(nullptr)
    , m_stackedWidget(nullptr)
    , m_isLoggedIn(false)
    , m_currentUserId(-1)
    , m_networkManager(nullptr)
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
    m_contentContainer->setObjectName("accountContainer");
    m_contentContainer->setFixedSize(400, 600);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(20);
    containerLayout->setContentsMargins(30, 30, 30, 30);

    QLabel* titleLabel = new QLabel("ACCOUNT", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        font-size: 45px; 
        font-weight: bold;
        font-family: "Knight Warrior"; 
        color: #f3d05a; 
        margin-top: 20px;
    )"
    );
    containerLayout->addWidget(titleLabel);

    // Stacked Widget pentru pagini diferite
    m_stackedWidget = new QStackedWidget(m_contentContainer);

    // === LOGIN PAGE ===
    m_loginPage = new QWidget();
    QVBoxLayout* loginLayout = new QVBoxLayout(m_loginPage);
    loginLayout->setSpacing(15);

    QLabel* loginTitle = new QLabel("Login to Your Account");
    loginTitle->setAlignment(Qt::AlignCenter);
    loginTitle->setStyleSheet(R"(
        font-size: 22px; 
        font-weight: bold; 
        font-family: "Jersey 15";
        color: #ffffff;
    )"
    );
    loginLayout->addWidget(loginTitle);

    m_loginUsernameInput = new QLineEdit();
    m_loginUsernameInput->setPlaceholderText("Username");
    m_loginUsernameInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 12px;
            font-size: 16px;
            font-family: "Jersey 15";
            color: #2C3E50;
        }
        QLineEdit:focus {
            border: 2px solid #f3d05a;
        }
    )");
    loginLayout->addWidget(m_loginUsernameInput);

    // Container for Password and Eye
    QWidget* passwordContainer = new QWidget();
    passwordContainer->setObjectName("passwordContainer");
    passwordContainer->setStyleSheet(R"(
        #passwordContainer {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
        }
    )");
    
    QHBoxLayout* passwordLayout = new QHBoxLayout(passwordContainer);
    passwordLayout->setContentsMargins(0, 0, 15, 0); // small right margin for the eye
    passwordLayout->setSpacing(0);

    m_loginPasswordInput = new QLineEdit();
    m_loginPasswordInput->setPlaceholderText("Password");
    m_loginPasswordInput->setEchoMode(QLineEdit::Password);
    m_loginPasswordInput->setStyleSheet(R"(
        QLineEdit {
            background-color: transparent;
            border: none;
            padding: 12px;
            font-size: 16px;
            font-family: "Jersey 15";
            color: #2C3E50;
        }
    )");
    
    m_loginVisibilityButton = new QPushButton();
    m_loginVisibilityButton->setFixedSize(24, 24);
    m_loginVisibilityButton->setCursor(Qt::PointingHandCursor);
    m_loginVisibilityButton->setStyleSheet(R"(
        QPushButton {
            border: none;
            background-color: transparent;
            border-image: url(Resources/ClosedEye.png);
        }
    )");
    connect(m_loginVisibilityButton, &QPushButton::clicked, this, &AccountDialog::onToggleLoginPasswordVisibility);

    passwordLayout->addWidget(m_loginPasswordInput);
    passwordLayout->addWidget(m_loginVisibilityButton);
    
    // Enter key triggers login
    connect(m_loginPasswordInput, &QLineEdit::returnPressed, this, &AccountDialog::onLoginClicked);
    connect(m_loginUsernameInput, &QLineEdit::returnPressed, [this]() {
        m_loginPasswordInput->setFocus();
    });
    
    loginLayout->addWidget(passwordContainer);

    m_loginButton = new QPushButton("LOGIN");
    m_loginButton->setFixedHeight(45);
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_loginButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 10px;
            font-family: "Jersey 15";
            font-size: 30px;
            letter-spacing: 1px;
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
    registerPrompt->setStyleSheet(R"(
        color: #ffffff; 
        font-size: 18px;
        font-family: "Jersey 15";
    )"
    );
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
            font-size: 22px;
            font-weight: bold;
            font-family: "Jersey 15";
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
    registerTitle->setStyleSheet(R"(
        font-size: 22px; 
        font-weight: bold; 
        font-family: "Jersey 15";
        color: #ffffff;
    )"
    );
    registerLayout->addWidget(registerTitle);

    m_registerUsernameInput = new QLineEdit();
    m_registerUsernameInput->setPlaceholderText("Username");
    m_registerUsernameInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 12px;
            font-size: 16px;
            font-family: "Jersey 15";
            color: #2C3E50;
        }
        QLineEdit:focus {
            border: 2px solid #f3d05a;
        }
    )");
    registerLayout->addWidget(m_registerUsernameInput);

    // Container for Password and Eye (Register)
    QWidget* regPasswordContainer = new QWidget();
    regPasswordContainer->setObjectName("regPasswordContainer");
    regPasswordContainer->setStyleSheet(R"(
        #regPasswordContainer {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
        }
    )");
    
    QHBoxLayout* regPasswordLayout = new QHBoxLayout(regPasswordContainer);
    regPasswordLayout->setContentsMargins(0, 0, 15, 0); 
    regPasswordLayout->setSpacing(0);

    m_registerPasswordInput = new QLineEdit();
    m_registerPasswordInput->setPlaceholderText("Password");
    m_registerPasswordInput->setEchoMode(QLineEdit::Password);
    m_registerPasswordInput->setStyleSheet(R"(
        QLineEdit {
            background-color: transparent;
            border: none;
            padding: 12px;
            font-size: 16px;
            font-family: "Jersey 15";
            color: #2C3E50;
        }
    )");
    
    m_registerVisibilityButton = new QPushButton();
    m_registerVisibilityButton->setFixedSize(24, 24);
    m_registerVisibilityButton->setCursor(Qt::PointingHandCursor);
    m_registerVisibilityButton->setStyleSheet(R"(
        QPushButton {
            border: none;
            background-color: transparent;
            border-image: url(Resources/ClosedEye.png);
        }
    )");
    connect(m_registerVisibilityButton, &QPushButton::clicked, this, &AccountDialog::onToggleRegisterPasswordVisibility);

    regPasswordLayout->addWidget(m_registerPasswordInput);
    regPasswordLayout->addWidget(m_registerVisibilityButton);

    registerLayout->addWidget(regPasswordContainer);

    // Enter key triggers register
    connect(m_registerPasswordInput, &QLineEdit::returnPressed, this, &AccountDialog::onRegisterClicked);
    connect(m_registerUsernameInput, &QLineEdit::returnPressed, [this]() {
        m_registerPasswordInput->setFocus();
    });

    m_registerButton = new QPushButton("REGISTER");
    m_registerButton->setFixedHeight(45);
    m_registerButton->setCursor(Qt::PointingHandCursor);
    m_registerButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 10px;
            font-family: "Jersey 15";
            font-size: 30px;
            letter-spacing: 1px;
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
    loginPrompt->setStyleSheet(R"(
        color: #ffffff; 
        font-size: 18px;
        font-family: "Jersey 15";
    )"
    );
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
            font-size: 22px;
            font-weight: bold;
            font-family: "Jersey 15";
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
    m_profileUsername->setStyleSheet(R"(
        font-size: 40px; 
        font-weight: bold;
        font-family: "Jersey 15"; 
        color: #f3d05a;
    )"
    );
    profileLayout->addWidget(m_profileUsername);

    // User ID label
    QLabel* userIdLabel = new QLabel("User ID: -");
    userIdLabel->setObjectName("userIdLabel");
    userIdLabel->setAlignment(Qt::AlignCenter);
    userIdLabel->setStyleSheet(R"(
        font-size: 18px;
        font-family: "Jersey 15";
        color: #ffffff;
    )"
    );
    profileLayout->addWidget(userIdLabel);

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
            font-size: 30px;
            font-weight: bold;
            font-family: "Knight Warrior";
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
    m_contentContainer->setStyleSheet(R"(
        #accountContainer{
            background-color: transparent;
            border-image: url(Resources/TextBox_2-1.png);
        }
    )"
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
    QLabel* userIdLabel = m_profilePage->findChild<QLabel*>("userIdLabel");
    if (userIdLabel) {
        userIdLabel->setText(QString("User ID: %1").arg(m_currentUserId));
    }
    m_stackedWidget->setCurrentWidget(m_profilePage);
}

void AccountDialog::setNetworkManager(std::shared_ptr<NetworkManager> networkManager)
{
    m_networkManager = networkManager;
}

void AccountDialog::onLoginClicked()
{
    QString username = m_loginUsernameInput->text().trimmed();
    QString password = m_loginPasswordInput->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login Failed", "Please fill in all fields.");
        return;
    }

    if (!m_networkManager) {
        QMessageBox::critical(this, "Error", "Network manager not initialized.");
        return;
    }

    LoginResponse response = m_networkManager->loginUser(
        username.toStdString(),
        password.toStdString()
    );

    if (response.success) {
        m_isLoggedIn = true;
        m_currentUsername = QString::fromStdString(response.username);
        m_currentUserId = response.user_id;

        QMessageBox::information(this, "Login Successful", "Welcome back, " + m_currentUsername + "!");
        m_loginPasswordInput->clear();
        showProfilePage();
    } else {
        QMessageBox::warning(this, "Login Failed", QString::fromStdString(response.error));
    }
}

void AccountDialog::onRegisterClicked()
{
    QString username = m_registerUsernameInput->text().trimmed();
    QString password = m_registerPasswordInput->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Registration Failed", "Please fill in all fields.");
        return;
    }

    if (password.length() < 4) {
        QMessageBox::warning(this, "Registration Failed", "Password must be at least 4 characters.");
        return;
    }

    if (!m_networkManager) {
        QMessageBox::critical(this, "Error", "Network manager not initialized.");
        return;
    }

    RegisterResponse response = m_networkManager->registerUser(
        username.toStdString(),
        password.toStdString()
    );

    if (response.success) {
        m_isLoggedIn = true;
        m_currentUsername = username;
        m_currentUserId = response.user_id;

        QMessageBox::information(this, "Registration Successful", "Account created for " + username + "!");
        m_registerPasswordInput->clear();
        showProfilePage();
    } else {
        QMessageBox::warning(this, "Registration Failed", QString::fromStdString(response.error));
    }
}

void AccountDialog::onLogoutClicked()
{
    m_isLoggedIn = false;
    m_currentUsername.clear();
    m_currentUserId = -1;

    m_loginUsernameInput->clear();
    m_loginPasswordInput->clear();
    m_registerUsernameInput->clear();
    m_registerPasswordInput->clear();

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

void AccountDialog::onToggleLoginPasswordVisibility()
{
    if (m_loginPasswordInput->echoMode() == QLineEdit::Password) {
        m_loginPasswordInput->setEchoMode(QLineEdit::Normal);
        m_loginVisibilityButton->setStyleSheet(R"(
            QPushButton {
                border: none;
                background-color: transparent;
                border-image: url(Resources/OpenEye.png);
            }
        )");
    } else {
        m_loginPasswordInput->setEchoMode(QLineEdit::Password);
        m_loginVisibilityButton->setStyleSheet(R"(
            QPushButton {
                border: none;
                background-color: transparent;
                border-image: url(Resources/ClosedEye.png);
            }
        )");
    }
}

void AccountDialog::onToggleRegisterPasswordVisibility()
{
    if (m_registerPasswordInput->echoMode() == QLineEdit::Password) {
        m_registerPasswordInput->setEchoMode(QLineEdit::Normal);
        m_registerVisibilityButton->setStyleSheet(R"(
            QPushButton {
                border: none;
                background-color: transparent;
                border-image: url(Resources/OpenEye.png);
            }
        )");
    } else {
        m_registerPasswordInput->setEchoMode(QLineEdit::Password);
        m_registerVisibilityButton->setStyleSheet(R"(
            QPushButton {
                border: none;
                background-color: transparent;
                border-image: url(Resources/ClosedEye.png);
            }
        )");
    }
}

void AccountDialog::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}