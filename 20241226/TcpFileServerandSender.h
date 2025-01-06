#ifndef TCPFILESERVERANDSENDER_H
#define TCPFILESERVERANDSENDER_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "tcpfilesender.h"
#include "tcpfileserver.h"

class TcpFileServerandSender : public QWidget
{
    Q_OBJECT

public:
    explicit TcpFileServerandSender(QWidget *parent = nullptr);

private slots:
    void startTeacherMode(); // 老師端模式
    void startStudentMode(); // 學生端模式
    void switchToFullScreen(); // 切換到全螢幕

private:
    TcpFileSender *sender;
    TcpFileServer *receiver;
};

#endif // TCPFILESERVERANDSENDER_H
