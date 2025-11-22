#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QVector>

struct Resolution {
    QString displayName;
    int width;
    int height;
    bool isFullscreen;
};

class SettingsDialog : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() = default;

    void showOverlay();
    void hideOverlay();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void setupStyle();
    void populateResolutions();
    void updateResolutionDisplay();
    void updateCurrentResolution();

    QWidget* m_contentContainer;
    QLabel* m_resolutionLabel;
    QPushButton* m_leftArrow;
    QPushButton* m_rightArrow;

    QVector<Resolution> m_resolutions;
    int m_currentResolutionIndex;

private slots:
    void onPreviousResolution();
    void onNextResolution();
    void onApplyResolution();
};