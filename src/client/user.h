#pragma once
#include <string>
#include <vector>

#include "network.h"
#include "common.h"

// hoster raczej nie bedzie widziec tresci pytan, tylko np 6/12 i skip

class User {
protected:
    NetworkClient *netClient = nullptr;
    QuizInfo currQuiz;
public:
    virtual ~User() = default;

    NetworkClient& getNetClient() { return *netClient; }
    QuizInfo&      getQuizInfo() { return currQuiz; }

    void setNetClient(NetworkClient &nc) { netClient = &nc; }
    void setCurrentQuiz(QuizInfo &qi) { currQuiz = qi; }

    int connect(const std::string &host, const std::string &port);
    virtual int exit();
    virtual int proposeNickname(const std::string&) {
        return -1;
    }
};


class QuizHoster : public User {
protected:
    QuizFile quizFile;
    std::string quizCode;
public:
    QuizFile&   getQuizFile() { return quizFile; }
    std::string getQuizCode() { return quizCode; }

    void setQuizFile(QuizFile &qf) { quizFile = qf; }
    void setQuizCode(const std::string &qc) { quizCode = qc; }

    int postQuiz();
    int listQuizzes();
    int setupQuiz();
    int startQuiz();
    int getRanking();
    int skipQuestion();
    int checkQuizStatus(); // aktualny nr pytania itp
};


class QuizUser : public User {
protected:
    QuizQuestion currQuestion;
    QuizAnswer   questAnswer;
public:
    QuizQuestion& getCurrentQuestion() { return currQuestion; }
    QuizAnswer&   getAnswer() { return questAnswer;  }

    void setCurrentQuestion(QuizQuestion &cq) { currQuestion = cq; }
    void setAnswer(QuizAnswer &qa) { questAnswer = qa; }

    int joinQuiz();
    int proposeNickname(const std::string &nickname) override;
    int answer(const QuizAnswer &ans);
    int getOwnScore();
    int getRanking();
};
