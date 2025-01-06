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

        // 清空之前的選項輸入框
        optionInputs.clear();

        // 選項文字輸入框
        QVBoxLayout *optionsLayout = new QVBoxLayout();
        for (int i = 1; i <= 4; ++i) {
            QLineEdit *optionInput = new QLineEdit(fullScreenWindow);
            optionInput->setPlaceholderText(QStringLiteral("選項%1").arg(i));
            optionsLayout->addWidget(optionInput);

            // 保存到 optionInputs 中
            optionInputs.append(optionInput);
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
        connect(createQuestionButton, &QPushButton::clicked, this, [this, questionInput]() {
            questionText = questionInput->toPlainText();  // 保存題目
            optionsText.clear();

            for (QLineEdit *input : optionInputs) {
                if (input) {
                    optionsText.append(input->text());  // 保存選項
                }
            }

            qDebug() << "題目:" << questionText;
            qDebug() << "選項:" << optionsText;

            // 在此處傳送題目和選項給學生端，需擴展功能實現傳輸邏輯
        });

        questionLayout->addLayout(buttonLayout);

        // 將表格與題目布局整合
        QHBoxLayout *contentLayout = new QHBoxLayout();
        contentLayout->addWidget(studentTable);
        contentLayout->addLayout(questionLayout);

        mainLayout->addLayout(contentLayout);

        // 監聽學生連線信號，更新表格
        connect(receiver, &TcpFileServer::studentConnected, this, [studentTable](const QString &studentId) {
            bool studentExists = false;
            for (int row = 0; row < studentTable->rowCount(); ++row) {
                QTableWidgetItem *idItem = studentTable->item(row, 0);  // 檢查學號欄位
                if (idItem && idItem->text() == studentId) {
                    studentExists = true;  // 學生已經在表格中
                    break;
                }
            }

            // 如果學生不存在於表格中，則插入新一行
            if (!studentExists) {
                int row = studentTable->rowCount();  // 獲取當前表格的行數
                studentTable->insertRow(row);  // 在表格中插入新的一行

                // 插入學生的學號
                QTableWidgetItem *idItem = new QTableWidgetItem(studentId);
                idItem->setTextAlignment(Qt::AlignCenter);  // 設置學號居中
                studentTable->setItem(row, 0, idItem);

                // 插入學生的分數，預設為 0
                QTableWidgetItem *scoreItem = new QTableWidgetItem("0");
                scoreItem->setTextAlignment(Qt::AlignCenter);  // 設置分數居中
                studentTable->setItem(row, 1, scoreItem);

                // 插入學生的狀態，預設為 "在線"
                QTableWidgetItem *statusItem = new QTableWidgetItem("在線");
                statusItem->setTextAlignment(Qt::AlignCenter);  // 設置狀態居中
                studentTable->setItem(row, 2, statusItem);
            }
        });

        // 監聽學生斷線信號，更新表格
        connect(receiver, &TcpFileServer::studentDisconnected, this, [studentTable](const QString &studentId) {
            for (int row = 0; row < studentTable->rowCount(); ++row) {
                QTableWidgetItem *idItem = studentTable->item(row, 0);  // 檢查學號欄位
                if (idItem && idItem->text() == studentId) {
                    QTableWidgetItem *statusItem = studentTable->item(row, 2);  // 設置狀態欄位
                    if (statusItem) {
                        statusItem->setText("離線");  // 更新學生狀態為 "離線"
                    }
                    break;
                }
            }
        });

        fullScreenWindow->setLayout(mainLayout);
        fullScreenWindow->setStyleSheet("background-color: white;");
        fullScreenWindow->showFullScreen();
    });
}
void TcpFileServerandSender::startStudentMode()
{
    sender->show();
    connect(sender->getTcpClient(), &QTcpSocket::connected, this, [this]() {
        QString courseName = receiver->getCourseName();

        this->close();

        QWidget *fullScreenWindow = new QWidget();
        QVBoxLayout *mainLayout = new QVBoxLayout(fullScreenWindow);

        // 課程名稱標籤
        QLabel *courseNameLabel = new QLabel(courseName, fullScreenWindow);
        courseNameLabel->setAlignment(Qt::AlignCenter);
        courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin-top: 20px;");
        mainLayout->addWidget(courseNameLabel);

        // 顯示題目
        QLabel *questionLabel = new QLabel(questionText, fullScreenWindow);
        questionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        questionLabel->setStyleSheet("font-size: 18px; margin: 10px;");
        mainLayout->addWidget(questionLabel);

        // 顯示選項
        QGridLayout *optionsLayout = new QGridLayout();
        for (int i = 0; i < optionsText.size(); ++i) {
            QLabel *optionLabel = new QLabel(optionsText[i], fullScreenWindow);
            optionLabel->setStyleSheet("font-size: 16px;");
            optionsLayout->addWidget(optionLabel, i / 2, i % 2);  // 兩列顯示
        }

        mainLayout->addLayout(optionsLayout);

        // 關閉按鈕
        QPushButton *closeButton = new QPushButton(QStringLiteral("退出"), fullScreenWindow);
        closeButton->setFixedSize(100, 50);
        mainLayout->addWidget(closeButton);
        mainLayout->setAlignment(closeButton, Qt::AlignCenter);

        connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);

        fullScreenWindow->setStyleSheet("background-color: white;");
        fullScreenWindow->setLayout(mainLayout);
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
