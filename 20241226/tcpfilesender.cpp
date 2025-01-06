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
    // 初始化用戶介面
    usernameLabel = new QLabel(QStringLiteral("帳號:"));
    passwordLabel = new QLabel(QStringLiteral("密碼:"));
    usernameLineEdit = new QLineEdit();
    passwordLineEdit = new QLineEdit();
    passwordLineEdit->setEchoMode(QLineEdit::Password);

    ipLabel = new QLabel(QStringLiteral("IP地址:"));
    portLabel = new QLabel(QStringLiteral("Port號碼:"));
    ipLineEdit = new QLineEdit("127.0.0.1");
    portLineEdit = new QLineEdit("16998");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    startButton = new QPushButton(QStringLiteral("開始"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    startButton->setEnabled(false);

    // 使用 QFormLayout 排列
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(usernameLabel, usernameLineEdit);
    formLayout->addRow(passwordLabel, passwordLineEdit);
    formLayout->addRow(ipLabel, ipLineEdit);
    formLayout->addRow(portLabel, portLineEdit);

    // 底部按鈕布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(quitButton);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(QStringLiteral("學生端"));

    // 信號與槽的連接
    connect(startButton, &QPushButton::clicked, this, &TcpFileSender::start);
    connect(quitButton, &QPushButton::clicked, this, &TcpFileSender::close);
    connect(&tcpClient, &QTcpSocket::connected, this, &TcpFileSender::startTransfer);
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

    QString username = usernameLineEdit->text();
    QString password = passwordLineEdit->text();
    QString ipAddress = ipLineEdit->text();
    quint16 port = portLineEdit->text().toUShort();

    if (username == "student" && password == "1234") {
        tcpClient.connectToHost(QHostAddress(ipAddress), port);
    } else {
        QMessageBox::warning(this, QStringLiteral("登入失敗"), QStringLiteral("帳號或密碼錯誤！"));
        startButton->setEnabled(true);
    }
}

void TcpFileSender::startTransfer()
{
    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("應用程式"),
                             QStringLiteral("無法讀取 %1:\n%2.")
                                 .arg(fileName)
                                 .arg(localFile->errorString()));
        return;
    }

    totalBytes = localFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_6);

    // 傳送帳號和密碼
    QString username = usernameLineEdit->text();
    QString password = passwordLineEdit->text();
    sendOut << username << password;

    // 傳送檔案名稱
    QString currentFile = fileName.right(fileName.size() - fileName.lastIndexOf("/") - 1);
    sendOut << qint64(0) << qint64(0) << currentFile;
    totalBytes += outBlock.size();

    sendOut.device()->seek(0);
    sendOut << totalBytes << qint64((outBlock.size() - sizeof(qint64) * 2));

    // 開始傳送資料
    bytesToWrite = totalBytes - tcpClient.write(outBlock);
    outBlock.resize(0);
}

void TcpFileSender::updateClientProgress(qint64 numBytes)
{
    bytesWritten += (int)numBytes;
    if (bytesToWrite > 0) {
        outBlock = localFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= (int)tcpClient.write(outBlock);
        outBlock.resize(0);
    } else {
        localFile->close();
    }
}

void TcpFileSender::enableStartButton()
{
    bool isUsernameValid = !usernameLineEdit->text().isEmpty();
    bool isPasswordValid = !passwordLineEdit->text().isEmpty();
    bool isIpValid = !ipLineEdit->text().isEmpty();
    bool isPortValid = !portLineEdit->text().isEmpty();

    startButton->setEnabled(isUsernameValid && isPasswordValid && isIpValid && isPortValid);
}
QTcpSocket *TcpFileSender::getTcpClient()
{
    return &tcpClient; // 返回指向 TCP 客戶端的指標
}
