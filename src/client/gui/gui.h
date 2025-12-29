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

private:
    Ui::wConnect *ui;
};


class wRole : public QWidget {
    Q_OBJECT
public:
    explicit wRole(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wRole) {
        ui->setupUi(this);
        connect(ui->hosterBtn, &QPushButton::clicked, this, [this]() {
            emit chooseRole(true);
        });
        connect(ui->userBtn, &QPushButton::clicked, this, [this]() {
            emit chooseRole(false);
        });
    }

    ~wRole() { delete ui; }

signals:
    void chooseRole(bool isHoster);

private:
    Ui::wRole *ui;
};


class wNick : public QWidget {
    Q_OBJECT
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

private:
    Ui::wNick *ui;
};


class wSetup : public QWidget {
    Q_OBJECT
public:
    explicit wSetup(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wSetup) {
        ui->setupUi(this);
        connect(ui->listBtn, &QPushButton::clicked, this, [this]() {
            emit listQuizzes();
        });
        connect(ui->fileBtn, &QPushButton::clicked, this, [this]() {
            QString path = QFileDialog::getOpenFileName(
                this, "Wybierz plik quizu", "quizSource", "Quiz files (*.qcf)"
            );
            emit selectQuizFile(path);
        });
        connect(ui->sendBtn, &QPushButton::clicked, this, [this]() {
            emit postQuiz();
        });
        connect(ui->selectBtn, &QPushButton::clicked, this, [this]() {
            emit selectQuiz(ui->lineEdit->text());
        });
        connect(ui->codeBtn, &QPushButton::clicked, this, [this]() {
            emit requestCode();
        });
        connect(ui->launchBtn, &QPushButton::clicked, this, [this]() {
            emit launchQuiz();
        });
    }

    void showList(const QString &text) {
        ui->quizzes->setPlainText(text);
    }

    void setCodeLabel(const QString &code) { ui->codeLabel->setText("Twój kod: " + code); }

    ~wSetup() { delete ui; }

signals:
    void listQuizzes();
    void selectQuizFile(const QString &path);
    void postQuiz();
    void selectQuiz(QString name);
    void requestCode();
    void launchQuiz();

private:
    Ui::wSetup *ui;
    QString filePath;
};


class wPanel : public QWidget {
    Q_OBJECT
public:
    explicit wPanel(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wPanel) {
        ui->setupUi(this);
    }

    void showRank(const QString &text) {
        ui->textBrowser->setPlainText(text);
    }

    ~wPanel() { delete ui; }

private:
    Ui::wPanel *ui;
};


class wWaiting : public QWidget {
    Q_OBJECT
public:
    explicit wWaiting(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wWaiting) {
        ui->setupUi(this);
    }

    ~wWaiting() { delete ui; }

private:
    Ui::wWaiting *ui;
};


class wQuestion : public QWidget {
    Q_OBJECT
public:
    explicit wQuestion(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wQuestion) {
        ui->setupUi(this);
        connect(ui->btn1, &QPushButton::clicked, this, [this]() {
            emit answer(0);
        });
        connect(ui->btn2, &QPushButton::clicked, this, [this]() {
            emit answer(1);
        });
        connect(ui->btn3, &QPushButton::clicked, this, [this]() {
            emit answer(2);
        });
        connect(ui->btn4, &QPushButton::clicked, this, [this]() {
            emit answer(3);
        });
    }

    void setScoreLabel(const QString &content) {
        ui->score->setText("Wynik: " + content);
    }

    void loadQuestion(const QString &raw) {
        QRegularExpression re("\"([^\"]*)\"");
        QRegularExpressionMatchIterator it = re.globalMatch(raw);

        QStringList parts;
        while (it.hasNext()) {
            parts << it.next().captured(1);
        }

        if (parts.size() < 1)
            return;

        ui->qText->setText(parts[0]);

        QPushButton *buttons[4] = { ui->btn1, ui->btn2, ui->btn3, ui->btn4 };

        for (int i = 0; i < 4; i++) {
            if (i + 1 < parts.size())
                buttons[i]->setText(parts[i + 1]);
            else
                buttons[i]->setText("-");
        }
    }

    ~wQuestion() { delete ui; }

signals:
    void answer(int answerId);
    void goNext();

private:
    Ui::wQuestion *ui;
};


class wSummary : public QWidget {
    Q_OBJECT
public:
    explicit wSummary(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wSummary) {
        ui->setupUi(this);
    }

    void showRank(const QString &text) {
        ui->textBrowser->setPlainText(text);
    }

    ~wSummary() { delete ui; }

private:
    Ui::wSummary *ui;
};


class wJoin : public QWidget {
    Q_OBJECT
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

private:
    Ui::wJoin *ui;
};


#endif // GUI_H
