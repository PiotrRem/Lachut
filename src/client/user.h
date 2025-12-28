#pragma once
#include <string>
#include <vector>

#include "network.h"
#include "common.h"


class User {
protected:
    NetworkClient *netClient = nullptr;
public:
    virtual ~User() = default;
    void setNetClient(NetworkClient &nc) { netClient = &nc; }

    virtual int exit();
    virtual int getRanking() = 0;
};


class QuizHoster : public User {
private:
    std::string quizName = "";
    std::string quizCode = "";
public:
    void setQuizCode(const std::string &qc) { quizCode = qc; }
    void setQuizName(const std::string &name) { quizName = name; }

    int listQuizzes();
    int postQuiz(const QuizFile &qf);
    int setupQuiz();
    int launchQuiz();
    int checkQuizStatus();
    int getRanking() override;
};


class QuizPlayer : public User {
public:
    int joinQuiz(const std::string &code);
    int proposeNickname(const std::string &nick);
    int answer(int questionId, int answerId);
    int getOwnScore();
    int getRanking() override;
    int handleResponse(const std::string &msg);
};
