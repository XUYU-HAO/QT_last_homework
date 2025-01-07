#include "TcpFileServerandSender.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QTextEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include <QMessageBox>

TcpFileServerandSender::TcpFileServerandSender(QWidget *parent)
    : QWidget(parent), sender(new TcpFileSender(this)), receiver(new TcpFileServer(this))
{
    // 設置主窗口大小
    setFixedSize(450, 250);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QPushButton *teacherButton = new QPushButton(QStringLiteral("老師端"), this);
    QPushButton *studentButton = new QPushButton(QStringLiteral("學生端"), this);

    teacherButton->setFixedSize(200, 200);
    studentButton->setFixedSize(200, 200);

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

        QLabel *courseNameLabel = new QLabel(courseName, fullScreenWindow);
        courseNameLabel->setAlignment(Qt::AlignCenter);
        courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin: 20px;");
        mainLayout->addWidget(courseNameLabel);

        QTableWidget *studentTable = new QTableWidget(fullScreenWindow);
        studentTable->setColumnCount(3);
        studentTable->setHorizontalHeaderLabels(QStringList() << "學號" << "分數" << "狀態");
        studentTable->horizontalHeader()->setStretchLastSection(true);
        studentTable->verticalHeader()->setVisible(false);
        studentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        studentTable->setSelectionMode(QAbstractItemView::NoSelection);
        studentTable->setFixedWidth(300);
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
                studentTable->insertRow(row);        // 插入新的一行

                // 插入學生的學號
                QTableWidgetItem *idItem = new QTableWidgetItem(studentId);
                idItem->setTextAlignment(Qt::AlignCenter);
                studentTable->setItem(row, 0, idItem);

                // 插入學生的分數，預設為 0
                QTableWidgetItem *scoreItem = new QTableWidgetItem("0");
                scoreItem->setTextAlignment(Qt::AlignCenter);
                studentTable->setItem(row, 1, scoreItem);

                // 插入學生的狀態，預設為 "在線"
                QTableWidgetItem *statusItem = new QTableWidgetItem("在線");
                statusItem->setTextAlignment(Qt::AlignCenter);
                studentTable->setItem(row, 2, statusItem);
            }
        });
        connect(receiver, &TcpFileServer::studentCorrectAnswer, this, [studentTable](const QString &studentId) {
            qDebug() << "接收到 studentCorrectAnswer 信號，學號：" << studentId;
            qDebug() << "新增學生到表格，學號：" << studentId;
            // 遍歷學生表格，找到匹配的學號
            for (int row = 0; row < studentTable->rowCount(); ++row) {
                QTableWidgetItem *idItem = studentTable->item(row, 0); // 第 0 列存放學號
                if (idItem && idItem->text().trimmed() == studentId.trimmed()) {
                    // 找到匹配學號，更新分數
                    qDebug() << "找到學號：" << studentId << "，更新分數中...";
                    QTableWidgetItem *scoreItem = studentTable->item(row, 1); // 第 1 列存放分數
                    if (scoreItem) {
                        int currentScore = scoreItem->text().toInt(); // 獲取當前分數
                        currentScore++; // 增加分數
                        scoreItem->setText(QString::number(currentScore)); // 更新分數
                        qDebug() << "學號" << studentId << "的新分數為:" << currentScore;
                    } else {
                        qWarning() << "分數欄位為空，無法更新分數。";
                    }
                    return; // 找到後直接退出循環
                }
            }
            qWarning() << "未找到學號:" << studentId << "，無法更新分數。";
        });
        QVBoxLayout *questionLayout = new QVBoxLayout();
        QLabel *questionLabel = new QLabel("題目輸入區", fullScreenWindow);
        questionLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
        QTextEdit *questionInput = new QTextEdit(fullScreenWindow);

        questionLayout->addWidget(questionLabel);
        questionLayout->addWidget(questionInput);

        optionInputs.clear();

        // 選項文字輸入框與正確答案選擇
        QVBoxLayout *optionsLayout = new QVBoxLayout();
        QButtonGroup *correctAnswerGroup = new QButtonGroup(fullScreenWindow);
        for (int i = 1; i <= 4; ++i) {
            QHBoxLayout *optionLayout = new QHBoxLayout();

            QLineEdit *optionInput = new QLineEdit(fullScreenWindow);
            optionInput->setPlaceholderText(QStringLiteral("選項%1").arg(i));
            optionsLayout->addWidget(optionInput);

            QRadioButton *correctAnswerButton = new QRadioButton(QStringLiteral("正確答案"), fullScreenWindow);
            correctAnswerGroup->addButton(correctAnswerButton, i - 1); // 將按鈕加入組，設置ID為選項索引
            optionLayout->addWidget(optionInput);
            optionLayout->addWidget(correctAnswerButton);

            optionsLayout->addLayout(optionLayout);

            optionInputs.append(optionInput); // 保存選項輸入框
        }
        questionLayout->addLayout(optionsLayout);
        QHBoxLayout *buttonLayout = new QHBoxLayout();

        QPushButton *closeButton = new QPushButton(QStringLiteral("關閉伺服器"), fullScreenWindow);
        closeButton->setFixedSize(100, 50);
        connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);
        buttonLayout->addWidget(closeButton);

        QPushButton *createQuestionButton = new QPushButton(QStringLiteral("創建題目"), fullScreenWindow);
        createQuestionButton->setFixedSize(100, 50);
        buttonLayout->addWidget(createQuestionButton);

        connect(createQuestionButton, &QPushButton::clicked, this, [this, correctAnswerGroup, questionInput]() {
            questionText = questionInput->toPlainText();  // 保存題目
            optionsText.clear();

            for (QLineEdit *input : optionInputs) {
                if (input) {
                    optionsText.append(input->text());  // 保存選項
                }
            }

            // 獲取正確答案索引
            int correctAnswerIndex = correctAnswerGroup->checkedId(); // 取得選中的正確答案索引

            qDebug() << "題目:" << questionText;
            qDebug() << "選項:" << optionsText;
            qDebug() << "正確答案索引:" << correctAnswerIndex;

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_6);

            // 傳送題目、選項與正確答案
            out << QString("question") << questionText << optionsText << correctAnswerIndex;

            for (QTcpSocket *client : receiver->getClientConnections()) {
                client->write(block);
                client->flush();
            }
        });


        questionLayout->addLayout(buttonLayout);

        QHBoxLayout *contentLayout = new QHBoxLayout();
        contentLayout->addWidget(studentTable);
        contentLayout->addLayout(questionLayout);

        mainLayout->addLayout(contentLayout);
        // 新增「更新記分板」按鈕
        QMap<QString, bool> studentAnswerStatus; // 用於追蹤學生是否已答對

        // 接收正確答案信號，更新答題狀態
        connect(receiver, &TcpFileServer::studentCorrectAnswer, this, [this, &studentAnswerStatus](const QString &studentId) {
            studentAnswerStatus[studentId] = true; // 設置該學生的狀態為已答對
            qDebug() << "學號" << studentId << "的答題狀態已更新為: 已答對";
        });

        fullScreenWindow->setLayout(mainLayout);
        fullScreenWindow->setStyleSheet("background-color: white;");
        fullScreenWindow->showFullScreen();
    });

}

