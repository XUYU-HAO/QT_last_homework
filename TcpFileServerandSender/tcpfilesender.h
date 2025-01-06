#ifndef TCPFILESENDER_H
#define TCPFILESENDER_H

#include <QDialog>
#include <QTcpSocket>
#include <QFile>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFormLayout>

class TcpFileSender : public QDialog
{
    Q_OBJECT

public:
    explicit TcpFileSender(QWidget *parent = nullptr);
    ~TcpFileSender();

    // 新增用於傳送學生答案的函式
    void sendStudentAnswer(const QString &answer);
    QTcpSocket* getTcpClient(); // 公有方法，返回 tcpClient 指針

private slots:
    void start();

private:
    QTcpSocket tcpClient;                // TCP 客戶端
    QLabel *statusLabel;                 // 狀態標籤
    QLabel *usernameLabel;               // 帳號標籤
    QLabel *passwordLabel;               // 密碼標籤
    QLabel *ipLabel;                     // IP 標籤
    QLabel *portLabel;                   // Port 標籤
    QLineEdit *usernameLineEdit;         // 帳號輸入框
    QLineEdit *passwordLineEdit;         // 密碼輸入框
    QLineEdit *ipLineEdit;               // IP 輸入框
    QLineEdit *portLineEdit;             // Port 輸入框
    QPushButton *startButton;            // 開始按鈕
    QPushButton *quitButton;             // 退出按鈕
};

#endif // TCPFILESENDER_H
