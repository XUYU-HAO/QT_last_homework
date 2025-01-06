#ifndef TCPFILESERVER_H
#define TCPFILESERVER_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class TcpFileServer : public QDialog
{
    Q_OBJECT

public:
    explicit TcpFileServer(QWidget *parent = nullptr);
    ~TcpFileServer();
    void setMathQuestion(const QString &question); // 設定數學題目
    QString getMathQuestion() const; // 獲取數學題目

signals:
    void serverStarted(); // 新增信號

private slots:
    void start();                        // 啟動伺服器
    void acceptConnection();             // 接受新客戶端連線
    void readClientData();               // 讀取客戶端發送的數據
    void displayError(QAbstractSocket::SocketError socketError); // 處理錯誤
    void receiveAnswer();                // 接收答案

private:
    QTcpServer tcpServer;                // TCP 伺服器
    QList<QTcpSocket*> tcpConnections;   // 儲存所有客戶端連線
    QPushButton *startButton;            // 啟動伺服器按鈕
    QPushButton *returnButton;           // 返回按鈕
    QLineEdit *courseNameLineEdit;       // 課程名稱輸入框
    QLineEdit *ipLineEdit;               // IP 地址輸入框
    QLineEdit *portLineEdit;             // Port 號碼輸入框
    QString mathQuestion;                // 數學題目
};

#endif // TCPFILESERVER_H
