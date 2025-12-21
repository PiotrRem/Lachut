#include "user.h"


// UŻYTKOWNIK

int User::connect(const std::string &host, const std::string &port) {
    netClient->connectToServer(host, port);
    return 0;
}

int User::exit() {
    netClient->queueMessage("EXIT\n");
    return 0;
}


// TWÓRCA QUIZU : UŻYTKOWNIK

int QuizHoster::postQuiz() {
    netClient->queueFile(quizFile);
    return 0;
}

int QuizHoster::listQuizzes() {
    netClient->queueMessage("LIST\n");
    return 0;
}

int QuizHoster::setupQuiz() {
    netClient->queueMessage("SETUP " + std::to_string(currQuiz.quizId) + '\n');
    return 0;
}

int QuizHoster::startQuiz() {
    netClient->queueMessage("LAUNCH " + quizCode + '\n');
    return 0;
}

int QuizHoster::getRanking() {
    netClient->queueMessage("GETRANK\n");
    return 0;
}

int QuizHoster::skipQuestion() {
    netClient->queueMessage("SKIP\n");
    return 0;
}

int QuizHoster::checkQuizStatus() {
    netClient->queueMessage("STATUS\n");
    return 0;
}


// UCZESTNIK : UŻYTKOWNIK          

int QuizUser::joinQuiz() {
    netClient->queueMessage("JOIN " + std::to_string(currQuiz.quizId) + '\n');
    return 0;
}

int QuizUser::proposeNickname(const std::string &nickname) {
    netClient->queueMessage("NICK " + nickname + '\n');
    return 0;
}

int QuizUser::answer(const QuizAnswer &ans) {
    std::string msg = "ANSWER " +
        std::to_string(ans.questionId) + " " +
        std::to_string(ans.answerId) + '\n';

    netClient->queueMessage(msg);
    return 0;
}

int QuizUser::getOwnScore() {
    netClient->queueMessage("GETRANK\n");
    return 0;
}

int QuizUser::getRanking() {
    netClient->queueMessage("GETRANK\n");
    return 0;
}
