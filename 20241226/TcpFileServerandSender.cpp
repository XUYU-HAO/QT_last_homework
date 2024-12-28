#include "TcpFileServerandSender.h"
#include <QScreen>

// 構造函數
TcpFileServerandSender::TcpFileServerandSender(QWidget *parent)
    : QWidget(parent), serverDialog(new TcpFileServer(this))
{
    setupUI();

    // 當伺服器創建完成時，處理全螢幕切換
    connect(serverDialog, &TcpFileServer::serverCreated, this, &TcpFileServerandSender::switchToFullScreen);
}

// 初始化 UI
void TcpFileServerandSender::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    teacherLoginButton = new QPushButton(QStringLiteral("教師登入"), this);
    studentLoginButton = new QPushButton(QStringLiteral("學生登入"), this);

    // 設定按鈕大小
    QSize buttonSize(200, 100);
    teacherLoginButton->setMinimumSize(buttonSize);
    studentLoginButton->setMinimumSize(buttonSize);

    QFont buttonFont = teacherLoginButton->font();
    buttonFont.setPointSize(16);
    teacherLoginButton->setFont(buttonFont);
    studentLoginButton->setFont(buttonFont);

    mainLayout->addWidget(teacherLoginButton);
    mainLayout->addWidget(studentLoginButton);

    // 教師登入按鈕行為
    connect(teacherLoginButton, &QPushButton::clicked, this, [this]() {
        serverDialog->show(); // 顯示創建伺服器小視窗
    });

    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("登入介面"));
}

// 切換至全螢幕模式
void TcpFileServerandSender::switchToFullScreen()
{
    // 關閉當前所有視窗
    this->close();       // 關閉主視窗
    serverDialog->close(); // 關閉伺服器小視窗

    // 創建全螢幕視窗
    QWidget *fullScreenWindow = new QWidget();
    fullScreenWindow->setWindowTitle(QStringLiteral("教師端操作介面"));
    fullScreenWindow->setWindowState(Qt::WindowFullScreen);

    // 顯示全螢幕視窗
    fullScreenWindow->show();
}
