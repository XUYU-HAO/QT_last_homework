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

private:
    int correctAnswerIndex;
    QString questionText;        // 保存題目
    QStringList optionsText;     // 保存選項
    TcpFileSender *sender;
    TcpFileServer *receiver;
    QVector<QLineEdit*> optionInputs;      // 保存選項輸入框指標
};

#endif // TCPFILESERVERANDSENDER_H
