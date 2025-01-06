#ifndef TCPFILESERVERANDSENDER_H
#define TCPFILESERVERANDSENDER_H

#include <QWidget>
#include "TcpFileSender.h"
#include "TcpFileServer.h"

class TcpFileServerandSender : public QWidget
{
    Q_OBJECT

public:
    explicit TcpFileServerandSender(QWidget *parent = nullptr);

private slots:
    void startTeacherMode();
    void startStudentMode();
    void switchToFullScreen(const QString &courseName); // 確保接受 QString 引數
    void handleClientDisconnected(); // 新增槽函數

private:
    TcpFileSender *sender;
    TcpFileServer *receiver;
};

#endif // TCPFILESERVERANDSENDER_H
