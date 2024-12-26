#include "TcpFileServerandSender.h"
#include <QDir>
#include <QFileInfoList>
#include <QFont>

TcpFileServerandSender::TcpFileServerandSender(QWidget *parent)
    : QWidget(parent), sender(new TcpFileSender(this)), receiver(new TcpFileServer(this))
{
    setupUI();
}

void TcpFileServerandSender::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this); // 使用 QHBoxLayout 使按鈕水平排列

    teacherLoginButton = new QPushButton(QStringLiteral("教師登入"), this);
    studentLoginButton = new QPushButton(QStringLiteral("學生登入"), this);

    // 設定按鈕大小
    QSize buttonSize(200, 100);
    teacherLoginButton->setMinimumSize(buttonSize);
    studentLoginButton->setMinimumSize(buttonSize);

    // 設定按鈕字體大小
    QFont buttonFont = teacherLoginButton->font();
    buttonFont.setPointSize(16); // 設定字體大小為16
    teacherLoginButton->setFont(buttonFont);
    studentLoginButton->setFont(buttonFont);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(teacherLoginButton);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(studentLoginButton);

    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    connect(teacherLoginButton, &QPushButton::clicked, this, [this]() {
        receiver->show();
    });

    connect(studentLoginButton, &QPushButton::clicked, this, [this]() {
        sender->show();
    });

    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("登入介面"));
}
