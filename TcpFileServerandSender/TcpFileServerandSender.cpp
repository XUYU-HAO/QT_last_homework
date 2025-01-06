#include "TcpFileSender.h"
#include "TcpFileServer.h"
#include "TcpFileServerandSender.h"
#include <QInputDialog>
#include <QComboBox>
#include <QStackedWidget>
#include <QPushButton>

TcpFileServerandSender::TcpFileServerandSender(QWidget *parent)
    : QWidget(parent), sender(new TcpFileSender(this)), receiver(new TcpFileServer(this))
{
    // 设置主窗口大小
    setFixedSize(450, 250);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QPushButton *teacherButton = new QPushButton(QStringLiteral("老师端"), this);
    QPushButton *studentButton = new QPushButton(QStringLiteral("学生端"), this);

    // 设置按钮大小
    teacherButton->setFixedSize(200, 200);
    studentButton->setFixedSize(200, 200);

    // 设置按钮为左右排列
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(teacherButton);
    buttonLayout->addWidget(studentButton);

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("选择模式"));

    connect(teacherButton, &QPushButton::clicked, this, &TcpFileServerandSender::startTeacherMode);
    connect(studentButton, &QPushButton::clicked, this, &TcpFileServerandSender::startStudentMode);
}

void TcpFileServerandSender::startTeacherMode()
{
    receiver->show();
    connect(receiver, &TcpFileServer::serverStarted, this, [this]() {
        // 从 TcpFileServer 获取课程名称并传递给全屏窗口
        QString courseName = receiver->getCourseName();

        this->close();

        QWidget *fullScreenWindow = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(fullScreenWindow);

        // 课程名称标签
        QLabel *courseNameLabel = new QLabel(courseName, fullScreenWindow);
        courseNameLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin-top: 20px;");
        layout->addWidget(courseNameLabel);

        // 输入题数的对话框
        bool ok;
        int questionCount = QInputDialog::getInt(fullScreenWindow, QStringLiteral("输入题数"),
                                                 QStringLiteral("请输入题目数量:"), 1, 1, 100, 1, &ok);
        if (!ok || questionCount <= 0) {
            return;
        }

        // 使用 QStackedWidget 来实现分页
        QStackedWidget *stackedWidget = new QStackedWidget(fullScreenWindow);

        for (int i = 0; i < questionCount; ++i) {
            // 每一页代表一个题目的输入
            QWidget *page = new QWidget();
            QVBoxLayout *pageLayout = new QVBoxLayout(page);

            // 题目输入框
            QLineEdit *questionInput = new QLineEdit(page);
            questionInput->setPlaceholderText(QStringLiteral("请输入第 %1 题的题目").arg(i + 1));
            pageLayout->addWidget(questionInput);

            // 选项输入框
            QVBoxLayout *optionsLayout = new QVBoxLayout();
            QLabel *optionsLabel = new QLabel(QStringLiteral("输入选项 (共四个):"), page);
            pageLayout->addWidget(optionsLabel);
            for (int j = 0; j < 4; ++j) {
                QLineEdit *optionInput = new QLineEdit(page);
                optionInput->setPlaceholderText(QStringLiteral("选项 %1").arg(j + 1));
                optionsLayout->addWidget(optionInput);
            }
            pageLayout->addLayout(optionsLayout);

            // 正确答案选择
            QComboBox *correctAnswerComboBox = new QComboBox(page);
            correctAnswerComboBox->addItem(QStringLiteral("选项 1"));
            correctAnswerComboBox->addItem(QStringLiteral("选项 2"));
            correctAnswerComboBox->addItem(QStringLiteral("选项 3"));
            correctAnswerComboBox->addItem(QStringLiteral("选项 4"));
            correctAnswerComboBox->setPlaceholderText(QStringLiteral("选择正确答案"));
            pageLayout->addWidget(correctAnswerComboBox);

            // 添加页面到 QStackedWidget
            stackedWidget->addWidget(page);
        }

        // 分页控制按钮
        QHBoxLayout *navigationLayout = new QHBoxLayout();
        QPushButton *prevButton = new QPushButton(QStringLiteral("上一题"), fullScreenWindow);
        QPushButton *nextButton = new QPushButton(QStringLiteral("下一题"), fullScreenWindow);
        navigationLayout->addWidget(prevButton);
        navigationLayout->addWidget(nextButton);

        layout->addWidget(stackedWidget);
        layout->addLayout(navigationLayout);

        // 上一题按钮逻辑
        connect(prevButton, &QPushButton::clicked, [stackedWidget]() {
            int currentIndex = stackedWidget->currentIndex();
            if (currentIndex > 0) {
                stackedWidget->setCurrentIndex(currentIndex - 1);
            }
        });

        // 下一题按钮逻辑
        connect(nextButton, &QPushButton::clicked, [stackedWidget, questionCount]() {
            int currentIndex = stackedWidget->currentIndex();
            if (currentIndex < questionCount - 1) {
                stackedWidget->setCurrentIndex(currentIndex + 1);
            }
        });

        // 提交按钮
        QPushButton *submitButton = new QPushButton(QStringLiteral("提交"), fullScreenWindow);
        submitButton->setFixedSize(100, 50);
        layout->addWidget(submitButton);
        layout->setAlignment(submitButton, Qt::AlignHCenter);

        connect(submitButton, &QPushButton::clicked, fullScreenWindow, [this, fullScreenWindow]() {
            // 提交按钮点击后的操作 (例如将题目数据发送给学生)
            // 这里可以加入将题目和选项的数据传送给学生端的代码

            fullScreenWindow->close();  // 关闭全屏窗口
        });

        // 关闭按钮
        QPushButton *closeButton = new QPushButton(QStringLiteral("关闭"), fullScreenWindow);
        closeButton->setFixedSize(100, 50);
        layout->addWidget(closeButton);
        layout->setAlignment(closeButton, Qt::AlignHCenter);

        connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);

        fullScreenWindow->setStyleSheet("background-color: white;");
        fullScreenWindow->setLayout(layout);
        fullScreenWindow->showFullScreen();
    });
}

