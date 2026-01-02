#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QFileDialog>
#include <QTimer>
#include <QRegularExpression>

#include "ui_Connect.h"
#include "ui_Role.h"
#include "ui_Nick.h"
#include "ui_Setup.h"
#include "ui_Panel.h"
#include "ui_Waiting.h"
#include "ui_Question.h"
#include "ui_Summary.h"
#include "ui_Join.h"

#include "network.h"
#include "user.h"


class wConnect : public QWidget {
    Q_OBJECT
private:
    Ui::wConnect *ui;
public:
    explicit wConnect(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wConnect) {
        ui->setupUi(this);

        connect(ui->connectBtn, &QPushButton::clicked, this, [this]() { 
            QString ip   = ui->ipInput->text();
            QString port = ui->portInput->text();
            emit connectToServer(ip, port);
        });
    }

    ~wConnect() { delete ui; }

signals:
    void connectToServer(QString ip, QString port);
};


class wRole : public QWidget {
    Q_OBJECT
private:
    Ui::wRole *ui;
public:
    explicit wRole(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wRole) {
        ui->setupUi(this);

        connect(ui->hosterBtn, &QPushButton::clicked, this, [this]() { emit chooseRole(true); });
        connect(ui->userBtn,   &QPushButton::clicked, this, [this]() { emit chooseRole(false); });
    }

    ~wRole() { delete ui; }

signals:
    void chooseRole(bool isHoster);
};


class wNick : public QWidget {
    Q_OBJECT
private:
    Ui::wNick *ui;
public:
    explicit wNick(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wNick) {
        ui->setupUi(this);

        connect(ui->nickBtn, &QPushButton::clicked, this, [this]() {
            QString nick = ui->nickInput->text();
            emit proposeNickname(nick);
        });
    }

    ~wNick() { delete ui; }

signals:
    void proposeNickname(QString nick);
};


class wSetup : public QWidget {
    Q_OBJECT
private:
    Ui::wSetup *ui;
public:
    explicit wSetup(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wSetup) {
        ui->setupUi(this);

        connect(ui->listBtn,   &QPushButton::clicked, this, [this]() { emit listQuizzes(); });
        connect(ui->sendBtn,   &QPushButton::clicked, this, [this]() { emit postQuiz(); });
        connect(ui->selectBtn, &QPushButton::clicked, this, [this]() { emit selectQuiz(ui->lineEdit->text()); });
        connect(ui->codeBtn,   &QPushButton::clicked, this, [this]() { emit requestCode(); });
        connect(ui->launchBtn, &QPushButton::clicked, this, [this]() { emit launchQuiz(); });
        connect(ui->fileBtn,   &QPushButton::clicked, this, [this]() {
            QString path = QFileDialog::getOpenFileName(
                this, "Wybierz plik quizu", "quizSource", "Quiz files (*.qcf)"
            );
            emit selectQuizFile(path);
        });
    }

    ~wSetup() { delete ui; }

    void setCodeLabel(const QString &code) { ui->codeLabel->setText("Twój kod: " + code); }
    void setQuizLabel(const QString &name) { ui->quizLabel->setText("Wybrany: " + name); }

    void showList(const QString &text) {
        ui->quizzes->setPlainText(text);
    }

signals:
    void listQuizzes();
    void selectQuizFile(const QString &path);
    void postQuiz();
    void selectQuiz(QString name);
    void requestCode();
    void launchQuiz();
};


class wPanel : public QWidget {
    Q_OBJECT
private:
    Ui::wPanel *ui;
public:
    explicit wPanel(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wPanel) {
        ui->setupUi(this);
    }

    ~wPanel() { delete ui; }

    void showRank(const QString &text) {
        ui->textBrowser->setPlainText(text);
    }

    void setQuizLabel(const QString &name) { ui->qName->setText(name); }
    void setQuestionNumLabel(const QString &num) { ui->qNum->setText("Aktualne pytanie: " + num); }
    void setUserCount(const QString &count) { ui->userNum->setText("Uczestników: " + count); }
};


class wWaiting : public QWidget {
    Q_OBJECT
private:
    Ui::wWaiting *ui;
public:
    explicit wWaiting(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wWaiting) {
        ui->setupUi(this);
    }

    ~wWaiting() { delete ui; }
};


class wQuestion : public QWidget {
    Q_OBJECT
private:
    Ui::wQuestion *ui;
public:
    explicit wQuestion(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wQuestion) {
        ui->setupUi(this);

        connect(ui->btn1, &QPushButton::clicked, this, [this]() { emit answer(0); });
        connect(ui->btn2, &QPushButton::clicked, this, [this]() { emit answer(1); });
        connect(ui->btn3, &QPushButton::clicked, this, [this]() { emit answer(2); });
        connect(ui->btn4, &QPushButton::clicked, this, [this]() { emit answer(3); });
    }

    ~wQuestion() { delete ui; }

    void setScoreLabel(const QString &content) { ui->score->setText("Wynik: " + content); }

    void loadQuestion(const QString &question) {
        std::istringstream ss(question.toStdString());

        std::vector<QString> parts;
        std::string part;

        while (ss >> std::ws && ss.peek() == '"') {
            ss.get();
            std::getline(ss, part, '"');
            parts.push_back(QString::fromStdString(part));
        }
        if (parts.empty()) return;

        ui->qText->setText(parts[0]);

        QPushButton *buttons[4] = { ui->btn1, ui->btn2, ui->btn3, ui->btn4 };
        for (size_t i = 0; i < 4; i++) {
            if (i + 1 < parts.size()) {
                buttons[i]->setText(parts[i + 1]);
            } else {
                buttons[i]->setText("");
            }
        }
    }

signals:
    void answer(int answerId);
};


class wSummary : public QWidget {
    Q_OBJECT
private:
    Ui::wSummary *ui;
public:
    explicit wSummary(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wSummary) {
        ui->setupUi(this);

        connect(ui->goNextBtn, &QPushButton::clicked, this, [&]() { emit goNext(); });
    }

    ~wSummary() { delete ui; }

    void showRank(const QString &text) {
        ui->textBrowser->setPlainText(text);
    }

signals:
    void goNext();
};


class wJoin : public QWidget {
    Q_OBJECT
private:
    Ui::wJoin *ui;
public:
    explicit wJoin(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wJoin) {
        ui->setupUi(this);

        connect(ui->codeBtn, &QPushButton::clicked, this, [this]() {
            QString code = ui->codeInput->text();
            emit joinQuiz(code);
        });
    }

    ~wJoin() { delete ui; }

signals:
    void joinQuiz(QString code);
};

#endif // GUI_H
