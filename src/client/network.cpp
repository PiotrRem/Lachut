#include "common.h"
#include "network.h"

#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <poll.h>
#include <error.h>
#include <errno.h>

NetworkClient::~NetworkClient() {
    if (sfd != -1)
        close(sfd);
}

void NetworkClient::connectToServer(const std::string &host, const std::string &port) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *resolved;

    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &resolved)) {
        error(1, errno, "Nie udało się rozwiązać nazwy");
    }

    for (addrinfo *r = resolved; r; r = r->ai_next) {
        sfd = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
        if (sfd == -1) continue;

        if (fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK) == -1) continue;

        int fail = connect(sfd, r->ai_addr, r->ai_addrlen);
        if (!fail || errno == EINPROGRESS) break;

        close(sfd);
        sfd = -1;
    }

    freeaddrinfo(resolved);

    connecting = true;
    connected = false;
    hasError = false;
    lastError.clear();
}

void NetworkClient::queueMessage(const std::string &msg) {
    outQueue.push_back({msg, 0});
}

void NetworkClient::queueFile(QuizFile qf) {
    std::string msg;
    msg.reserve(5 + 6 + 1 + qf.size); // POST 99999\n
    msg = "POST " + std::to_string(qf.size) + '\n';
    
    std::string payload;
    payload.resize(qf.size);
    int fd = open(qf.path.c_str(), O_RDONLY);
    read(fd, payload.data(), qf.size);
    close(fd);

    msg += payload;

    outQueue.push_back({msg, 0});
}

bool NetworkClient::hasMessage() {
    return !inQueue.empty();
}

std::string NetworkClient::popMessage() {
    std::string msg = inQueue.front();
    inQueue.pop_front();
    return msg;
}

void NetworkClient::handleWrite() {
    if (!connected) return;

    while (!outQueue.empty()) {
        OutMsg &m = outQueue.front();

        int n = send(sfd, m.data.data() + m.sent, m.data.size() - m.sent, 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;

            fail(strerror(errno));
            return;
        }

        m.sent += n;
        if (m.sent == m.data.size())
            outQueue.pop_front();
    }
}

void NetworkClient::handleRead() {
    if (!connected) return;

    char buf[4096];
    while (true) {
        int n = recv(sfd, buf, sizeof(buf), 0);
        if (n == 0) {
            fail("Serwer zamknął połączenie");
            return;
        }
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;

            fail(strerror(errno));
            return;
        }

        inBuffer.append(buf, n);

        while (!receivingFile) {
            int pos = inBuffer.find('\n');
            if (pos == -1)
                break;

            std::string line = inBuffer.substr(0, pos);
            inBuffer.erase(0, pos + 1);

            if (line.rfind("POST ", 0) == 0) {
                expectedPayload = std::stoi(line.substr(5));
                receivingFile = true;
                break;
            }

            inQueue.push_back(line);
        }

        if (receivingFile) {
            if (inBuffer.size() >= expectedPayload) {
                std::string fileData = inBuffer.substr(0, expectedPayload);
                inQueue.push_back(fileData);
                inBuffer.erase(0, expectedPayload);

                receivingFile = false;
                expectedPayload = 0;
            }
        }
    }
}

void NetworkClient::pollEvents() {    
    if (sfd < 0)
        return;

    struct pollfd pfd;
    pfd.fd = sfd;
    pfd.events = POLLIN | POLLOUT;

    int ready = poll(&pfd, 1, 0);
    if (ready <= 0)
        return;

    if (connecting && (pfd.revents & POLLOUT)) {
        int err = 0;
        socklen_t len = sizeof(err);
        getsockopt(sfd, SOL_SOCKET, SO_ERROR, &err, &len);

        if (err == 0) {
            connected = true;
            connecting = false;
        } else {
            fail(strerror(err));
            return;
        }
        return;
    }

    if (pfd.revents & POLLIN)
        handleRead();

    if (connected && pfd.revents & POLLOUT)
        handleWrite();

    if (pfd.revents & (POLLERR | POLLHUP)) {
        fail("Połączenie zerwane");
        return;
    }
}

void NetworkClient::fail(const std::string &err) {
    connected = false;
    connecting = false;
    hasError = true;
    lastError = err;
    
    if (sfd != -1) {
        close(sfd);
        sfd = -1;
    }
}

