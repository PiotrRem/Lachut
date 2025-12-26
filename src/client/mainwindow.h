#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>

#include "gui.h"
#include "network.h"
#include "user.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

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

    NetworkClient netClient;
    User* user = nullptr;

    QTimer pollTimer;
    bool established = false;
};

#endif
