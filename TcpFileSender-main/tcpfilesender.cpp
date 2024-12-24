#include "tcpfilesender.h"
#include <QIntValidator>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDataStream>
#include <QHostAddress>
#include <QDebug>
#include <QIODevice>

TcpFileSender::TcpFileSender(QWidget *parent)
    : QDialog(parent),
    totalBytes(0), bytesWritten(0), bytesToWrite(0)
{
    // 初始化客户端 UI

    // 增加帳號和密碼的Label和LineEdit
    usernameLabel = new QLabel(QStringLiteral("帳號:"));
    passwordLabel = new QLabel(QStringLiteral("密碼:"));
    usernameLineEdit = new QLineEdit("");
    passwordLineEdit = new QLineEdit("");
    passwordLineEdit->setEchoMode(QLineEdit::Password);

    ipLabel = new QLabel(QStringLiteral("  IP  :"));
    portLabel = new QLabel(QStringLiteral("Port:"));
    ipLineEdit = new QLineEdit("");
    portLineEdit = new QLineEdit("");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    startButton = new QPushButton(QStringLiteral("開始"));
    quitButton = new QPushButton(QStringLiteral("退出"));

    startButton->setEnabled(false);  // 默認禁用

    // 設置帳號和密碼欄位的布局
    QHBoxLayout *usernameLayout = new QHBoxLayout;
    usernameLayout->addWidget(usernameLabel);
    usernameLayout->addWidget(usernameLineEdit);

    QHBoxLayout *passwordLayout = new QHBoxLayout;
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(passwordLineEdit);

    // 設置IP和Port欄位的布局
    QHBoxLayout *ipLayout = new QHBoxLayout;
    ipLayout->addWidget(ipLabel);
    ipLayout->addWidget(ipLineEdit);

    QHBoxLayout *portLayout = new QHBoxLayout;
    portLayout->addWidget(portLabel);
    portLayout->addWidget(portLineEdit);

    // 使用 QHBoxLayout 使按鈕水平排列
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *clientLayout = new QVBoxLayout;

    // 添加帳號密碼布局
    clientLayout->addLayout(usernameLayout);
    clientLayout->addLayout(passwordLayout);

    // 添加IP和Port布局
    clientLayout->addLayout(ipLayout);
    clientLayout->addLayout(portLayout);

    clientLayout->addStretch(1); // 彈性空間
    clientLayout->addLayout(buttonLayout); // 將按鈕佈局加到主佈局

    QWidget *clientTabWidget = new QWidget;
    clientTabWidget->setLayout(clientLayout);

    tabWidget = new QTabWidget;
    tabWidget->addTab(clientTabWidget, QStringLiteral("學生登入"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    setWindowTitle(QStringLiteral("檔案傳送程式"));

    // 訊號與槽的連接
    connect(startButton, &QPushButton::clicked, this, &TcpFileSender::start);
    connect(&tcpClient, &QTcpSocket::connected, this, &TcpFileSender::startTransfer);
    connect(quitButton, &QPushButton::clicked, this, &TcpFileSender::close);

    // 連接信號與槽，當文本改變時檢查按鈕是否應啟用
    connect(ipLineEdit, &QLineEdit::textChanged, this, &TcpFileSender::enableStartButton);
    connect(portLineEdit, &QLineEdit::textChanged, this, &TcpFileSender::enableStartButton);
    connect(usernameLineEdit, &QLineEdit::textChanged, this, &TcpFileSender::enableStartButton);
    connect(passwordLineEdit, &QLineEdit::textChanged, this, &TcpFileSender::enableStartButton);
}

TcpFileSender::~TcpFileSender()
{
    if (localFile) {
        localFile->close();
        delete localFile;
    }
}

void TcpFileSender::start()
{
    startButton->setEnabled(false);
    bytesWritten = 0;

    QString ip = ipLineEdit->text();
    quint16 port = portLineEdit->text().toUShort();
    tcpClient.connectToHost(QHostAddress(ip), port);
}

void TcpFileSender::startTransfer()
{
    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, QStringLiteral("應用程式"),
                             QStringLiteral("無法讀取 %1:\n%2.")
                                 .arg(fileName)
                                 .arg(localFile->errorString()));
        return;
    }

    totalBytes = localFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_6);

    // 获取帳號和密碼
    QString username = usernameLineEdit->text();
    QString password = passwordLineEdit->text();

    // 将帳號和密碼附加到檔案資訊之前
    sendOut << username << password;

    // 傳送檔案名（只包含檔案名稱，不包括路徑）
    QString currentFile = fileName.right(fileName.size() - fileName.lastIndexOf("/") - 1);
    sendOut << qint64(0) << qint64(0) << currentFile;
    totalBytes += outBlock.size();

    sendOut.device()->seek(0);
    sendOut << totalBytes << qint64((outBlock.size() - sizeof(qint64) * 2));

    // 開始發送資料
    bytesToWrite = totalBytes - tcpClient.write(outBlock);
    outBlock.resize(0);
}


void TcpFileSender::updateClientProgress(qint64 numBytes)
{
    bytesWritten += (int)numBytes;
    if (bytesToWrite > 0)
    {
        outBlock = localFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= (int)tcpClient.write(outBlock);
        outBlock.resize(0);
    }
    else
    {
        localFile->close();
    }
}

void TcpFileSender::enableStartButton()
{
    bool isIpValid = !ipLineEdit->text().isEmpty();
    bool isPortValid = !portLineEdit->text().isEmpty();
    bool isUsernameValid = !usernameLineEdit->text().isEmpty();
    bool isPasswordValid = !passwordLineEdit->text().isEmpty();

    startButton->setEnabled(isIpValid && isPortValid && isUsernameValid && isPasswordValid);
}
