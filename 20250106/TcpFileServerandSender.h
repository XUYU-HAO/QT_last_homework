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
    void setMathQuestion(const QString &question); // 設定數學題目

private slots:
    void startTeacherMode(); // 老師端模式
    void startStudentMode(); // 學生端模式
    void switchToFullScreen(); // 切換到全螢幕
    void showMathQuestion(); // 顯示數學題目

private:
    TcpFileSender *sender;
    TcpFileServer *receiver;
    QString mathQuestion; // 數學題目
};

#endif // TCPFILESERVERANDSENDER_H
