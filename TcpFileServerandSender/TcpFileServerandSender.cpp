#include "TcpFileServerandSender.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QTextEdit>
#include <QComboBox>
#include <QMessageBox>  // 引入QMessageBox來顯示消息框

TcpFileServerandSender::TcpFileServerandSender(QWidget *parent)
    : QWidget(parent), sender(new TcpFileSender(this)), receiver(new TcpFileServer(this)), fullScreenWindow(nullptr)
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

        fullScreenWindow = new QWidget();
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

        QVBoxLayout *questionLayout = new QVBoxLayout();
        QLabel *questionLabel = new QLabel("題目輸入區", fullScreenWindow);
        questionLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
        QTextEdit *questionInput = new QTextEdit(fullScreenWindow);

        questionLayout->addWidget(questionLabel);
        questionLayout->addWidget(questionInput);

        optionInputs.clear();

        QVBoxLayout *optionsLayout = new QVBoxLayout();
        for (int i = 1; i <= 4; ++i) {
            QLineEdit *optionInput = new QLineEdit(fullScreenWindow);
            optionInput->setPlaceholderText(QStringLiteral("選項%1").arg(i));
            optionsLayout->addWidget(optionInput);

            optionInputs.append(optionInput);
        }
        questionLayout->addLayout(optionsLayout);

        QLabel *correctAnswerLabel = new QLabel("選擇正確答案", fullScreenWindow);
        QComboBox *correctAnswerCombo = new QComboBox(fullScreenWindow);
        correctAnswerCombo->addItems(QStringList() << "選項1" << "選項2" << "選項3" << "選項4");
        questionLayout->addWidget(correctAnswerLabel);
        questionLayout->addWidget(correctAnswerCombo);

        QHBoxLayout *buttonLayout = new QHBoxLayout();

        QPushButton *closeButton = new QPushButton(QStringLiteral("關閉伺服器"), fullScreenWindow);
        closeButton->setFixedSize(100, 50);
        connect(closeButton, &QPushButton::clicked, fullScreenWindow, &QWidget::close);
        buttonLayout->addWidget(closeButton);

        QPushButton *createQuestionButton = new QPushButton(QStringLiteral("創建題目"), fullScreenWindow);
        createQuestionButton->setFixedSize(100, 50);
        buttonLayout->addWidget(createQuestionButton);

        connect(createQuestionButton, &QPushButton::clicked, this, [this, questionInput, correctAnswerCombo]() {
            questionText = questionInput->toPlainText();
            optionsText.clear();

            for (QLineEdit *input : optionInputs) {
                if (input) {
                    optionsText.append(input->text());
                }
            }

            correctAnswer = correctAnswerCombo->currentIndex(); // 存儲正確選項的索引

            qDebug() << "題目:" << questionText;
            qDebug() << "選項:" << optionsText;
            qDebug() << "正確答案索引:" << correctAnswer;

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_6);

            out << QString("question") << questionText << optionsText << correctAnswer;

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
            in >> questionText >> optionsText >> correctAnswer;

            this->close();

            fullScreenWindow = new QWidget();
            QVBoxLayout *mainLayout = new QVBoxLayout(fullScreenWindow);

            QLabel *courseNameLabel = new QLabel(receiver->getCourseName(), fullScreenWindow);
            courseNameLabel->setAlignment(Qt::AlignCenter);
            courseNameLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin-top: 20px;");
            mainLayout->addWidget(courseNameLabel);

            QLabel *questionLabel = new QLabel(questionText, fullScreenWindow);
            questionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            questionLabel->setStyleSheet("font-size: 18px; margin: 10px;");
            mainLayout->addWidget(questionLabel);

            QGridLayout *optionsLayout = new QGridLayout();
            for (int i = 0; i < optionsText.size(); ++i) {
                QPushButton *optionButton = new QPushButton(optionsText[i], fullScreenWindow);
                optionButton->setStyleSheet("font-size: 16px; padding: 10px;");
                optionsLayout->addWidget(optionButton, i / 2, i % 2);

                connect(optionButton, &QPushButton::clicked, this, [this, i]() {
                    // 檢查選擇的答案是否正確
                    if (i == correctAnswer) {
                        sender->sendStudentAnswer("correct");

                        // 顯示答對的消息框
                        QMessageBox::information(fullScreenWindow, "恭喜!", "答對了！", QMessageBox::Ok);
                    } else {
                        sender->sendStudentAnswer("incorrect");

                        // 顯示答錯的消息框
                        QMessageBox::critical(fullScreenWindow, "答錯了", "答錯了，你真的好蔡！", QMessageBox::Ok);
                    }

                    fullScreenWindow->close(); // 關閉 UI
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

void TcpFileServerandSender::handleStudentAnswer(const QString &studentId, const QString &answerStatus)
{
    if (!fullScreenWindow) return; // 確保全屏窗口存在

    QTableWidget *studentTable = nullptr;
    // 假設您將學生表格放在全屏窗口的某個布局中
    for (QWidget *widget : fullScreenWindow->findChildren<QWidget*>()) {
        if (QTableWidget *table = qobject_cast<QTableWidget*>(widget)) {
            studentTable = table;
            break;
        }
    }

    if (!studentTable) return; // 如果沒有找到學生表格，則退出

    for (int row = 0; row < studentTable->rowCount(); ++row) {
        QTableWidgetItem *idItem = studentTable->item(row, 0);
        if (idItem && idItem->text() == studentId) {
            QTableWidgetItem *scoreItem = studentTable->item(row, 1);
            int currentScore = scoreItem->text().toInt();

            if (answerStatus == "correct") {
                currentScore++; // 正確答案加分
            }

            scoreItem->setText(QString::number(currentScore));

            // 如果需要，您也可以更新學生的狀態或其他信息
            QTableWidgetItem *statusItem = studentTable->item(row, 2);
            statusItem->setText(answerStatus == "correct" ? "正確" : "錯誤");

            break; // 找到對應學生並更新後退出
        }
    }
}

