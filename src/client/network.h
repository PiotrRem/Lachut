#pragma once
#include <string>
#include <deque>
#include <vector>
#include <cstdint>
#include <atomic>

struct QuizFile;

class NetworkClient {
private:
    int sfd = -1;

    struct OutMsg {
        std::string data;
        std::size_t sent = 0;
    };

    std::deque<OutMsg> outQueue;

    std::string inBuffer;
    std::deque<std::string> inQueue;
    
    bool receivingFile = false;
    uint16_t expectedPayload = 0;

    void handleRead();
    void handleWrite();
    void fail(const std::string &err);
public:
    bool connecting = false;
    bool connected = false;
    bool hasError = false;
    std::string lastError;

    NetworkClient() = default;
    ~NetworkClient();

    void connectToServer(const std::string &host, const std::string &port);
    void queueMessage(const std::string &msg);
    void queueFile(QuizFile qf);
    bool hasMessage();
    std::string popMessage();
    void pollEvents();
};
