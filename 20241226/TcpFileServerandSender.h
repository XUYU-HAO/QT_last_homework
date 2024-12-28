#ifndef TCPFILESERVERANDSENDER_H
#define TCPFILESERVERANDSENDER_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include "tcpfilesender.h"
#include "tcpfileserver.h"

class TcpFileServerandSender : public QWidget
{
    Q_OBJECT

public:
    TcpFileServerandSender(QWidget *parent = nullptr);
    void switchToFullScreen(); // 添加這行

private:
    QPushButton *teacherLoginButton;
    QPushButton *studentLoginButton;

    TcpFileSender *sender;
    TcpFileServer *receiver;
    TcpFileServer *serverDialog;

    void setupUI();
};

#endif // TCPFILESERVERANDSENDER_H
