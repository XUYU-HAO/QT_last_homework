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
#include <QTabWidget>

class TcpFileSender : public QDialog
{
    Q_OBJECT

public:
    explicit TcpFileSender(QWidget *parent = nullptr);
    ~TcpFileSender();

    void setConnectionDetails(const QString &ip, quint16 port); // 設定連線細節

    QTcpSocket tcpClient; // TCP 客戶端
    QTcpSocket *getTcpClient(); // 宣告此方法

private slots:
    void start();                // 開始連線
    void startTransfer();        // 開始傳輸
    void updateClientProgress(qint64 numBytes); // 更新進度條
    void enableStartButton();    // 檢查並啟用「開始」按鈕
    void sendAnswer(); // 發送答案

private:
    QFile *localFile;            // 本地檔案指標
    QProgressBar *clientProgressBar; // 傳輸進度條
    QLabel *clientStatusLabel;   // 狀態標籤
    QLabel *ipLabel;             // IP 標籤
    QLabel *portLabel;           // Port 標籤
    QLabel *usernameLabel;       // 帳號標籤
    QLabel *passwordLabel;       // 密碼標籤
    QLabel *statusLabel;         // 狀態標籤
    QLineEdit *ipLineEdit;       // IP 輸入框
    QLineEdit *portLineEdit;     // Port 輸入框
    QLineEdit *usernameLineEdit; // 帳號輸入框
    QLineEdit *passwordLineEdit; // 密碼輸入框
    QLineEdit *answerLineEdit;   // 答案輸入框
    QPushButton *startButton;    // 開始按鈕
    QPushButton *quitButton;     // 退出按鈕
    QTabWidget *tabWidget;       // Tab 視窗

    QString fileName;            // 傳輸的檔案名稱
    QByteArray outBlock;         // 資料塊
    qint64 totalBytes;           // 總位元組數
    qint64 bytesWritten;         // 已寫入位元組數
    qint64 bytesToWrite;         // 待寫入位元組數
    qint64 loadSize;             // 每次傳輸的資料大小
};

#endif // TCPFILESENDER_H
