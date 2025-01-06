#include "tcpfilesender.h"
#include <QIntValidator>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDataStream>
#include <QHostAddress>
#include <QDebug>

TcpFileSender::TcpFileSender(QWidget *parent)
    : QDialog(parent)
{
    // 初始化 UI 元件
    usernameLabel = new QLabel(QStringLiteral("帳號:"));
    passwordLabel = new QLabel(QStringLiteral("密碼:"));
    usernameLineEdit = new QLineEdit();
    passwordLineEdit = new QLineEdit();
    passwordLineEdit->setEchoMode(QLineEdit::Password);

    ipLabel = new QLabel(QStringLiteral("伺服器 IP:"));
    portLabel = new QLabel(QStringLiteral("伺服器 Port:"));
    ipLineEdit = new QLineEdit("127.0.0.1");
    portLineEdit = new QLineEdit("16998");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    startButton = new QPushButton(QStringLiteral("連線"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    statusLabel = new QLabel(QStringLiteral("等待操作..."));

    // 布局
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(usernameLabel, usernameLineEdit);
    formLayout->addRow(passwordLabel, passwordLineEdit);
    formLayout->addRow(ipLabel, ipLineEdit);
    formLayout->addRow(portLabel, portLineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("學生端"));

    // 信號與槽連接
    connect(startButton, &QPushButton::clicked, this, &TcpFileSender::start);
    connect(quitButton, &QPushButton::clicked, this, &TcpFileSender::close);
}

TcpFileSender::~TcpFileSender()
{
}
QTcpSocket* TcpFileSender::getTcpClient()
{
    return &tcpClient; // 返回指向 tcpClient 的指針
}

void TcpFileSender::start()
{
    startButton->setEnabled(false); // 禁用按鈕

    QString ipAddress = ipLineEdit->text();  // 取得 IP 位址
    quint16 port = portLineEdit->text().toUShort();  // 取得 Port 號碼
    QString username = usernameLineEdit->text(); // 取得帳號
    QString password = passwordLineEdit->text(); // 取得密碼

    if (ipAddress.isEmpty() || port == 0 || username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("錯誤"), QStringLiteral("請完整填寫所有欄位！"));
        startButton->setEnabled(true); // 恢復按鈕
        return;
    }
    // 發送帳號和密碼
    QByteArray userData;
    QDataStream out(&userData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << username << password;

    tcpClient.connectToHost(ipLineEdit->text(), portLineEdit->text().toUInt());
    connect(&tcpClient, &QTcpSocket::connected, this, [this, userData]() mutable {
        tcpClient.write(userData); // 傳送帳號和密碼
    });
    // 發起 TCP 連線
    tcpClient.connectToHost(ipAddress, port);

    // 更新狀態
    statusLabel->setText(QStringLiteral("正在連線到伺服器..."));

    // 當連線成功後傳送帳號和密碼
    connect(&tcpClient, &QTcpSocket::connected, this, [this, username, password]() {
        statusLabel->setText(QStringLiteral("連線成功，正在傳送資料..."));

        // 將帳號和密碼打包並傳送
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_6);
        out << username << password;

        tcpClient.write(data); // 傳送資料
    });

    // 處理伺服器回應
    connect(&tcpClient, &QTcpSocket::readyRead, this, [this]() {
        QByteArray response = tcpClient.readAll();
        QString result(response);

        if (result == "success") {
            QMessageBox::information(this, QStringLiteral("成功"), QStringLiteral("登入成功！"));
            this->close(); // 關閉登入介面
        } else {
            QMessageBox::warning(this, QStringLiteral("失敗"), QStringLiteral("帳號或密碼錯誤！"));
            startButton->setEnabled(true); // 恢復按鈕
        }
    });

    // 處理連線錯誤
    connect(&tcpClient, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError socketError) {
        Q_UNUSED(socketError)
        QMessageBox::critical(this, QStringLiteral("連線失敗"), tcpClient.errorString());
        startButton->setEnabled(true); // 恢復按鈕
    });
}
