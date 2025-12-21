#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct QuizInfo {
    int quizId = 0;
    std::string name;
    int totalQuestions = 0;
};

struct QuizQuestion {
    int quizId = 0;
    int questionId = 0;
    int correctId = 0;
    std::string text;
    std::vector<std::string> answers;
    int timeLimit = 0;
};

struct QuizAnswer {
    int quizId = 0;
    int questionId = 0;
    int answerId = 0;
};

struct QuizFile {
    std::string path;
    uint16_t size = 0;
};
