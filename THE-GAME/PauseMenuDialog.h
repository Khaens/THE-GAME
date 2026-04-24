#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class PauseMenuDialog : public QWidget
{
    Q_OBJECT

public:
    explicit PauseMenuDialog(QWidget* parent = nullptr);
    ~PauseMenuDialog() = default;

    void showOverlay();
    void hideOverlay();

signals:
    void resumeRequested();
    void settingsRequested();
    void leaveLobbyRequested();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void setupStyle();

    QWidget* m_contentContainer;
};
