#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <memory>
#include "NetworkManager.h"

class AchievementsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AchievementsDialog(std::shared_ptr<NetworkManager> networkManager, int userId, QWidget* parent = nullptr);
    ~AchievementsDialog();

private slots:
    void onBackClicked();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void loadAchievements();
    void addAchievementItem(const QString& title, const QString& description, bool unlocked);

    std::shared_ptr<NetworkManager> m_networkManager;
    int m_userId;
    
    // UI
    QWidget* m_background;
    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;
    QVBoxLayout* m_scrollLayout;
    QPushButton* m_backButton;
    QLabel* m_titleLabel;
};
