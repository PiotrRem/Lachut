#include "quiz.h"

Pytanie::Pytanie(){}
void Pytanie::setTresc(std::string tresc){this->tresc=tresc;}
void Pytanie::setLimitCzasu(unsigned int limit){this->limitCzasu=limit;}
void Pytanie::dodajOdpowiedz(std::pair<std::string, bool> odpowiedz){odpowiedzi.push_back(odpowiedz);}
std::string Pytanie::getTresc(){return tresc;}
void Pytanie::wypisz(){
    std::cout << tresc << std::endl;
    for(auto it : odpowiedzi){
        std::cout << it.second << ": " << it.first << std::endl;
    }
    std::cout<<"limit: " << limitCzasu << std::endl;
}

std::vector<std::pair<std::string, bool>> Pytanie::getOdpowiedzi(){
    return odpowiedzi;
}

std::string Pytanie::getTrescZOdpowiedziami(){
    std::string wynik = tresc;
    for(auto it : odpowiedzi){
        tresc += " ";
        tresc += it.first;
    }
    return wynik;
}

//-------------------------------------------------------------------------------------------------------------
Quiz::Quiz(){}
void Quiz::setNazwa(std::string nazwa){this->nazwa=nazwa;}
void Quiz::dodajPytanie(Pytanie pytanie){pytania.push_back(pytanie);}
std::string Quiz::getNazwa(){return nazwa;}
unsigned int Pytanie::getLimitCzasu(){return limitCzasu;}
void Quiz::wypisz(){
    for(Pytanie p : pytania){
        p.wypisz();
        printf("\n");
    }
}

bool Quiz::waliduj(){
    if(nazwa=="") return false;
    if(pytania.size()==0) return false;

    for(auto p : pytania){
        if(p.getTresc()=="") return false;
        if(p.getOdpowiedzi().size()==0) return false;
    }
    return true;
}

Pytanie Quiz::getPytanie(unsigned int id){
    if(pytania.size() <= id) return pytania[0];
    return pytania[id];
}

unsigned int Quiz::ilePytan(){
    return pytania.size();
}

BazaQuizow::BazaQuizow(){};
BazaQuizow& BazaQuizow::the() {
    static BazaQuizow instance; // tworzy się raz, wątkowo-bezpieczne (C++11+)
    return instance;
}

bool BazaQuizow::dodajQuiz(std::string nazwaPliku){
    Quiz quiz;
    ParserQuizu parser("quizSource/" + nazwaPliku + ".qcf", &quiz);
    std::cout << "Ładuję quizSource/" << nazwaPliku << ".qcf" << std::endl;
    bool result =  parser.parsujQuiz();
    if(!result) return false;
    if(!quiz.waliduj()) return false;
    bazaQuizow.push_back(quiz);
    return true;
}

Quiz BazaQuizow::getQuiz(std::string nazwa){
    for(auto it : bazaQuizow){
        if(it.getNazwa()==nazwa) return it;
    }
    if(dodajQuiz(nazwa)) for(auto it : bazaQuizow){
        if(it.getNazwa()==nazwa) return it;
    }
    return Quiz();
}

void BazaQuizow::wypiszTrescQuizu(int id){
    bazaQuizow[id].wypisz();
}

std::string BazaQuizow::getListaQuizow(){
    std::vector<std::string> pliki;

    for (const auto& entry : std::filesystem::directory_iterator("quizSource")) {
        if (entry.is_regular_file()) {
            std::string nazwa = entry.path().filename().string().substr(0, entry.path().filename().string().length() - 4);
            pliki.push_back(nazwa);
        }
    }

    std::string out;
    for (const auto& p : pliki) {
        out += p;
        out += '\n';
    }
    return out;
}

ParserQuizu::ParserQuizu(std::string nazwaPliku,  Quiz* quiz){
    f.open(nazwaPliku);
    if(!f) printf("Nie udało się otworzyć pliku z quizem! (%s)\n", nazwaPliku.c_str());
    this->quiz = quiz;
}

bool ParserQuizu::parsujQuiz(){
    quiz->setNazwa(wczytajPomiedzy2('"'));    
    if(wczytajDo('=')!="pytanie") return false;
    wczytajPytania();
    return true;
}

std::string ParserQuizu::wczytajPomiedzy2(char znak){
    std::string odczytane = "";

    char ch;
    bool started = false;
    while(f.get(ch)){
        if(ch==znak){
            if(!started) started = true;
            else break;
            continue;
        }
        if(started) odczytane += ch;
    }
    return odczytane;
}

std::string ParserQuizu::wczytajDo(char znak){
    std::string odczytane = "";

    char ch;
    char started = false;
    while(f.get(ch)){
        if(ch!=' ' && ch!='\t' && ch!='\n') started = true;
        if(ch==znak) break;
        if(started) odczytane += ch;
    }

    for(int i=odczytane.length()-1;i>=0;i--){
        if(odczytane[i]!=' ' && odczytane[i]!='\t' && odczytane[i]!='\n') break;
        odczytane = odczytane.substr(0, odczytane.length()-1);
    }

    return odczytane;
}

bool ParserQuizu::wczytajPytania(){
    Pytanie pytanie;
    pytanie.setTresc(wczytajPomiedzy2('"'));

    std::string slowo, odp;
    unsigned int limit;
    for(;;){
        slowo = wczytajDo('=');
        if(slowo=="P"){
            odp = wczytajPomiedzy2('"');
            pytanie.dodajOdpowiedz(std::pair<std::string, bool>(odp, true));
        }
        else if(slowo=="F"){
            odp = wczytajPomiedzy2('"');
            pytanie.dodajOdpowiedz(std::pair<std::string, bool>(odp, false));
        }
        else if(slowo=="limit"){
            f >> limit;
            pytanie.setLimitCzasu(limit);
        }
        else if(slowo=="pytanie"){
            quiz->dodajPytanie(pytanie);
            return wczytajPytania();
        }
        else{
            quiz->dodajPytanie(pytanie);
            break;
        }
    }
    return true;
}