#ifndef TCPFILESERVER_H
#define TCPFILESERVER_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLineEdit>
#include <QPushButton>

class TcpFileServer : public QDialog
{
    Q_OBJECT

public:
    explicit TcpFileServer(QWidget *parent = nullptr);
    ~TcpFileServer();
    QList<QTcpSocket*> getClientConnections() const; // 獲取所有客戶端連線
    QString getCourseName() const { return courseNameLineEdit->text(); }

signals:

    void studentAnswerReceived(const QString &studentId, const QString &answer);
    void studentConnected(const QString &studentId); // 當學生連線時發送學號
    void studentDisconnected(const QString &studentId); // 當學生斷線時發送學號
    void serverStarted();  // 定義 serverStarted 信號
    void studentCorrectAnswer(const QString &studentId);

private slots:
    void start();
    void acceptConnection();

    void readClientData();
    void displayError(QAbstractSocket::SocketError socketError);

private:
    QMap<QString, int> studentScores; // 儲存學生學號與分數對應的 QMap
    int correctAnswerIndex = -1; // 初始化為 -1 表示未設置
    QTcpServer tcpServer;
    QList<QTcpSocket*> tcpConnections;
    QLineEdit *courseNameLineEdit;
    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;
    QPushButton *startButton;
    QPushButton *returnButton;
};

#endif // TCPFILESERVER_H
