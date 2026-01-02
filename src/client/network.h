#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <deque>
#include <cstdint>
#include <QObject>


class NetworkClient : public QObject {
    Q_OBJECT
private:
    int sfd = -1;

    struct OutMsg {
        std::string msg;
        std::size_t sent = 0;
    };

    std::deque<OutMsg> outQueue;

    std::string inBuffer;
    std::deque<std::string> inQueue;
    
    bool receivingFile = false;
    std::string fileType;
    uint16_t expectedPayload = 0;

    void handleRead();
    void handleWrite();
    void fail(const std::string &err);
public:
    bool connecting = false;
    bool connected = false;

    NetworkClient() = default;
    ~NetworkClient();

    void connectToServer(const std::string &host, const std::string &port);
    void queueMessage(const std::string &msg);
    void queueFile(const std::string &path, uint16_t size);
    bool hasMessage();
    std::string popMessage();
    void pollEvents();

signals:
    void fileReceived(const std::string &type, const std::string &content);
    void msgReceived();
    void reportError(const std::string &err);
};

#endif // NETWORK_H
