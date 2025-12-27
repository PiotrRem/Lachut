#include "user.h"


// UŻYTKOWNIK

int User::exit() {
    netClient->queueMessage("EXIT\n");
    return 0;
}


// TWÓRCA QUIZU : UŻYTKOWNIK

int QuizHoster::postQuiz(const QuizFile &qf) {
    netClient->queueFile(qf.path, qf.size);
    return 0;
}

int QuizHoster::listQuizzes() {
    netClient->queueMessage("LIST\n");
    return 0;
}

int QuizHoster::setupQuiz() {
    netClient->queueMessage("SETUP " + std::to_string(quizId) + "\n");
    return 0;
}

int QuizHoster::launchQuiz() {
    netClient->queueMessage("LAUNCH " + quizCode + "\n");
    return 0;
}

int QuizHoster::getRanking() {
    netClient->queueMessage("GETRANK\n");
    return 0;
}

int QuizHoster::checkQuizStatus() {
    netClient->queueMessage("STATUS\n");
    return 0;
}


// UCZESTNIK : UŻYTKOWNIK          

int QuizPlayer::joinQuiz(const std::string &code) {
    netClient->queueMessage("JOIN " + code + "\n");
    return 0;
}

int QuizPlayer::proposeNickname(const std::string &nick) {
    netClient->queueMessage("NICK " + nick + "\n");
    return 0;
}

int QuizPlayer::answer(int questionId, int answerId) {
    std::string msg = "ANSWER " +
        std::to_string(questionId) + " " +
        std::to_string(answerId) + "\n";

    netClient->queueMessage(msg);
    return 0;
}

int QuizPlayer::getOwnScore() {
    netClient->queueMessage("GETRANK\n");
    return 0;
}

int QuizPlayer::getRanking() {
    netClient->queueMessage("GETRANK\n");
    return 0;
}
