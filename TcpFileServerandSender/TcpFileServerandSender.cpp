#include "TcpFileServerandSender.h"

TcpFileServerandSender::TcpFileServerandSender(QWidget *parent)
    : QWidget(parent), sender(new TcpFileSender(this)), receiver(new TcpFileServer(this))
{
    // 設置主窗口大小
    setFixedSize(450, 250);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QPushButton *teacherButton = new QPushButton(QStringLiteral("老師端"), this);
    QPushButton *studentButton = new QPushButton(QStringLiteral("學生端"), this);

    // 設置按鈕大小
    teacherButton->setFixedSize(200, 200);
    studentButton->setFixedSize(200, 200);

    // 設置按鈕為左右排列
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(teacherButton);
    buttonLayout->addWidget(studentButton);

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("選擇模式"));

    connect(teacherButton, &QPushButton::clicked, this, &TcpFileServerandSender::startTeacherMode);
    connect(studentButton, &QPushButton::clicked, this, &TcpFileServerandSender::startStudentMode);
}

void TcpFileServerandSender::startTeacherMode()
{
    receiver->show();
    connect(receiver, &TcpFileServer::serverStarted, this, [this]() {
        // 從 TcpFileServer 獲取課程名稱並傳遞給全螢幕視窗
        QString courseName = receiver->getCourseName();

        this->close();

        QWidget *fullScreenWindow = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(fullScreenWindow);

        // 課程名稱標籤
        QLabel *courseNameLabel = new QLabel(courseName, fullScreenWindow);
        courseNameLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin-top: 20px;");
        layout->addWidget(courseNameLabel);

        // 關閉按鈕
        QPushButton *closeButton = new QPushButton(QStringLiteral("關閉"), fullScreenWindow);
        closeButton->setFixedSize(100, 50);
        layout->addWidget(closeButton);
        layout->setAlignment(closeButton, Qt::AlignHCenter);

        connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);

        fullScreenWindow->setStyleSheet("background-color: white;");
        fullScreenWindow->setLayout(layout);
        fullScreenWindow->showFullScreen();
    });
}

void TcpFileServerandSender::startStudentMode()
{
    sender->show();
    connect(sender->getTcpClient(), &QTcpSocket::connected, this, [this]() {
        // 從 TcpFileServer 獲取課程名稱並傳遞給全螢幕視窗
        QString courseName = receiver->getCourseName();

        this->close();

        QWidget *fullScreenWindow = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(fullScreenWindow);

        // 課程名稱標籤
        QLabel *courseNameLabel = new QLabel(courseName, fullScreenWindow);
        courseNameLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin-top: 20px;");
        layout->addWidget(courseNameLabel);

        // 關閉按鈕
        QPushButton *closeButton = new QPushButton(QStringLiteral("關閉"), fullScreenWindow);
        closeButton->setFixedSize(100, 50);
        layout->addWidget(closeButton);
        layout->setAlignment(closeButton, Qt::AlignHCenter);

        connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);

        fullScreenWindow->setStyleSheet("background-color: white;");
        fullScreenWindow->setLayout(layout);
        fullScreenWindow->showFullScreen();
    });
}

void TcpFileServerandSender::switchToFullScreen(const QString &courseName)
{
    // 此函數已不再使用，邏輯移至 startTeacherMode 和 startStudentMode
}