void TcpFileServerandSender::startStudentMode()
{
    sender->show();
    connect(sender->getTcpClient(), &QTcpSocket::connected, this, [this]() {
        // 从 TcpFileServer 获取课程名称并传递给全屏窗口
        QString courseName = receiver->getCourseName();

        this->close();

        QWidget *fullScreenWindow = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(fullScreenWindow);

        // 课程名称标签
        QLabel *courseNameLabel = new QLabel(courseName, fullScreenWindow);
        courseNameLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin-top: 20px;");
        layout->addWidget(courseNameLabel);

        // 关闭按钮
        QPushButton *closeButton = new QPushButton(QStringLiteral("关闭"), fullScreenWindow);
        closeButton->setFixedSize(100, 50);
        layout->addWidget(closeButton);
        layout->setAlignment(closeButton, Qt::AlignHCenter);

        connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);

        fullScreenWindow->setStyleSheet("background-color: white;");
        fullScreenWindow->setLayout(layout);
        fullScreenWindow->showFullScreen();
    });
}

void TcpFileServerandSender::switchToFullScreen(const QString &courseName)
{
    QWidget *fullScreenWindow = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(fullScreenWindow);

    // 显示课程名称
    QLabel *courseNameLabel = new QLabel(courseName, fullScreenWindow);
    courseNameLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin-top: 20px;");
    layout->addWidget(courseNameLabel);

    // 关闭按钮
    QPushButton *closeButton = new QPushButton(QStringLiteral("关闭"), fullScreenWindow);
    closeButton->setFixedSize(100, 50);
    layout->addWidget(closeButton);
    layout->setAlignment(closeButton, Qt::AlignHCenter);

    connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);

    fullScreenWindow->setStyleSheet("background-color: white;");
    fullScreenWindow->setLayout(layout);
    fullScreenWindow->showFullScreen();
}
