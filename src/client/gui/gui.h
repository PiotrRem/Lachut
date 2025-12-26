#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>

#include "ui_Connect.h"
#include "ui_Role.h"
#include "ui_Nick.h"
#include "ui_Setup.h"
#include "ui_Panel.h"
#include "ui_Waiting.h"
#include "ui_Question.h"
#include "ui_Summary.h"

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
            emit goNext(ip, port);
        });
    }

    ~wConnect() { delete ui; }

signals:
    void goNext(QString ip, QString port);

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
        connect(ui->pushButton_5, &QPushButton::clicked, this, [this](){ emit goNext(); });
    }

    ~wSetup() { delete ui; }

signals:
    void goNext();

private:
    Ui::wSetup *ui;
};


class wPanel : public QWidget {
    Q_OBJECT
public:
    explicit wPanel(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wPanel) {
        ui->setupUi(this);
        connect(ui->pushButton, &QPushButton::clicked, this, [this](){ emit goNext(); });// TODO: USUNĄĆ
    }

    ~wPanel() { delete ui; }

signals:
    void goNext();

private:
    Ui::wPanel *ui;
};


class wWaiting : public QWidget {
    Q_OBJECT
public:
    explicit wWaiting(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wWaiting) {
        ui->setupUi(this);
        connect(ui->pushButton, &QPushButton::clicked, this, [this](){ emit goNext(); });// TODO: USUNĄĆ
    }

    ~wWaiting() { delete ui; }

signals:
    void goNext();

private:
    Ui::wWaiting *ui;
};


class wQuestion : public QWidget {
    Q_OBJECT
public:
    explicit wQuestion(QWidget *parent = nullptr) : QWidget(parent), ui(new Ui::wQuestion) {
        ui->setupUi(this);

        auto handler = [this](){ emit goNext(); };
        connect(ui->btn1, &QPushButton::clicked, this, handler);
        connect(ui->btn2, &QPushButton::clicked, this, handler);
        connect(ui->btn3, &QPushButton::clicked, this, handler);
        connect(ui->btn4, &QPushButton::clicked, this, handler);
    }

    ~wQuestion() { delete ui; }

signals:
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

    ~wSummary() { delete ui; }

private:
    Ui::wSummary *ui;
};

#endif // GUI_H
