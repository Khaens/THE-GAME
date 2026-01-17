#include "PeasantDialog.h"

PeasantDialog::PeasantDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Peasant");
    setFixedSize(300, 150);
    setModal(true);
    
    // Remove window frame for cleaner look
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // Main layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);
    
    // Style the dialog background
    setStyleSheet(R"(
        PeasantDialog {
            background-color: rgba(40, 30, 20, 0.95);
            border: 2px solid #8B4513;
            border-radius: 10px;
        }
    )");
    
    // Message label
    QLabel* messageLabel = new QLabel("You're a peasant...\nwhat did you expect?", this);
    messageLabel->setStyleSheet(R"(
        QLabel {
            color: #f3d05a;
            font-size: 18px;
            font-weight: bold;
            font-family: 'Jersey 15';
        }
    )");
    messageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(messageLabel);
    
    // Button
    QPushButton* okButton = new QPushButton("oh :(", this);
    okButton->setStyleSheet(R"(
        QPushButton {
            border-image: url(Resources/Button.png);
            font-family: 'Jersey 15';
            font-size: 16px;
            color: white;
            font-weight: bold;
            padding: 10px 20px;
            min-width: 80px;
        }
        QPushButton:pressed {
            border-image: url(Resources/Button_Pressed.png);
        }
    )");
    okButton->setCursor(Qt::PointingHandCursor);
    layout->addWidget(okButton, 0, Qt::AlignCenter);
    
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
}
