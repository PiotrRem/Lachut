#include <cstdio>
#include <cstring>
#include <vector>
#include <atomic>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>

// komunikacja sieciowa
#define HEADER_SIZE 4
#define CMD_SIZE 7
#define MAX_MSG_SIZE ((1 << 16) - HEADER_SIZE)
// quiz
#define QID_SIZE 5
#define QCODE_SIZE 7
#define QNAME_SIZE 64
// gracze
#define NICK_SIZE 16

int sfd;
std::atomic<bool> running{false};

struct QuizInfo {
    char id[QID_SIZE];
    char name[QNAME_SIZE];
};

struct QuizQuestion {

};

// do tego kolejka ale jeszcze nwm
struct inBuffer {
    char content[MAX_MSG_SIZE];
    uint16_t size = 0;
    int pos = 0;
    bool done = true;
};

struct outBuffer {
    char content[MAX_MSG_SIZE];
    uint16_t size = 0;
    int pos = 0;
    bool done = true;
};


void setNonBlocking(int sfd) {
    if (fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK) == -1) {
        error(1, errno, "Ustawienie gniazda nieblokującego nie powiodło się.");
    }
}


class User {
public:
    User() = default;
    virtual ~User() = default;

    char* getQuizCode() { return quizCode; }
    void setQuizCode(const char code[QCODE_SIZE]) { 
        memcpy(this->quizCode, code, QCODE_SIZE);
    }
protected:
    char quizCode[QCODE_SIZE] = "000000";

    ssize_t sendAll(int sfd, const void *buf, ssize_t len) {
        uint8_t *ptr = (uint8_t*) buf;
        ssize_t sent = 0;

        while (sent < len) {
            ssize_t n = send(sfd, ptr + sent, len - sent, 0);
            if (n <= 0) return -1;
            sent += n;
        }
        return sent;
    }

    ssize_t recvAll(int sfd, void *buf, uint32_t len) {
        uint8_t *ptr = (uint8_t*) buf;
        ssize_t recd = 0;

        while (recd < len) {
            ssize_t n = recv(sfd, ptr + recd, len - recd, 0);
            if (n <= 0) return -1;
            recd += n;
        }
        return recd;
    }

    ssize_t sendMessage(int sfd, char *msg, uint32_t msgLen) {
        uint32_t len = htonl(msgLen);
        char buf[HEADER_SIZE + msgLen];

        memcpy(buf, &len, HEADER_SIZE);
        memcpy(buf + HEADER_SIZE, msg, msgLen);

        return sendAll(sfd, buf, HEADER_SIZE + msgLen);
    }

    ssize_t recvMessage(int sfd, char *buf, uint32_t bufSize) {
        uint32_t len;
        if (recvAll(sfd, &len, HEADER_SIZE) <= 0) {
            return -1;
        }

        uint32_t msgLen = ntohl(len);
        if (msgLen > bufSize) {
            return -2;
        }

        return recvAll(sfd, buf, msgLen);
    }

    int constructCommand(char *out, size_t outSize, char cmd[CMD_SIZE], int argc, char **argv) {
        memcpy(out, cmd, CMD_SIZE - 1);
        size_t pos = CMD_SIZE - 1;
        if (argc > 0) {
            if (pos + 1 >= outSize) return -1;
            out[pos++] = ' ';
        }

        for (int i = 0; i < argc; i++) {
            size_t argLen = strlen(argv[i]);

            if (pos + argLen >= outSize) return -1;
            memcpy(out + pos, argv[i], argLen);
            pos += argLen;

            if (i + 1 < argc) {
                if (pos + 1 >= outSize) return -1;
                out[pos++] = ' ';
            }
        }
        
        if (pos + 1 >= outSize) return -1;
        out[pos++] = '\n';
        return pos;
    }

    std::vector<QuizInfo> listQuizzes() {
        return {};
    }
};


class QuizHoster : public User {
private:
    QuizInfo quizInfo;
public:
    void setQuizInfo(char id[QID_SIZE], char name[QNAME_SIZE]) {
        memcpy(quizInfo.id, id, QID_SIZE);
        memcpy(quizInfo.name, name, QNAME_SIZE);
    }

    int launchQuiz() {
        char cmd[CMD_SIZE] = "LAUNCH";
        char msg[MAX_MSG_SIZE];
        char *args[] = {this->quizCode, nullptr};
        int len = constructCommand(msg, MAX_MSG_SIZE, cmd, 1, args);
        return sendMessage(sfd, msg, len);
    }

    int getQuizStatus() {
        char cmd[CMD_SIZE] = "STATUS";
        char msg[MAX_MSG_SIZE];
        char *args[] = {this->quizCode, nullptr};
        int len = constructCommand(msg, MAX_MSG_SIZE, cmd, 1, args);
        return sendMessage(sfd, msg, len);
    }
};


class QuizUser : public User {
public:
    int proposeNickname(char nickname[NICK_SIZE]) {
        char cmd[CMD_SIZE] = "NICK  ";
        char msg[MAX_MSG_SIZE];
        char *args[] = {nickname, nullptr};
        int len = constructCommand(msg, MAX_MSG_SIZE, cmd, 1, args);
        return sendMessage(sfd, msg, len);
    }

    int joinQuiz() {
        char cmd[CMD_SIZE] = "JOIN  ";
        char msg[MAX_MSG_SIZE];
        char *args[] = {this->quizCode, nullptr};
        int len = constructCommand(msg, MAX_MSG_SIZE, cmd, 1, args);
        return sendMessage(sfd, msg, len);
    }

    int answer(char qcode[QCODE_SIZE], struct QuizQuestion) {
        return 0;
    }

    int update() {
        return 0;
    }
};

void ctrl_c(int) {
    running = false;
}

void connectToHost(const char *host, const char *port) {
    addrinfo hints {};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    addrinfo *resolved;
    if (getaddrinfo(host, port, &hints, &resolved)) {
        error(1, errno, "Nie udało się rozwiązać nazwy");
    }

    sfd = socket(resolved->ai_family, resolved->ai_socktype, resolved->ai_protocol);
    if (sfd == -1) {
        error(1, errno, "Nie udało się stworzyć gniazda");
    }
    if (connect(sfd, resolved->ai_addr, resolved->ai_addrlen)) {
        error(1, errno, "Nie udało się połączyć na wskazanego hosta");
    }
    freeaddrinfo(resolved);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        error(1, 0, "Podaj adres oraz port");
    }

    signal(SIGINT, ctrl_c);
    connectToHost(argv[1], argv[2]);

    QuizHoster *h = new QuizHoster();
    h->setQuizCode("123456");
    h->launchQuiz();
    h->getQuizStatus();
    delete h;

    printf("\n");

    QuizUser *u = new QuizUser();
    u->setQuizCode("654321");
    char nickname[NICK_SIZE] = "test";
    u->proposeNickname(nickname);
    u->joinQuiz();
    delete u;

    shutdown(sfd, SHUT_RDWR);
    close(sfd);
}