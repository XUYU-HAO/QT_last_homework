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

void TcpFileSender::startTransfer()
{
    // 顯示數學題目和答案輸入框
    QDataStream in(&tcpClient);
    in.setVersion(QDataStream::Qt_4_6);

    QString question;
    in >> question;

    QWidget *mathQuestionWindow = new QWidget();
    mathQuestionWindow->setWindowTitle(QStringLiteral("數學題目"));
    mathQuestionWindow->setStyleSheet("background-color: white;");

    QLabel *questionLabel = new QLabel(question, mathQuestionWindow);
    questionLabel->setAlignment(Qt::AlignCenter);

    answerLineEdit = new QLineEdit(mathQuestionWindow);
    answerLineEdit->setPlaceholderText(QStringLiteral("請輸入答案"));

    QPushButton *submitButton = new QPushButton(QStringLiteral("提交"), mathQuestionWindow);
    connect(submitButton, &QPushButton::clicked, this, &TcpFileSender::sendAnswer);

    QVBoxLayout *layout = new QVBoxLayout(mathQuestionWindow);
    layout->addWidget(questionLabel);
    layout->addWidget(answerLineEdit);
    layout->addWidget(submitButton);

    mathQuestionWindow->setLayout(layout);
    mathQuestionWindow->showFullScreen();

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

void TcpFileSender::sendAnswer()
{
    QString answer = answerLineEdit->text();
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << answer;
    tcpClient.write(block);
    tcpClient.flush(); // 確保數據立即發送
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
