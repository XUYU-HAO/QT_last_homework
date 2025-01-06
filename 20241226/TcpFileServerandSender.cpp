#include "TcpFileServerandSender.h"

TcpFileServerandSender::TcpFileServerandSender(QWidget *parent)
    : QWidget(parent), sender(new TcpFileSender(this)), receiver(new TcpFileServer(this))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QPushButton *teacherButton = new QPushButton(QStringLiteral("老師端"), this);
    QPushButton *studentButton = new QPushButton(QStringLiteral("學生端"), this);

    mainLayout->addWidget(teacherButton);
    mainLayout->addWidget(studentButton);

    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("選擇模式"));

    connect(teacherButton, &QPushButton::clicked, this, &TcpFileServerandSender::startTeacherMode);
    connect(studentButton, &QPushButton::clicked, this, &TcpFileServerandSender::startStudentMode);
}

void TcpFileServerandSender::startTeacherMode()
{
    receiver->show();
    connect(receiver, &TcpFileServer::serverStarted, this, [this]() {
        switchToFullScreen();
    });
}

void TcpFileServerandSender::startStudentMode()
{
    sender->show();
    connect(sender->getTcpClient(), &QTcpSocket::connected, this, [this]() {
        switchToFullScreen();
    });
}

void TcpFileServerandSender::switchToFullScreen()
{
    this->close();

    QWidget *fullScreenWindow = new QWidget();
    fullScreenWindow->setWindowTitle(QStringLiteral("傳輸完成"));
    fullScreenWindow->setStyleSheet("background-color: white;");
    fullScreenWindow->showFullScreen();
}
