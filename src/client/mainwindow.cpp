#include "mainwindow.h"

void MainWindow::goToSummary() {
    expectingExit = true;
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
        scrSummary->showRank(text);
        scrPanel->showRank(text);
    }
}

void MainWindow::handleMsg(const std::string &type, const std::string &content) {
    QString qContent = QString::fromStdString(content);

    if (type == "FAIL") {
        if (qContent == "JOIN") {
            QMessageBox::warning(this, "Błąd", "Nie udało się dołączyć do quizu.");
            return;
        }
        if (qContent == "NICK") {
            QMessageBox::warning(this, "Błąd", "Ten nick jest już zajęty.");
            return;
        }
        if (qContent == "LAUNCH") {
            QMessageBox::warning(this, "Błąd", "Potrzeba przynajmniej jednego gracza.");
            return;
        }
        if (qContent == "ANSWER") {
            QMessageBox::warning(this, "Błąd", "Udzieliłeś już odpowiedzi na to pytanie.");
            return;
        }
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
            if (hoster) hoster->checkQuizStatus();
            return;
        }
        if (qContent == "POST") {
            QMessageBox::information(this, "Sukces", "Quiz został dodany.");
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
        std::istringstream ss(content);

        int num = -1;
        ss >> num;
        currQuestionId = num;
        if (num == -1) {
            goToSummary();
            return;
        }

        std::string question;
        std::getline(ss, question);

        if (hoster) {
            scrPanel->setQuestionNumLabel(QString::number(num + 1));
            hoster->checkQuizStatus();
        } else {
            stack->setCurrentWidget(scrQuestion);
            scrQuestion->loadQuestion(QString::fromStdString(question));
        }

        return;
    }

    if (type == "STATUS") {
        std::istringstream ss(content);
        int num, userCount;
        ss >> num >> userCount;
        currQuestionId = num;

        scrPanel->setUserCount(QString::number(userCount));
        scrPanel->setQuestionNumLabel(QString::number(num + 1));

        return;
    }
}

void MainWindow::cleanup() {
    user = nullptr;
    if (hoster) {
        delete hoster;
        hoster = nullptr;
    }
    if (player) {
        delete player;
        player = nullptr;
    }

    established = false;
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

    // KLIENT SIECIOWY

    connect(&netClient, &NetworkClient::reportError, this, [&](const std::string &err) {        
        if (expectingExit) {
            expectingExit = false;
            QMetaObject::invokeMethod(this, "cleanup", Qt::QueuedConnection);
            return;
        }

        QMessageBox::warning(this, "Błąd", QString::fromStdString(err));
        stack->setCurrentWidget(scrConnect);

        if (!established) return;
        QMetaObject::invokeMethod(this, "cleanup", Qt::QueuedConnection);
    });
    connect(&netClient, &NetworkClient::fileReceived, this, [&](const std::string &type, const std::string &content) {
        handleFile(type, content);
    });
    connect(&netClient, &NetworkClient::msgReceived, this, [&]() {
        while (netClient.hasMessage()) {
            std::string line = netClient.popMessage();

            size_t pos = line.find(' ');
            std::string type =    (pos == std::string::npos) ? line : line.substr(0, pos);
            std::string content = (pos == std::string::npos) ? ""   : line.substr(pos + 1);

            handleMsg(type, content);
        }
    });

    // WSPÓLNE

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
    connect(scrSummary, &wSummary::goNext, this, [&]() {
        stack->setCurrentWidget(scrConnect);
        scrSummary->showRank("");
        scrPanel->showRank("");
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
        scrSetup->setQuizLabel(name);
        scrPanel->setQuizLabel(name);
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
    connect(scrQuestion, &wQuestion::answer, this, [&](int answerId){
        player->answer(currQuestionId, answerId);
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
