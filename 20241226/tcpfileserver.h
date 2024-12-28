#ifndef TCPFILESERVER_H
#define TCPFILESERVER_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QLineEdit>

class TcpFileServer : public QDialog
{
    Q_OBJECT

public:
    explicit TcpFileServer(QWidget *parent = nullptr);
    ~TcpFileServer();
signals:
    void serverCreated(); // 添加這行

private slots:
    void start();
    void acceptConnection();
    void readClientData();
    void displayError(QAbstractSocket::SocketError socketError);

private:
    QTcpServer tcpServer;
    QList<QTcpSocket*> tcpConnections;  // 用來儲存所有的客戶端連線
    QPushButton *startButton;
    QPushButton *returnButton;
    QLineEdit *courseNameLineEdit;
    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;
};

#endif // TCPFILESERVER_H
