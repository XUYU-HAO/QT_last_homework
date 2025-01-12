#include "tcpfileserver.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QHostAddress>
#include <QDataStream>
#include <QDebug>
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
    ipLineEdit->setText("127.0.0.1"); // 預設為本地測試IP

    QLabel *portLabel = new QLabel("伺服器端口:");
    portLineEdit = new QLineEdit;
    portLineEdit->setPlaceholderText("請輸入伺服器端口號");
    portLineEdit->setText("17000"); // 預設端口號

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

    // 驗證IP地址和端口號
    if (ipAddress.isEmpty() || QHostAddress(ipAddress).isNull() || port < 1024 || port > 65535) {
        QMessageBox::warning(this, "輸入錯誤", "請輸入有效的 IP 地址和端口號（1024-65535）。");
        startButton->setEnabled(true);
        return;
    }

    // 嘗試啟動伺服器
    if (!tcpServer.listen(QHostAddress(ipAddress), port)) {
        QMessageBox::critical(this, "伺服器錯誤",
                              QString("無法啟動伺服器: %1").arg(tcpServer.errorString()));
        startButton->setEnabled(true);
        return;
    }

    emit serverStarted();

    QString courseName = courseNameLineEdit->text(); // 取得課程名稱
    QMessageBox::information(this, "成功", QString("伺服器已啟動，等待連線中...\n課程名稱: %1").arg(courseName));
    qDebug() << "伺服器啟動成功，IP:" << ipAddress << "Port:" << port << "課程名稱:" << courseName;
}

void TcpFileServer::acceptConnection()
{
    QTcpSocket* clientConnection = tcpServer.nextPendingConnection();
    if (!clientConnection) {
        QMessageBox::warning(this, "錯誤", "無法接受連線！");
        return;
    }

    tcpConnections.append(clientConnection);

    connect(clientConnection, &QTcpSocket::readyRead, this, &TcpFileServer::readClientData);
    connect(clientConnection, &QTcpSocket::disconnected, this, [this, clientConnection]() {
        QString studentId = clientConnection->property("studentId").toString();
        emit studentDisconnected(studentId); // 發送學生斷線信號
        tcpConnections.removeOne(clientConnection);
        clientConnection->deleteLater();
    });

    QMessageBox::information(this, "連線", "新的客戶端已連線！");
}

void TcpFileServer::readClientData()
{
    qDebug() << "開始處理數據...";
    QTcpSocket* clientConnection = qobject_cast<QTcpSocket*>(sender());
    if (!clientConnection) {
        qWarning() << "無法獲取客戶端連接。";
        return;
    }

    QDataStream in(clientConnection);
    in.setVersion(QDataStream::Qt_4_6);

    QString messageType;
    in >> messageType; // 讀取消息類型
    qDebug() << "接收到消息類型：" << messageType;

    if (messageType == "login") {
        QString username;
        QString password;

        in >> username >> password; // 讀取帳號和密碼

        qDebug() << "接收到帳號：" << username;
        qDebug() << "接收到密碼：" << password;

        clientConnection->setProperty("studentId", username); // 設定學號屬性

        // 檢查帳號是否合法
        if (username.left(6) == "412431") { // 判斷帳號是否以指定開頭
            emit studentConnected(username);                      // 發送學生連線信號
            clientConnection->write("success");                   // 回應成功訊息
            qDebug() << "學生登入成功，學號：" << username;
        } else {
            clientConnection->write("failure"); // 回應失敗訊息
            clientConnection->disconnectFromHost();
            qDebug() << "學生登入失敗，學號：" << username;
        }
    } else if (messageType == "answerResult") {
        bool isCorrect;
        in >> isCorrect;

        QString studentId = clientConnection->property("studentId").toString();
        if (studentId.isEmpty()) {
            qWarning() << "未能獲取學生學號，無法更新分數";
            return;
        }

        qDebug() << "收到學生回答結果，學號：" << studentId << " 結果：" << (isCorrect ? "正確" : "錯誤");

        if (isCorrect) {
            studentScores[studentId] += 1; // 答對加 1 分
            emit studentCorrectAnswer(studentId); // 發送正確答案信號
        }

        qDebug() << "學生學號：" << studentId << " 分數已更新：" << studentScores[studentId];

        // 回傳更新後的分數給學生
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_6);

        out << QString("updateScore") << studentScores[studentId];
        clientConnection->write(block);
        clientConnection->flush();

        qDebug() << "分數已回傳給學生，學號：" << studentId;
    } else if (messageType == "updateScore") {
        // 處理分數更新請求
        QString studentId = clientConnection->property("studentId").toString();
        if (studentId.isEmpty()) {
            qWarning() << "未能獲取學生學號，無法處理分數更新請求";
            return;
        }

        int currentScore = studentScores.value(studentId, 0); // 獲取當前分數（默認為 0）
        qDebug() << "收到學生分數更新請求，學號：" << studentId << " 當前分數：" << currentScore;

        // 回傳當前分數給學生
        QByteArray scoreBlock;
        QDataStream scoreStream(&scoreBlock, QIODevice::WriteOnly);
        scoreStream.setVersion(QDataStream::Qt_4_6);

        scoreStream << QString("updateScore") << currentScore;
        clientConnection->write(scoreBlock);
        clientConnection->flush();

        qDebug() << "已回傳分數給學生，學號：" << studentId << " 分數：" << currentScore;
    } else {
        qWarning() << "收到未知的消息類型：" << messageType;
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
QList<QTcpSocket*> TcpFileServer::getClientConnections() const
{
    return tcpConnections; // 返回所有客戶端連線
}
