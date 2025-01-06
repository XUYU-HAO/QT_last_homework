#include "TcpFileServerandSender.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QTextEdit>

TcpFileServerandSender::TcpFileServerandSender(QWidget *parent)
    : QWidget(parent), sender(new TcpFileSender(this)), receiver(new TcpFileServer(this))
{
    // 設置主窗口大小
    setFixedSize(450, 250);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QPushButton *teacherButton = new QPushButton(QStringLiteral("老師端"), this);
    QPushButton *studentButton = new QPushButton(QStringLiteral("學生端"), this);

    // 設置按鈕大小
    teacherButton->setFixedSize(200, 200);
    studentButton->setFixedSize(200, 200);

    // 設置按鈕為左右排列
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(teacherButton);
    buttonLayout->addWidget(studentButton);

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("選擇模式"));

    connect(teacherButton, &QPushButton::clicked, this, &TcpFileServerandSender::startTeacherMode);
    connect(studentButton, &QPushButton::clicked, this, &TcpFileServerandSender::startStudentMode);
}

void TcpFileServerandSender::startTeacherMode()
{
    receiver->show();
    connect(receiver, &TcpFileServer::serverStarted, this, [this]() {
        QString courseName = receiver->getCourseName();

        this->close();

        QWidget *fullScreenWindow = new QWidget();
        QVBoxLayout *mainLayout = new QVBoxLayout(fullScreenWindow);

        // 上方課程名稱
        QLabel *courseNameLabel = new QLabel(courseName, fullScreenWindow);
        courseNameLabel->setAlignment(Qt::AlignCenter);
        courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin: 20px;");
        mainLayout->addWidget(courseNameLabel);

        // 左側學生表格
        QTableWidget *studentTable = new QTableWidget(fullScreenWindow);
        studentTable->setColumnCount(3);
        studentTable->setHorizontalHeaderLabels(QStringList() << "學號" << "分數" << "狀態");
        studentTable->horizontalHeader()->setStretchLastSection(true);
        studentTable->verticalHeader()->setVisible(false);
        studentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        studentTable->setSelectionMode(QAbstractItemView::NoSelection);
        studentTable->setFixedWidth(300);

        // 中間題目輸入區
        QVBoxLayout *questionLayout = new QVBoxLayout();
        QLabel *questionLabel = new QLabel("題目輸入區", fullScreenWindow);
        questionLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
        QTextEdit *questionInput = new QTextEdit(fullScreenWindow);

        questionLayout->addWidget(questionLabel);
        questionLayout->addWidget(questionInput);

        // 選項文字輸入框
        QVBoxLayout *optionsLayout = new QVBoxLayout();
        for (int i = 1; i <= 4; ++i) {
            QLineEdit *optionInput = new QLineEdit(fullScreenWindow);
            optionInput->setPlaceholderText(QStringLiteral("選項%1").arg(i));
            optionsLayout->addWidget(optionInput);
        }
        questionLayout->addLayout(optionsLayout);

        // 底部按鈕
        QHBoxLayout *buttonLayout = new QHBoxLayout();

        // 關閉伺服器按鈕
        QPushButton *closeButton = new QPushButton(QStringLiteral("關閉伺服器"), fullScreenWindow);
        closeButton->setFixedSize(100, 50);
        connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);
        buttonLayout->addWidget(closeButton);

        // 創建題目按鈕
        QPushButton *createQuestionButton = new QPushButton(QStringLiteral("創建題目"), fullScreenWindow);
        createQuestionButton->setFixedSize(100, 50);
        buttonLayout->addWidget(createQuestionButton);

        // 當按下「創建題目」按鈕時，傳送題目和選項給學生端
        connect(createQuestionButton, &QPushButton::clicked, this, [this, questionInput, optionsLayout]() {
            QString questionText = questionInput->toPlainText();
            QStringList options;
            for (int i = 0; i < 4; ++i) {
                QLineEdit *optionInput = qobject_cast<QLineEdit *>(optionsLayout->itemAt(i)->widget());
                if (optionInput) {
                    options << optionInput->text();
                }
            }

            qDebug() << "題目:" << questionText;
            qDebug() << "選項:" << options;

            // 在此處傳送題目和選項給學生端，需擴展功能實現傳輸邏輯
        });

        questionLayout->addLayout(buttonLayout);

        // 將表格與題目布局整合
        QHBoxLayout *contentLayout = new QHBoxLayout();
        contentLayout->addWidget(studentTable);
        contentLayout->addLayout(questionLayout);

        mainLayout->addLayout(contentLayout);

        fullScreenWindow->setLayout(mainLayout);
        fullScreenWindow->setStyleSheet("background-color: white;");
        fullScreenWindow->showFullScreen();
    });
}


void TcpFileServerandSender::startStudentMode()
{
    sender->show();
    connect(sender->getTcpClient(), &QTcpSocket::connected, this, [this]() {
        // 從 TcpFileServer 獲取課程名稱並傳遞給全螢幕視窗
        QString courseName = receiver->getCourseName();

        this->close();

        QWidget *fullScreenWindow = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(fullScreenWindow);

        // 課程名稱標籤
        QLabel *courseNameLabel = new QLabel(courseName, fullScreenWindow);
        courseNameLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin-top: 20px;");
        layout->addWidget(courseNameLabel);

        // 關閉按鈕
        QPushButton *closeButton = new QPushButton(QStringLiteral("關閉"), fullScreenWindow);
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

    // 顯示課程名稱
    QLabel *courseNameLabel = new QLabel(courseName, fullScreenWindow);
    courseNameLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin-top: 20px;");
    layout->addWidget(courseNameLabel);

    // 關閉按鈕
    QPushButton *closeButton = new QPushButton(QStringLiteral("關閉"), fullScreenWindow);
    closeButton->setFixedSize(100, 50);
    layout->addWidget(closeButton);
    layout->setAlignment(closeButton, Qt::AlignHCenter);

    connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);

    fullScreenWindow->setStyleSheet("background-color: white;");
    fullScreenWindow->setLayout(layout);
    fullScreenWindow->showFullScreen();
}
