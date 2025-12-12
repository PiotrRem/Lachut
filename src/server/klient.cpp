#include "klient.h"

Klient::Klient(int fd){
    this->fd = fd;
}

bool Klient::obsluzZdarzenie(){
    std::cout<< "Obsluga poloczenia "<<fd<<std::endl;

    char buf[RBUFSIZE];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if(n < 1) return false;
    dodajDane(buf, n);

    if(czyWysylaPlik) pobierajPlik();
    else odczytajKomende();
    return true;
}

void Klient::dodajDane(const char* dane, size_t dlu){
    rbuf.append(dane, dlu);
}

void Klient::odczytajKomende(){
    while (true) {
        size_t pos = rbuf.find('\n');
        if (pos == std::string::npos) return;

        std::string line = rbuf.substr(0, pos);
        std::cout<<"odczytano: " << line << std::endl;
        rbuf.erase(0, pos + 1);

        obsluzKomende(line);
    }
}

void Klient::obsluzKomende(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "JOIN") {
        int id;
        iss >> id;
        std::cout<<"JOIN "<< id << std::endl;
        //join_quiz(k, id);
    }

    else if (cmd == "NICK") {
        std::string nick;
        iss >> nick;
        std::cout<<"NICK "<< nick << std::endl;
        //set_nick(k, nick);
    }

    else if (cmd == "LIST") {
        std::cout<<"LIST"<< std::endl;
        //send_quiz_list(k);
    }

    else if (cmd == "START") {
        int id;
        iss >> id;
        std::cout<<"START "<< id << std::endl;
        //start_quiz(k, id);
    }

    else if (cmd == "ANSWER") {
        int nr, nr_pytania;
        std::vector<int> odp;
        iss >> nr_pytania;
        while (iss >> nr) odp.push_back(nr);
        std::cout<<"ANSWER to "<< nr_pytania << " is: ";
        for(auto it : odp) std::cout<< it << " ";
        std::cout<<std::endl;
        //answer_question(k, odp);
    }

    else if (cmd == "POST") {
        size_t len;
        iss >> len;
        std::cout<<"POST "<< len << std::endl;
        zacznijPobieracPlik(len);
    }
}

void Klient::zacznijPobieracPlik(size_t len) {
    czyWysylaPlik = true;
    dlugoscPliku = len;
    plik.clear();
}

void Klient::pobierajPlik() {
    size_t need = dlugoscPliku - plik.size();
    size_t take = std::min(need, rbuf.size());

    plik.append(rbuf.data(), take);
    rbuf.erase(0, take);

    if (plik.size() < dlugoscPliku) return;

    zakonczPobieraniePliku();
    
    czyWysylaPlik = false;
    dlugoscPliku = 0;
    plik.clear();
}

void Klient::zakonczPobieraniePliku(){
    std::cout << "Plik został odczytany w całości!\n";
    std::cout<<"------------------------------------\n";
    std::cout<<plik << std::endl;
    std::cout<<"------------------------------------\n";
}