#include "mainwindow.h"

void MainWindow::goToSummary() {
    stack->setCurrentWidget(scrSummary);
    user->getRanking();
}

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
    scrJoin     = new wJoin();

    stack->addWidget(scrConnect);
    stack->addWidget(scrRole);
    stack->addWidget(scrNick);
    stack->addWidget(scrSetup);
    stack->addWidget(scrPanel);
    stack->addWidget(scrWait);
    stack->addWidget(scrQuestion);
    stack->addWidget(scrSummary);
    stack->addWidget(scrJoin);

    connect(scrConnect, &wConnect::connectToServer, this, [&](QString ip, QString port) { 
        netClient.connectToServer(ip.toStdString(), port.toStdString());
    });
    // connect(scrConnect, &wConnect::connectionFailed, this, [&](QString ip, QString port) { 
    //     netClient.connectToServer(ip.toStdString(), port.toStdString());
    // });
    // connect(scrConnect, &wConnect::connectionSucceeded, this, [&](QString ip, QString port) { 
    //     netClient.connectToServer(ip.toStdString(), port.toStdString());
    // });

    connect(scrRole, &wRole::chooseRole, this, [&](bool isHoster) {
        if (isHoster) {
            hoster = new QuizHoster();
            user = hoster;
            stack->setCurrentWidget(scrSetup);
        } else {
            player = new QuizPlayer();
            user = player;
            stack->setCurrentWidget(scrJoin);
        }

        user->setNetClient(netClient);
    });

    // TWÓRCA QUIZU

    connect(scrSetup, &wSetup::listQuizzes, this, [&]() {
        hoster->listQuizzes();
    });
    connect(scrSetup, &wSetup::selectQuizFile, this, [&](QString path) {
        qf.path = path.toStdString();
        qf.size = QFile(path).size();
    });
    connect(scrSetup, &wSetup::postQuiz, this, [&]() {
        hoster->postQuiz(qf);
    });
    connect(scrSetup, &wSetup::selectQuiz, this, [&](int quizId) {
        hoster->setQuizId(quizId);
    });
    connect(scrSetup, &wSetup::requestCode, this, [&]() {
        hoster->setupQuiz();
    });
    // connect(scrSetup, &wSetup::codeAssigned, this, [&]() {
    //     user->setQuizCode(code);
    //     stack->setCurrentWidget(scrPanel);
    // });
    connect(scrSetup, &wSetup::launchQuiz, this, [&]() {
        hoster->launchQuiz();
        stack->setCurrentWidget(scrPanel);
    });

    connect(scrPanel, &wPanel::goNext, this, [&]() {
        goToSummary();
    });

    // UCZESTNIK QUIZU
    connect(scrJoin, &wJoin::joinQuiz, this, [&](QString code) {
        player->joinQuiz(code.toStdString());
        stack->setCurrentWidget(scrNick);
    });

    connect(scrNick, &wNick::proposeNickname, this, [&](QString nick) {
        player->proposeNickname(nick.toStdString());
        stack->setCurrentWidget(scrWait);
    });
    // connect(&netClient, &NetworkClient::nicknameAccepted, this, [&]() {
    //     stack->setCurrentWidget(scrWait);
    // });
    // connect(&netClient, &NetworkClient::nicknameRejected, this, [&](QString reason) {
    //     return;
    // });

    connect(scrWait, &wWaiting::goNext, this, [&](){ stack->setCurrentWidget(scrQuestion); });
    connect(scrQuestion, &wQuestion::goNext, this, [&]() {
        goToSummary();
    });
    connect(scrQuestion, &wQuestion::answer, this, [&](int answerId){
        player->answer(currQuestionId, answerId);
    });

    connect(&netClient, &NetworkClient::fileReceived, this, [&](const std::string &type, const std::string &content) {
        QString text = QString::fromStdString(content);

        if (type == "LIST") {
            if (hoster) {
                scrSetup->showList(text);
                return;
            }
        }
        if (type == "RANK") {
            if (stack->currentWidget() == scrSummary) scrSummary->showRank(text);
            if (hoster && stack->currentWidget() == scrPanel) scrPanel->showRank(text);
        }
    });


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
