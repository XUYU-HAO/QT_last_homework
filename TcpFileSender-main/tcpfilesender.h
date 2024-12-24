#ifndef TCPFILESENDER_H
#define TCPFILESENDER_H

#include <QDialog>
#include <QTcpSocket>
#include <QFile>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>

class TcpFileSender : public QDialog
{
    Q_OBJECT

public:
    explicit TcpFileSender(QWidget *parent = nullptr);
    ~TcpFileSender();

private slots:
    void start();
    void startTransfer();
    void updateClientProgress(qint64 numBytes);
    void enableStartButton();

private:
    QTcpSocket tcpClient;
    QFile *localFile;
    QProgressBar *clientProgressBar;
    QLabel *clientStatusLabel;
    QLabel *ipLabel;
    QLabel *portLabel;
    QLabel *usernameLabel;
    QLabel *passwordLabel;
    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;
    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    QPushButton *startButton;
    QPushButton *quitButton;
    QTabWidget *tabWidget;

    QString fileName;
    QByteArray outBlock;
    qint64 totalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    qint64 loadSize;
};

#endif // TCPFILESENDER_H
