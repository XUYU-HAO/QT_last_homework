#include "TcpFileServerandSender.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QTextEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include <QMessageBox>
#include <QApplication>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <QSpinBox>

TcpFileServerandSender::TcpFileServerandSender(QWidget *parent)
    : QWidget(parent), sender(new TcpFileSender(this)), receiver(new TcpFileServer(this)), waitingWindow(nullptr)
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

        // 課程名稱
        QLabel *courseNameLabel = new QLabel(courseName, fullScreenWindow);
        courseNameLabel->setAlignment(Qt::AlignCenter);
        courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin: 20px;");
        mainLayout->addWidget(courseNameLabel);

        // 學生狀態表
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
                QTableWidgetItem *idItem = studentTable->item(row, 0);
                if (idItem && idItem->text() == studentId) {
                    studentExists = true;
                    break;
                }
            }

            if (!studentExists) {
                int row = studentTable->rowCount();
                studentTable->insertRow(row);

                QTableWidgetItem *idItem = new QTableWidgetItem(studentId);
                idItem->setTextAlignment(Qt::AlignCenter);
                studentTable->setItem(row, 0, idItem);

                QTableWidgetItem *scoreItem = new QTableWidgetItem("0");
                scoreItem->setTextAlignment(Qt::AlignCenter);
                studentTable->setItem(row, 1, scoreItem);

                QTableWidgetItem *statusItem = new QTableWidgetItem("在線");
                statusItem->setTextAlignment(Qt::AlignCenter);
                studentTable->setItem(row, 2, statusItem);
            }
        });

        // 答題時間設定區域
        QHBoxLayout *timeSettingLayout = new QHBoxLayout();

        QSlider *timeSlider = new QSlider(Qt::Horizontal, fullScreenWindow);
        timeSlider->setRange(1, 120);
        timeSlider->setValue(60);
        timeSlider->setTickPosition(QSlider::TicksBelow);
        timeSlider->setTickInterval(10);
        timeSlider->setFixedWidth(200);

        QSpinBox *timeSpinBox = new QSpinBox(fullScreenWindow);
        timeSpinBox->setRange(1, 120);
        timeSpinBox->setValue(60);
        timeSpinBox->setSuffix(" 秒");
        timeSpinBox->setAlignment(Qt::AlignCenter);

        QLabel *timeLabel = new QLabel("答題時間設定:", fullScreenWindow);
        timeLabel->setAlignment(Qt::AlignRight);
        timeLabel->setStyleSheet("font-size: 16px;");

        timeSettingLayout->addWidget(timeLabel);
        timeSettingLayout->addWidget(timeSlider);
        timeSettingLayout->addWidget(timeSpinBox);

        connect(timeSlider, &QSlider::valueChanged, timeSpinBox, &QSpinBox::setValue);
        connect(timeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), timeSlider, &QSlider::setValue);

        mainLayout->addLayout(timeSettingLayout);

        // 題目與選項設置區域
        QVBoxLayout *questionLayout = new QVBoxLayout();

        QLabel *questionLabel = new QLabel("題目輸入區", fullScreenWindow);
        questionLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
        QTextEdit *questionInput = new QTextEdit(fullScreenWindow);
        questionLayout->addWidget(questionLabel);
        questionLayout->addWidget(questionInput);

        QVBoxLayout *optionsLayout = new QVBoxLayout();
        QButtonGroup *correctAnswerGroup = new QButtonGroup(fullScreenWindow);
        optionInputs.clear();

        for (int i = 1; i <= 4; ++i) {
            QHBoxLayout *optionLayout = new QHBoxLayout();

            QLineEdit *optionInput = new QLineEdit(fullScreenWindow);
            optionInput->setPlaceholderText(QStringLiteral("選項%1").arg(i));

            QRadioButton *correctAnswerButton = new QRadioButton(QStringLiteral("正確答案"), fullScreenWindow);
            correctAnswerGroup->addButton(correctAnswerButton, i - 1);

            optionLayout->addWidget(optionInput);
            optionLayout->addWidget(correctAnswerButton);

            optionsLayout->addLayout(optionLayout);
            optionInputs.append(optionInput);
        }

        questionLayout->addLayout(optionsLayout);

        // 操作按鈕區域
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *closeButton = new QPushButton(QStringLiteral("關閉伺服器"), fullScreenWindow);
        closeButton->setFixedSize(100, 50);
        connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);
        buttonLayout->addWidget(closeButton);

        QPushButton *createQuestionButton = new QPushButton(QStringLiteral("創建題目"), fullScreenWindow);
        createQuestionButton->setFixedSize(100, 50);
        buttonLayout->addWidget(createQuestionButton);

        connect(createQuestionButton, &QPushButton::clicked, this, [this, correctAnswerGroup, questionInput, timeSlider]() {
            questionText = questionInput->toPlainText();
            optionsText.clear();

            for (QLineEdit *input : optionInputs) {
                if (input) {
                    optionsText.append(input->text());
                }
            }

            int correctAnswerIndex = correctAnswerGroup->checkedId();
            int answerTime = timeSlider->value();

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_6);

            out << QString("question") << questionText << optionsText << correctAnswerIndex << answerTime;

            for (QTcpSocket *client : receiver->getClientConnections()) {
                if (client && client->state() == QAbstractSocket::ConnectedState) {
                    client->write(block);
                    client->flush();
                    qDebug() << "已發送題目給學生：" << client->peerAddress().toString();
                }
            }
        });

        questionLayout->addLayout(buttonLayout);

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

    studentScore = 0; // 初始化分數為 0

    connect(sender->getTcpClient(), &QTcpSocket::connected, this, [this]() {
        qDebug() << "學生端已成功連線到伺服器";

        if (!waitingWindow) {
            waitingWindow = new QWidget();
            QVBoxLayout *waitingLayout = new QVBoxLayout(waitingWindow);

            QLabel *waitingLabel = new QLabel("等待老師出題中...", waitingWindow);
            waitingLabel->setAlignment(Qt::AlignCenter);
            waitingLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: gray;");
            waitingLayout->addWidget(waitingLabel);

            // 分數顯示標籤
            scoreLabel = new QLabel(QString("目前分數: %1").arg(studentScore), waitingWindow);
            scoreLabel->setAlignment(Qt::AlignCenter);
            scoreLabel->setStyleSheet("font-size: 24px; color: blue;");
            waitingLayout->addWidget(scoreLabel);

            // 新增「更新分數」按鈕，移到分數標籤下方
            QPushButton *updateScoreButton = new QPushButton("更新分數", waitingWindow);
            updateScoreButton->setFixedSize(200, 50);
            updateScoreButton->setStyleSheet("font-size: 18px;");
            waitingLayout->addWidget(updateScoreButton, 0, Qt::AlignCenter);

            connect(updateScoreButton, &QPushButton::clicked, this, [this]() {
                QTcpSocket *client = sender->getTcpClient();
                if (client->state() == QAbstractSocket::ConnectedState) {
                    QByteArray block;
                    QDataStream out(&block, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_6);

                    out << QString("updateScore"); // 向伺服器請求更新分數
                    client->write(block);
                    client->flush();

                    qDebug() << "已向伺服器請求更新分數";
                } else {
                    QMessageBox::warning(waitingWindow, "錯誤", "尚未連線至伺服器，無法更新分數！");
                }
            });

            QPushButton *exitButton = new QPushButton("退出", waitingWindow);
            exitButton->setFixedSize(150, 100);
            waitingLayout->addWidget(exitButton);
            waitingLayout->setAlignment(exitButton, Qt::AlignCenter);

            connect(exitButton, &QPushButton::clicked, this, [this]() {
                QApplication::quit();
            });

            waitingWindow->setLayout(waitingLayout);
            waitingWindow->setStyleSheet("background-color: white;");
        }

        waitingWindow->showFullScreen();
    });

    connect(sender->getTcpClient(), &QTcpSocket::readyRead, this, [this]() {
        QTcpSocket *client = sender->getTcpClient();
        QDataStream in(client);
        in.setVersion(QDataStream::Qt_4_6);

        QString messageType;
        in >> messageType;

        if (messageType == "result") {
            bool isCorrect;
            in >> isCorrect;

            // 更新分數
            if (isCorrect) {
                studentScore += 10; // 答對加 10 分
                qDebug() << "答對，加分後目前分數：" << studentScore;
            } else {
                qDebug() << "答錯，分數保持不變：" << studentScore;
            }

            // 更新分數標籤
            if (scoreLabel) {
                scoreLabel->setText(QString("目前分數: %1").arg(studentScore));
            }
        }

        if (messageType == "updateScore") {
            // 處理伺服器回傳的更新分數訊息
            int updatedScore;
            in >> updatedScore;

            studentScore = updatedScore; // 更新本地分數
            if (scoreLabel) {
                scoreLabel->setText(QString("目前分數: %1").arg(studentScore));
            }

            qDebug() << "從伺服器獲取最新分數：" << studentScore;
        }

        if (messageType == "question") {
            if (waitingWindow) {
                waitingWindow->hide();
            }

            int correctAnswerIndex, timeLimit;
            in >> questionText >> optionsText >> correctAnswerIndex >> timeLimit;

            QWidget *questionWindow = new QWidget();
            QVBoxLayout *mainLayout = new QVBoxLayout(questionWindow);

            QLabel *courseNameLabel = new QLabel("答題開始", questionWindow);
            courseNameLabel->setAlignment(Qt::AlignCenter);
            courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin-top: 20px;");
            mainLayout->addWidget(courseNameLabel);

            QLabel *questionLabel = new QLabel(questionText, questionWindow);
            questionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            questionLabel->setStyleSheet("font-size: 18px; margin: 10px;");
            mainLayout->addWidget(questionLabel);

            QGridLayout *optionsLayout = new QGridLayout();
            for (int i = 0; i < optionsText.size(); ++i) {
                QPushButton *optionButton = new QPushButton(optionsText[i], questionWindow);
                optionButton->setStyleSheet("font-size: 16px; padding: 10px;");
                optionsLayout->addWidget(optionButton, i / 2, i % 2);

                connect(optionButton, &QPushButton::clicked, this, [this, i, correctAnswerIndex, client, questionWindow]() {
                    bool isCorrect = (i == correctAnswerIndex);

                    QByteArray block;
                    QDataStream out(&block, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_6);
                    out << QString("answerResult") << isCorrect;

                    client->write(block);
                    client->flush();
                    qDebug() << "答案結果已回傳給伺服器：" << (isCorrect ? "正確" : "錯誤");

                    QMessageBox::information(
                        questionWindow,
                        isCorrect ? "結果" : "錯誤",
                        isCorrect ? "答對了！" : "答錯了！"
                        );

                    questionWindow->close();
                    waitingWindow->showFullScreen();
                });
            }

            mainLayout->addLayout(optionsLayout);

            // 倒計時標籤
            QLabel *timeLabel = new QLabel(QString("剩餘時間: %1").arg(timeLimit), questionWindow);
            timeLabel->setAlignment(Qt::AlignRight);
            timeLabel->setStyleSheet("font-size: 16px; margin: 10px;");
            mainLayout->addWidget(timeLabel);

            QTimer *timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, [this, timeLabel, timeLimit, questionWindow, timer]() mutable {
                int remainingTime = timeLabel->text().split(":").last().toInt();
                remainingTime--;

                if (remainingTime <= 0) {
                    timer->stop();
                    questionWindow->close();
                    waitingWindow->showFullScreen();
                    return;
                }

                timeLabel->setText(QString("剩餘時間: %1").arg(remainingTime));
            });

            timer->start(1000);  // 每秒更新一次

            QPushButton *closeButton = new QPushButton(QStringLiteral("退出"), questionWindow);
            closeButton->setFixedSize(100, 50);
            mainLayout->addWidget(closeButton);
            connect(closeButton, &QPushButton::clicked, questionWindow, &QWidget::close);

            questionWindow->setStyleSheet("background-color: white;");
            questionWindow->setLayout(mainLayout);
            questionWindow->showFullScreen();
        }
    });
}

void TcpFileServerandSender::switchToFullScreen(const QString &courseName)
{

}
