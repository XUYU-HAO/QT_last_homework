#ifndef TCPFILESERVERANDSENDER_H
#define TCPFILESERVERANDSENDER_H

#include <QWidget>
#include <QTcpSocket>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QInputDialog>

class TcpFileSender;
class TcpFileServer;

class TcpFileServerandSender : public QWidget
{
    Q_OBJECT

public:
    TcpFileServerandSender(QWidget *parent = nullptr);

private slots:
    void startTeacherMode();
    void startStudentMode();
    void switchToFullScreen(const QString &courseName);

private:
    TcpFileSender *sender;
    TcpFileServer *receiver;
};

#endif // TCPFILESERVERANDSENDER_H
