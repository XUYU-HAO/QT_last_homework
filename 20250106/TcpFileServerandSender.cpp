#include "TcpFileServerandSender.h"
#include <QRandomGenerator>

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
        mathQuestion = receiver->getMathQuestion(); // 使用 getMathQuestion 方法獲取數學題目
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
    showMathQuestion();
}

void TcpFileServerandSender::showMathQuestion()
{
    QWidget *mathQuestionWindow = new QWidget();
    mathQuestionWindow->setWindowTitle(QStringLiteral("數學題目"));
    mathQuestionWindow->setStyleSheet("background-color: white;");

    QLabel *questionLabel = new QLabel(mathQuestion, mathQuestionWindow); // 使用設定的數學題目
    questionLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout(mathQuestionWindow);
    layout->addWidget(questionLabel);

    mathQuestionWindow->setLayout(layout);
    mathQuestionWindow->showFullScreen();
}

void TcpFileServerandSender::setMathQuestion(const QString &question)
{
    mathQuestion = question;
}
