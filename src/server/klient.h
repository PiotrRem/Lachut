#ifndef KLIENT_H
#define KLIENT_H

#include<unistd.h>
#include<stdio.h>
#include<string>
#include<iostream>
#include<sstream>
#include<vector>
#include<sys/socket.h>

#define RBUFSIZE 64
#define WBUFSIZE 1024

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

    public:
        Klient(){}
        Klient(int fd);
        bool obsluzZdarzenie();
};

#endif // KLIENT_H