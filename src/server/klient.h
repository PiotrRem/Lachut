#ifndef KLIENT_H
#define KLIENT_H

#include<unistd.h>
#include<stdio.h>
#include<string>
#include<iostream>
#include<sstream>
#include<vector>
#include<sys/socket.h>
#include<fcntl.h>

#define RBUFSIZE 64
#define WBUFSIZE 1024

#include "uzytkownik.h"
#include "quiz.h"

class MenegerPokoi{
    private:
        MenegerPokoi(){};
        ~MenegerPokoi() = default;
        MenegerPokoi(const MenegerPokoi&) = delete;
        MenegerPokoi& operator=(const MenegerPokoi&) = delete;

    protected:
        std::map<int, unsigned int> pokoje; // fd Klienta -> id instancji quizu

    public:
        static MenegerPokoi& the();
        unsigned int znajdzPokoj(int fd);
        bool dodajDoPokoju(int fd, unsigned int idQuizu);
        bool usunPokoj(unsigned int idQuizu);
        bool usunGoscia(int fd);
        std::set<int> znajdzGosci(unsigned int idQuizu);
};

class Klient{
    protected:
        int fd;
        std::string rbuf;
        std::string plik;
        size_t dlugoscPliku = 0;
        bool czyWysylaPlik = false;

        void dodajDane(const char* dane, size_t dlu);
        void odczytajKomende();
        void obsluzKomende(const std::string& line);
        void zacznijPobieracPlik(size_t dlu);
        void pobierajPlik();
        void zakonczPobieraniePliku();
        bool wyslijWiadomosc(std::string wiadomosc);

    public:
        ~Klient();
        Klient(){}
        Klient(int fd);
        bool obsluzZdarzenie();
        void usun();
};

#endif // KLIENT_H