#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>

class HelpDialog : public QWidget
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget* parent = nullptr);
    ~HelpDialog() = default;

    void showOverlay();
    void hideOverlay();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void setupStyle();
    QString loadGameRules();
    void finishLoadingContent();

    QWidget* m_contentContainer;
    QTextBrowser* m_rulesText;
    static QString s_cachedRules;
};