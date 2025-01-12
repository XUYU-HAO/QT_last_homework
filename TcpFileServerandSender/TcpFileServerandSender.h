#ifndef TCPFILESERVERANDSENDER_H
#define TCPFILESERVERANDSENDER_H

#include <QWidget>
#include <QVector>
#include <QLineEdit>
#include <QStringList>

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
    int studentScore = 0;         // 用來儲存學生的分數
    QLabel *scoreLabel = nullptr; // 用來顯示分數的標籤
    QWidget *waitingWindow; // 用於顯示等待視窗
    int correctAnswerIndex; // 正確答案的索引
    QString questionText;   // 保存題目
    QStringList optionsText; // 保存選項
    TcpFileSender *sender;   // 連接學生端的 TcpFileSender
    TcpFileServer *receiver; // 監聽教師端的 TcpFileServer
    QVector<QLineEdit*> optionInputs; // 保存選項輸入框指標
};

#endif // TCPFILESERVERANDSENDER_H
