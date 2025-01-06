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

    QString getCourseName() const { return courseNameLineEdit->text(); }

signals:
    void serverStarted();  // 定義 serverStarted 信號

private slots:
    void start();
    void acceptConnection();
    void readClientData();
    void displayError(QAbstractSocket::SocketError socketError);

private:
    QTcpServer tcpServer;
    QList<QTcpSocket*> tcpConnections;
    QLineEdit *courseNameLineEdit;
    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;
    QPushButton *startButton;
    QPushButton *returnButton;
};

#endif // TCPFILESERVER_H
