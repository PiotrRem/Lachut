#include<cstdlib>
#include<cstdio>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<signal.h>
#include<error.h>
#include<errno.h>
#include<fcntl.h>
#include<map>

#include "quiz.h"
#include "klient.h"

void ctrl_c(int);

class Serwer{
protected:
    int sfd;
    int efd;
    std::map<int, Klient> klienci;
    
    void setReuseAddr(int sock){
        const int one = 1;
        int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        if(res) error(1, errno, "setsockopt nie powiodło się");
    }
    inline void ustawNieblokujace(int fd){
        if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)==-1) error(1, errno, "fcntl nie powiodło się");
    }
    uint16_t czytajPort(char* tekst){
        char * ptr;
        auto port = strtol(tekst, &ptr, 10);
        if(*ptr!=0 || port<1 || (port>((1<<16)-1))) error(1,0,"Niewłaściwy argument %s", tekst);
        return port;
    }
    void usun_klienta(int fd){
        std::cout << "Rozłączanie klienta fd=" << fd << std::endl;
        klienci[fd].usun();
        epoll_ctl(efd, EPOLL_CTL_DEL, fd, nullptr);
        shutdown(fd, SHUT_RDWR);
        close(fd);
        klienci.erase(fd);
    }

public:
    Serwer(char* portStr){
        auto port = czytajPort(portStr);
        sfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sfd==-1) error(1, errno, "socket nie powiodło się");
        setReuseAddr(sfd);
        sockaddr_in localAddr={};
        localAddr.sin_addr.s_addr = INADDR_ANY;
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons((short) port);
        if(bind(sfd, (sockaddr*) &localAddr, sizeof(localAddr))==-1) error(1, errno, "bind nie powiodło się");
        if(listen(sfd, 1)==-1) error(1, errno, "listen nie powiodło się");
        efd = epoll_create1(0);
        if(efd==-1) error(1, errno, "epoll_create1 nie powiodło się");
        epoll_event ev;
        ev.data.fd = sfd;
        ev.events = EPOLLIN;
        if(epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &ev)==-1){
            error(1, errno, "epoll_ctl nie powiodło się");
            ctrl_c(SIGINT);
        }
    }
    void wylaczSerwer(){
        for(auto it : klienci) usun_klienta(it.first);
    }

    void obsluguj(){
        epoll_event ev;
        while(1){
            if(epoll_wait(efd, &ev, 1, -1)==-1) error(1, errno, "epoll_wait nie powiodło się");

            while(true){
                int doRozlaczenia = MenegerQuizow::the().getKlientDoRozlaczenia();
                if(doRozlaczenia==-1) break;
                usun_klienta(doRozlaczenia);
            }
            
            if(ev.data.fd==sfd){
                sockaddr_in remoteAddr={};
                socklen_t remoteAddrlen = sizeof(remoteAddr);

                int cfd = accept(sfd, (sockaddr*) &remoteAddr, &remoteAddrlen);
                if(cfd==-1) continue;

                epoll_event client_ev;
                client_ev.events = EPOLLIN | EPOLLRDHUP;
                client_ev.data.fd = cfd;
                if (epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &client_ev) == -1) continue;

                klienci.emplace(cfd, Klient(cfd));
                printf("Nowe połączenia od %s:%d fd = %d\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port), cfd);
                continue;
            }
            
            if(ev.events & EPOLLIN){
                if(!klienci[ev.data.fd].obsluzZdarzenie()) usun_klienta(ev.data.fd);
            }

            else if(ev.events & EPOLLRDHUP){
                usun_klienta(ev.data.fd);
            }
        }
    }
};

Serwer* serwer;
void ctrl_c(int){
    printf("Wyłączanie serwera\n");
    delete serwer;
    exit(0);
}

int main(int argc, char** argv){
    if(argc!=2) error(1, 0, "Podaj 1 argument - numer portu");
    signal(SIGINT, ctrl_c);

    printf("Serwer Lachut: start\n");
    serwer = new Serwer(argv[1]);
    serwer->obsluguj();
}