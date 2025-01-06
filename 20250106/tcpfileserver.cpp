#include "tcpfileserver.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QHostAddress>
#include <QDataStream>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>

TcpFileServer::TcpFileServer(QWidget *parent)
    : QDialog(parent)
{
    QLabel *courseNameLabel = new QLabel("課程名稱:");
    courseNameLineEdit = new QLineEdit;
    courseNameLineEdit->setPlaceholderText("請輸入課程名稱");

    QLabel *ipLabel = new QLabel("伺服器 IP:");
    ipLineEdit = new QLineEdit;
    ipLineEdit->setPlaceholderText("請輸入伺服器 IP 地址");

    QLabel *portLabel = new QLabel("伺服器端口:");
    portLineEdit = new QLineEdit;
    portLineEdit->setPlaceholderText("請輸入伺服器端口號");
    portLineEdit->setText("16998");

    startButton = new QPushButton("創建伺服器");
    returnButton = new QPushButton("返回");

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(returnButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(courseNameLabel);
    mainLayout->addWidget(courseNameLineEdit);
    mainLayout->addWidget(ipLabel);
    mainLayout->addWidget(ipLineEdit);
    mainLayout->addWidget(portLabel);
    mainLayout->addWidget(portLineEdit);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
    setWindowTitle("TCP 伺服器");

    connect(startButton, &QPushButton::clicked, this, &TcpFileServer::start);
    connect(returnButton, &QPushButton::clicked, this, &TcpFileServer::close);
    connect(&tcpServer, &QTcpServer::newConnection, this, &TcpFileServer::acceptConnection);
    connect(&tcpServer, &QTcpServer::acceptError, this, &TcpFileServer::displayError);
}

TcpFileServer::~TcpFileServer()
{
    qDeleteAll(tcpConnections);
}

void TcpFileServer::start()
{
    startButton->setEnabled(false);

    QString ipAddress = ipLineEdit->text();
    quint16 port = portLineEdit->text().toUInt();

    if (ipAddress.isEmpty() || port == 0) {
        QMessageBox::warning(this, "輸入錯誤", "請輸入有效的 IP 地址和端口號。");
        startButton->setEnabled(true);
        return;
    }

    while (!tcpServer.isListening() && !tcpServer.listen(QHostAddress(ipAddress), port)) {
        QMessageBox::StandardButton ret = QMessageBox::critical(this,
                                                                "伺服器錯誤",
                                                                QString("無法啟動伺服器: %1.").arg(tcpServer.errorString()),
                                                                QMessageBox::Retry | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) {
            startButton->setEnabled(true);
            return;
        }
    }

    bool ok;
    QString question = QInputDialog::getText(this, "輸入數學題目", "請輸入數學題目:", QLineEdit::Normal, "", &ok);
    if (ok && !question.isEmpty()) {
        setMathQuestion(question);
    } else {
        QMessageBox::warning(this, "輸入錯誤", "數學題目不能為空。");
        tcpServer.close();
        startButton->setEnabled(true);
        return;
    }

    emit serverStarted(); // 發送伺服器啟動信號
    QMessageBox::information(this, "成功", "伺服器已啟動，等待連線中...");
}

void TcpFileServer::setMathQuestion(const QString &question)
{
    mathQuestion = question;
}

QString TcpFileServer::getMathQuestion() const
{
    return mathQuestion;
}

void TcpFileServer::acceptConnection()
{
    QTcpSocket* clientConnection = tcpServer.nextPendingConnection();
    tcpConnections.append(clientConnection);

    connect(clientConnection, &QTcpSocket::readyRead, this, &TcpFileServer::readClientData);
    connect(clientConnection, &QTcpSocket::readyRead, this, &TcpFileServer::receiveAnswer); // 連接 readyRead 信號
    connect(clientConnection, &QTcpSocket::errorOccurred, this, &TcpFileServer::displayError);

    // 發送數學題目給客戶端
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << mathQuestion;
    clientConnection->write(block);

    QMessageBox::information(this, "連線", "新的客戶端已連線！");
}

void TcpFileServer::receiveAnswer()
{
    QTcpSocket* clientConnection = qobject_cast<QTcpSocket*>(sender());
    if (!clientConnection) return;

    QDataStream in(clientConnection);
    in.setVersion(QDataStream::Qt_4_6);

    QString answer;
    in.startTransaction();
    in >> answer;

    if (!in.commitTransaction()) {
        return; // 如果數據不完整，則返回等待下一次 readyRead 信號
    }

    QMessageBox::information(this, "收到答案", QString("學生的答案是: %1").arg(answer));
}

void TcpFileServer::readClientData()
{
    QTcpSocket* clientConnection = qobject_cast<QTcpSocket*>(sender());
    if (!clientConnection) return;

    QDataStream in(clientConnection);
    in.setVersion(QDataStream::Qt_4_6);

    QString username;
    QString password;

    in >> username >> password;

    qDebug() << "接收到帳號：" << username;
    qDebug() << "接收到密碼：" << password;

    if (username == "student" && password == "1234") {
        clientConnection->write("success");
    } else {
        clientConnection->write("failure");
    }
}

void TcpFileServer::displayError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    QTcpSocket* clientConnection = qobject_cast<QTcpSocket*>(sender());
    if (clientConnection) {
        QMessageBox::information(this, "網絡錯誤", QString("發生錯誤: %1").arg(clientConnection->errorString()));
        clientConnection->close();
        tcpConnections.removeOne(clientConnection);
        delete clientConnection;
    }
}
