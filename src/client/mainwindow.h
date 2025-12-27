#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>
#include <QFile>

#include "gui.h"
#include "network.h"
#include "user.h"
#include "common.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void goToSummary();
private:
    QStackedWidget *stack;

    wConnect  *scrConnect;
    wRole     *scrRole;
    wNick     *scrNick;
    wSetup    *scrSetup;
    wPanel    *scrPanel;
    wWaiting  *scrWait;
    wQuestion *scrQuestion;
    wSummary  *scrSummary;
    wJoin     *scrJoin;

    NetworkClient netClient;
    User *user = nullptr;
    QuizHoster *hoster = nullptr;
    QuizPlayer *player = nullptr;

    QTimer pollTimer;
    bool established = false;

    QuizFile qf;
    QString assignedQuizCode;
    QuizInfo selectedQuizInfo;

    int currQuestionId = 0;
};

#endif
