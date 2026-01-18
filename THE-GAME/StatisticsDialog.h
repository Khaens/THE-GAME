#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <memory>

class NetworkManager;

class StatisticsDialog : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsDialog(QWidget* parent = nullptr);
    ~StatisticsDialog() = default;

    void showOverlay();
    void hideOverlay();
    void setNetworkManager(std::shared_ptr<NetworkManager> networkManager);
    void setUserId(int userId);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void setupStyle();
    void loadStatistics();

    QWidget* m_contentContainer;
    QLabel* m_hoursLabel;
    QLabel* m_performanceScoreLabel;

    std::shared_ptr<NetworkManager> m_networkManager;
    int m_userId;
};
