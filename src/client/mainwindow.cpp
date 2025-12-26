#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    scrConnect  = new wConnect();
    scrRole     = new wRole();
    scrNick     = new wNick();
    scrSetup    = new wSetup();
    scrPanel    = new wPanel();
    scrWait     = new wWaiting();
    scrQuestion = new wQuestion();
    scrSummary  = new wSummary();

    stack->addWidget(scrConnect);
    stack->addWidget(scrRole);
    stack->addWidget(scrNick);
    stack->addWidget(scrSetup);
    stack->addWidget(scrPanel);
    stack->addWidget(scrWait);
    stack->addWidget(scrQuestion);
    stack->addWidget(scrSummary);

    connect(scrConnect, &wConnect::goNext, this, [&](QString ip, QString port) { 
        netClient.connectToServer(ip.toStdString(), port.toStdString());
    });

    connect(scrRole, &wRole::chooseRole, this, [&](bool isHoster) {
        if (user) delete user;

        if (isHoster) {
            user = new QuizHoster();
            stack->setCurrentWidget(scrSetup);
        } else {
            user = new QuizUser();
            stack->setCurrentWidget(scrNick);
        }

        user->setNetClient(netClient);
    });

    connect(scrNick, &wNick::proposeNickname, this, [&](QString nick) {
        if (!user) return;
        user->proposeNickname(nick.toStdString());
        stack->setCurrentWidget(scrWait);//////////
    });
    // connect(&netClient, &NetworkClient::nicknameAccepted, this, [&]() {
    //     stack->setCurrentWidget(scrWait);
    // });
    // connect(&netClient, &NetworkClient::nicknameRejected, this, [&](QString reason) {
    //     return;
    // });

    connect(scrSetup,    &wSetup::goNext,    this, [&](){ stack->setCurrentWidget(scrPanel); });
    connect(scrPanel,    &wPanel::goNext,    this, [&](){ stack->setCurrentWidget(scrSummary); });
    connect(scrWait,     &wWaiting::goNext,  this, [&](){ stack->setCurrentWidget(scrQuestion); });
    connect(scrQuestion, &wQuestion::goNext, this, [&](){ stack->setCurrentWidget(scrSummary); });

    pollTimer.setInterval(10);
    connect(&pollTimer, &QTimer::timeout, this, [&]() {
        netClient.pollEvents();
        if (netClient.connected && !established) {
            stack->setCurrentWidget(scrRole);
            established = true;
        }
    });
    pollTimer.start();

    stack->setCurrentWidget(scrConnect);
}
