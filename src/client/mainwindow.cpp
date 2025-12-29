#include "mainwindow.h"

void MainWindow::goToSummary() {
    stack->setCurrentWidget(scrSummary);
    user->getRanking();
}

void MainWindow::handleFile(const std::string &type, const std::string &content) {
    QString text = QString::fromStdString(content);

    if (type == "LIST") {
        scrSetup->showList(text);
        return;
    }
    if (type == "RANK") {
        if (stack->currentWidget() == scrSummary) scrSummary->showRank(text);
        if (hoster && stack->currentWidget() == scrPanel) scrPanel->showRank(text);
    }
}

void MainWindow::handleMsg(const std::string &type, const std::string &content) {
    std::cout << type << "[" << content << "]\n";

    QString qContent = QString::fromStdString(content);

    if (type == "FAIL") {
        QMessageBox::warning(this, "Błąd", qContent);
        return;
    }

    if (type == "OK") {
        if (qContent == "JOIN") {
            stack->setCurrentWidget(scrNick);
            return;
        }
        if (qContent == "NICK") {
            stack->setCurrentWidget(scrWait);
            return;
        }
        if (qContent == "LAUNCH") {
            stack->setCurrentWidget(scrPanel);
            return;
        }
    }

    if (type == "YOURID") {
        scrSetup->setCodeLabel(qContent);
        return;
    }

    if (type == "YOURRANK") {
        scrQuestion->setScoreLabel(qContent);
    }

    if (type == "QUESTION") {
        std::string s = content;
        std::stringstream ss(s);

        int nr = -1;
        ss >> nr;
        currQuestionId = nr;

        if (nr == -1) {
            stack->setCurrentWidget(scrSummary);
            return;
        }

        std::string content;
        std::getline(ss, content);
        if (!content.empty() && content[0] == ' ') content.erase(0, 1);

        stack->setCurrentWidget(scrQuestion);
        scrQuestion->loadQuestion(QString::fromStdString(content));
        return;
    }
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
    connect(scrSetup, &wSetup::selectQuiz, this, [&](QString name) {
        hoster->setQuizName(name.toStdString());
    });
    connect(scrSetup, &wSetup::requestCode, this, [&]() {
        hoster->setupQuiz();
    });
    connect(scrSetup, &wSetup::launchQuiz, this, [&]() {
        hoster->launchQuiz();
    });

    // UCZESTNIK QUIZU
    connect(scrJoin, &wJoin::joinQuiz, this, [&](QString code) {
        player->joinQuiz(code.toStdString());
    });

    connect(scrNick, &wNick::proposeNickname, this, [&](QString nick) {
        player->proposeNickname(nick.toStdString());
    });

    connect(scrQuestion, &wQuestion::goNext, this, [&]() {
        goToSummary();
    });
    connect(scrQuestion, &wQuestion::answer, this, [&](int answerId){
        player->answer(currQuestionId, answerId);
    });

    connect(&netClient, &NetworkClient::fileReceived, this, [&](const std::string &type, const std::string &content) {
        handleFile(type, content);
    });

    connect(&netClient, &NetworkClient::msgReceived, this, [&]() {
        while (netClient.hasMessage()) {
            std::string line = netClient.popMessage();

            size_t pos = line.find(' ');

            std::string type = (pos == std::string::npos) ? line : line.substr(0, pos);
            std::string content = (pos == std::string::npos) ? "" : line.substr(pos + 1);

            if (!content.empty() && content[0] == ' ')
                content.erase(0, 1);

            handleMsg(type, content);
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
