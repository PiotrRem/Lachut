#ifndef UZYTKOWNIK_H
#define UZYTKOWNIK_H

#include<string>
#include<mutex>
#include<ctime>
#include<set>
#include<thread>
#include<chrono>
#include<sys/socket.h>
#include<unistd.h>

#include "quiz.h"
class InstancjaQuizu;
class Uzytkownik;
class TworcaQuizu;
class UczestnikQuizu;

class Uzytkownik{
    protected:
        std::string nick;
        int fd;
    public:
        Uzytkownik(){}
        Uzytkownik(int fd);
        void setNick(std::string nick);
        std::string getNick();
        int getfd();
        bool wyslijWiadomosc(std::string wiadomosc);
};
    
class TworcaQuizu : Uzytkownik{
    protected:

    public:
        TworcaQuizu(){}
        TworcaQuizu(int fd){this->fd=fd;};
        void setNick(std::string nick){return Uzytkownik::setNick(nick);}
        std::string getNick(){return Uzytkownik::getNick();}
        inline int getfd(){return Uzytkownik::getfd();};
        bool wyslijWiadomosc(std::string wiadomosc){return Uzytkownik::wyslijWiadomosc(wiadomosc);};
};

class UczestnikQuizu : Uzytkownik{
    protected:

    public:
        UczestnikQuizu(){};
        UczestnikQuizu(int fd){this->fd=fd;};
        void setNick(std::string nick){return Uzytkownik::setNick(nick);}
        std::string getNick(){return Uzytkownik::getNick();}
        inline int getfd(){return Uzytkownik::getfd();};
        bool wyslijWiadomosc(std::string wiadomosc){return Uzytkownik::wyslijWiadomosc(wiadomosc);};
};

class InstancjaQuizu{
    protected:
        Quiz quiz;
        std::vector<std::pair<UczestnikQuizu, unsigned int>> uczestnicy; // uczestnik : wynik
        TworcaQuizu tworcaQuizu;
        bool czyTworcaOpuscil = false;

        enum Stan : char {CZEKAJACY, TRWAJACY, ZAKONCZONY};
        Stan stan = Stan::CZEKAJACY;
        unsigned int id;

        volatile unsigned int biezacePytanie = 0;
        time_t poczBiezacegoPytania;
        std::set<int> ktoOdpowiedzial;

        bool kolejnePytanie(bool czyPoczatkowe);
        void przewinPytanie(unsigned int wystartowanoDlaPytania);
    public:
        InstancjaQuizu(){}
        InstancjaQuizu(Quiz quiz, int fdTworcy, unsigned int id);
        bool dodajUczestnika(int fdUczestnika);
        bool wystartuj(int fd);
        void zakoncz();
        bool usun(int fd);
        void wyslijRanking(int fd);
        bool wyslijPytanie(int fd);
        bool wyslijStatus(int fd);

        bool zarejestrujOdpowiedz(int fd, unsigned int nrPytania, std::set<unsigned int> odpowiedzi);
        std::string getRanking(int fd);
        std::string getRanking();
        std::string pobierzPytanieZOdpowiedziami();
        bool setNick(int fd, std::string nick);
};

class MenegerQuizow{
    private:
        MenegerQuizow();
        ~MenegerQuizow() = default;
        MenegerQuizow(const MenegerQuizow&) = delete;
        MenegerQuizow& operator=(const MenegerQuizow&) = delete;
        std::set<int> klienciDoRozlaczenia;

    protected:
        std::map<unsigned int, InstancjaQuizu> bazaQuizow; // id instancji quizu -> instancja quizu
        volatile unsigned int sequence = 1;
        unsigned int nextval();

    public:
        static MenegerQuizow& the();
        unsigned int dodajInstancjeQuizu(std::string nazwaQuizu, int fdTworcy);
        bool usunInstancjeQuizu(unsigned int id);
        InstancjaQuizu& getInstancjaQuizu(unsigned int id);
        int getKlientDoRozlaczenia();
        void dodajKlientaDoRozlaczenia(int fd);
};

#endif // UZYTKOWNIK_H