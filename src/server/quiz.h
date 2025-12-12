#ifndef QUIZ_H
#define QUIZ_H

#include<string>
#include<vector>
#include<fstream>
#include<iostream>

class Pytanie{
    protected:
        std::string tresc; // treść pytania
        std::vector<std::pair<std::string, bool>> odpowiedzi; // wektor treści sugerowanych odpowiedzi z informacją, czy jest ona poprawna
        unsigned int limitCzasu = 0; // limit czasu w sekundach

    public:
        Pytanie();
        void setTresc(std::string tresc);
        void setLimitCzasu(unsigned int limit);
        void dodajOdpowiedz(std::pair<std::string, bool> odpowiedz);
        void wypisz();
        std::vector<std::pair<std::string, bool>> getOdpowiedzi();
        std::string getTresc();
        unsigned int getLimitCzasu();
};

class Quiz{
    protected:
        std::string nazwa; // nazwa - unikalna nazwa quizu
        std::vector<Pytanie> pytania; // pytania - wektor pytań w quizie

    public:
        Quiz();
        void setNazwa(std::string nazwa);
        void dodajPytanie(Pytanie pytanie);
        void wypisz();

        std::string getNazwa();
        bool waliduj();
};

class BazaQuizow{
    protected:
        std::vector<Quiz> bazaQuizow;
    
    public:
        BazaQuizow();
        bool dodajQuiz(std::string nazwaPliku);
        void wypiszTrescQuizu(int id);
};

class ParserQuizu{
    protected:
        std::ifstream f;
        Quiz* quiz;

        std::string wczytajPomiedzy2(char znak);
        std::string wczytajDo(char znak);
        bool wczytajPytania();

    public:
        ParserQuizu(std::string nazwaPliku, Quiz* quiz);
        bool parsujQuiz();
};

#endif // QUIZ_H