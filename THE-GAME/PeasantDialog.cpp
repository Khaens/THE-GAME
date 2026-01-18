#include "PeasantDialog.h"

PeasantDialog::PeasantDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Peasant");
    setFixedSize(300, 150);
    setModal(true);
    
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* container = new QWidget(this);
    container->setObjectName("container");
    container->setStyleSheet(R"(
        #container {
            border-image: url(Resources/TextBox_1-2.png);
            background-color: transparent;
        }
    )");
    
    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(20, 20, 20, 20);
    containerLayout->setSpacing(15);
    
    QLabel* messageLabel = new QLabel("You're a peasant...\nwhat did you expect?", container);
    messageLabel->setStyleSheet(R"(
        QLabel {
            color: #f3d05a;
            font-size: 18px;
            font-weight: bold;
            font-family: 'Jersey 15';
            background: transparent;
        }
    )");
    messageLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(messageLabel);
    
    QPushButton* okButton = new QPushButton("oh :(", container);
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
    containerLayout->addWidget(okButton, 0, Qt::AlignCenter);
    
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    mainLayout->addWidget(container);
}
