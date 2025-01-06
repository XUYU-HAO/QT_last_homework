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
    void handleStudentAnswer(const QString &studentId, const QString &answerStatus);

private:
    QString questionText;        // 保存題目
    QStringList optionsText;     // 保存選項
    int correctAnswer;           // 正確答案的索引
    TcpFileSender *sender;
    TcpFileServer *receiver;
    QVector<QLineEdit*> optionInputs;  // 保存選項輸入框指標
    QWidget *fullScreenWindow;       // 保存全屏顯示窗口的指標
};

#endif // TCPFILESERVERANDSENDER_H
