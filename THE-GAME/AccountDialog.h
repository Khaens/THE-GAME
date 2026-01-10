#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QStackedWidget>
#include <memory>

class NetworkManager;

class AccountDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AccountDialog(QWidget* parent = nullptr);
    ~AccountDialog() = default;

    void showOverlay();
    void hideOverlay();
    bool isUserLoggedIn() const { return m_isLoggedIn; }
    QString getCurrentUsername() const { return m_currentUsername; }
    int getCurrentUserId() const { return m_currentUserId; }
    void setNetworkManager(std::shared_ptr<NetworkManager> networkManager);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void setupStyle();
    void showLoginPage();
    void showRegisterPage();
    void showProfilePage();

    // UI Components
    QWidget* m_contentContainer;
    QStackedWidget* m_stackedWidget;

    // Login Page
    QWidget* m_loginPage;
    QLineEdit* m_loginUsernameInput;
    QLineEdit* m_loginPasswordInput;
    QPushButton* m_loginVisibilityButton;
    QPushButton* m_loginButton;
    QPushButton* m_goToRegisterButton;

    // Register Page
    QWidget* m_registerPage;
    QLineEdit* m_registerUsernameInput;
    QLineEdit* m_registerPasswordInput;
    QPushButton* m_registerVisibilityButton;
    QPushButton* m_registerButton;
    QPushButton* m_goToLoginButton;

    // Profile Page
    QWidget* m_profilePage;
    QLabel* m_profilePicture;
    QLabel* m_profileUsername;
    QPushButton* m_changeProfilePicButton;
    QPushButton* m_logoutButton;

    // Session data
    bool m_isLoggedIn;
    QString m_currentUsername;
    int m_currentUserId;

    // Network
    std::shared_ptr<NetworkManager> m_networkManager;

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onLogoutClicked();
    void onGoToRegisterClicked();
    void onGoToLoginClicked();
    void onToggleLoginPasswordVisibility();
    void onToggleRegisterPasswordVisibility();
    void onChangeProfilePicClicked();
};