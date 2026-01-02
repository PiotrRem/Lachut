#ifndef USER_H
#define USER_H

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

    virtual void exit();
    virtual void getRanking() = 0;
};


class QuizHoster : public User {
private:
    std::string quizName = "";
    std::string quizCode = "";
public:
    void setQuizCode(const std::string &qc) { quizCode = qc; }
    void setQuizName(const std::string &name) { quizName = name; }

    void listQuizzes();
    void postQuiz(const QuizFile &qf);
    void setupQuiz();
    void launchQuiz();
    void checkQuizStatus();
    void getRanking() override;
};


class QuizPlayer : public User {
public:
    void joinQuiz(const std::string &code);
    void proposeNickname(const std::string &nick);
    void answer(int questionId, int answerId);
    void getOwnScore();
    void getRanking() override;
};

#endif // USER_H