#include "user.h"

// UŻYTKOWNIK

void User::exit() {
    netClient->queueMessage("EXIT\n");
}

// TWÓRCA QUIZU : UŻYTKOWNIK

void QuizHoster::postQuiz(const QuizFile &qf) {
    netClient->queueFile(qf.path, qf.size);
}

void QuizHoster::listQuizzes() {
    netClient->queueMessage("LIST\n");
}

void QuizHoster::setupQuiz() {
    netClient->queueMessage("SETUP " + quizName + "\n");
}

void QuizHoster::launchQuiz() {
    netClient->queueMessage("LAUNCH " + quizCode + "\n");
}

void QuizHoster::getRanking() {
    netClient->queueMessage("GETRANK\n");
}

void QuizHoster::checkQuizStatus() {
    netClient->queueMessage("STATUS\n");
}

// UCZESTNIK : UŻYTKOWNIK          

void QuizPlayer::joinQuiz(const std::string &code) {
    netClient->queueMessage("JOIN " + code + "\n");
}

void QuizPlayer::proposeNickname(const std::string &nick) {
    netClient->queueMessage("NICK " + nick + "\n");
}

void QuizPlayer::answer(int questionId, int answerId) {
    std::string msg = "ANSWER " +
        std::to_string(questionId) + " " +
        std::to_string(answerId) + "\n";

    netClient->queueMessage(msg);
}

void QuizPlayer::getOwnScore() {
    netClient->queueMessage("MYSCORE\n");
}

void QuizPlayer::getRanking() {
    netClient->queueMessage("GETRANK\n");
}