void TcpFileServerandSender::startStudentMode()
{
    sender->show();
    connect(sender->getTcpClient(), &QTcpSocket::readyRead, this, [this]() {
        QTcpSocket *client = sender->getTcpClient();
        QDataStream in(client);
        in.setVersion(QDataStream::Qt_4_6);

        QString messageType;
        in >> messageType;

        if (messageType == "question") {
            int correctAnswerIndex; // 定義接收正確答案的變數
            in >> questionText >> optionsText >> correctAnswerIndex; // 接收題目、選項和正確答案索引

            this->close();

            // 創建全屏顯示窗口
            QWidget *fullScreenWindow = new QWidget();
            QVBoxLayout *mainLayout = new QVBoxLayout(fullScreenWindow);

            // 顯示課程名稱
            QLabel *courseNameLabel = new QLabel(receiver->getCourseName(), fullScreenWindow);
            courseNameLabel->setAlignment(Qt::AlignCenter);
            courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin-top: 20px;");
            mainLayout->addWidget(courseNameLabel);

            // 顯示題目
            QLabel *questionLabel = new QLabel(questionText, fullScreenWindow);
            questionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            questionLabel->setStyleSheet("font-size: 18px; margin: 10px;");
            mainLayout->addWidget(questionLabel);

            // 顯示選項按鈕
            QGridLayout *optionsLayout = new QGridLayout();
            for (int i = 0; i < optionsText.size(); ++i) {
                QPushButton *optionButton = new QPushButton(optionsText[i], fullScreenWindow);
                optionButton->setStyleSheet("font-size: 16px; padding: 10px;");
                optionsLayout->addWidget(optionButton, i / 2, i % 2);

                // 處理學生選擇選項的事件
                connect(optionButton, &QPushButton::clicked, this, [this, i, correctAnswerIndex,client, fullScreenWindow]() {
                    QString selectedOption = optionsText[i];
                    qDebug() << "學生選擇了選項:" << selectedOption;

                    QByteArray block;
                    QDataStream out(&block, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_6);

                    // 傳送選項索引
                    out << QString("answer") << (i);
                    client->write(block);
                    client->flush();
                    qDebug() << "傳送選項索引:" << i;
                    qDebug() << "傳送選項索引:" << correctAnswerIndex;
                    // 接收判斷結果（模擬處理，實際應由伺服器回應判斷結果）
                    bool isCorrect = (i == correctAnswerIndex); // 假設有一個 `correctAnswerIndex` 保存正確答案索引

                    // 顯示結果對話框
                    QMessageBox::information(
                        fullScreenWindow,
                        isCorrect ? "結果" : "錯誤",
                        isCorrect ? "答對了！" : "答錯了！"
                        );

                    // 關閉題目窗口
                    fullScreenWindow->close();
                });
            }

            mainLayout->addLayout(optionsLayout);

            QPushButton *closeButton = new QPushButton(QStringLiteral("退出"), fullScreenWindow);
            closeButton->setFixedSize(100, 50);
            mainLayout->addWidget(closeButton);
            mainLayout->setAlignment(closeButton, Qt::AlignCenter);

            connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);

            fullScreenWindow->setStyleSheet("background-color: white;");
            fullScreenWindow->setLayout(mainLayout);
            fullScreenWindow->showFullScreen();
        }
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
