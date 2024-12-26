#include "TcpFileServerandSender.h" // 正確引用檔名
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TcpFileServerandSender mainWindow; // 類名與檔名一致
    mainWindow.show();

    return app.exec();
}
