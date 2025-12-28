#include "uzytkownik.h"

Uzytkownik::Uzytkownik(int fd){
    this->fd =fd;
}
void Uzytkownik::setNick(std::string nick){
    this->nick = nick;
}
std::string Uzytkownik::getNick(){
    return nick;
}
int Uzytkownik::getfd(){
    return fd;
}
bool Uzytkownik::wyslijWiadomosc(std::string tresc){
    const char* data = tresc.data();
    size_t total = 0;
    size_t len = tresc.size();
    while (total < len) {
        ssize_t sent = send(fd, data + total, len - total, MSG_NOSIGNAL);
        if (sent > 0) total += sent;
        else if (sent == -1 && errno == EINTR) continue;
        else return false;
    }
    return true;
}
//----------------------------------------------------------------------------------------------
MenegerQuizow::MenegerQuizow(){}
MenegerQuizow& MenegerQuizow::the(){
    static MenegerQuizow instance;
    return instance;
}
unsigned int MenegerQuizow::dodajInstancjeQuizu(std::string nazwaQuizu, int fdTworcy){
    Quiz quiz = BazaQuizow::the().getQuiz(nazwaQuizu);
    if(quiz.getNazwa()!=""){
        unsigned int idInstancji = nextval();
        InstancjaQuizu x(quiz, fdTworcy, idInstancji);
        bazaQuizow[idInstancji] = x;
        return idInstancji;
    }
    return 0;
}
InstancjaQuizu& MenegerQuizow::getInstancjaQuizu(unsigned int id){
    try{
        return bazaQuizow[id];
    }
    catch(std::out_of_range const&){
        return bazaQuizow[0];
    }
}
unsigned int MenegerQuizow::nextval(){
    unsigned int result = sequence++;
    while(bazaQuizow.find(result)!=bazaQuizow.end()) result++;
    return sequence;
}
bool MenegerQuizow::usunInstancjeQuizu(unsigned int id){
    if(bazaQuizow.erase(id)==0) return false;
    return true;
}
int MenegerQuizow::getKlientDoRozlaczenia(){
    for(auto it : klienciDoRozlaczenia){
        int wynik = it;
        klienciDoRozlaczenia.erase(it);
        return wynik;
    }
    return -1;
}
void MenegerQuizow::dodajKlientaDoRozlaczenia(int fd){
    klienciDoRozlaczenia.insert(fd);
}
//--------------------------------------------------------------------------------------------------------------------------
InstancjaQuizu::InstancjaQuizu(Quiz quiz, int fdTworcy, unsigned int id){
    this->quiz = quiz;
    this->tworcaQuizu = TworcaQuizu(fdTworcy);
    this->id = id;
}

bool InstancjaQuizu::wystartuj(int fd){
    if(fd!=tworcaQuizu.getfd()) return false;

    if(stan == Stan::CZEKAJACY){
        if(uczestnicy.size() < 1) return false; // brak uczestników
        stan = Stan::TRWAJACY;
        poczBiezacegoPytania = time(NULL);
        kolejnePytanie(true);
        return true;
    }
    return false;
}

bool InstancjaQuizu::dodajUczestnika(int fd){
    if(quiz.getNazwa()=="" ) return false;
    if(stan != Stan::CZEKAJACY) return false;
    uczestnicy.push_back(std::pair<UczestnikQuizu, unsigned int>(UczestnikQuizu(fd), 0));

    for(unsigned int i=0;;i++) if(setNick(fd, "NONAME"+std::to_string(fd + i))) break;
    
    return true;
}

bool InstancjaQuizu::kolejnePytanie(bool czyPoczatkowe){
    if(!czyPoczatkowe) biezacePytanie++;
    ktoOdpowiedzial.clear();
    if(biezacePytanie > quiz.ilePytan()-1){
        zakoncz();
        return false;
    }

    poczBiezacegoPytania = time(NULL);
    if(quiz.getPytanie(biezacePytanie).getLimitCzasu() != 0){
        std::thread(&InstancjaQuizu::przewinPytanie, this, biezacePytanie).detach();
    }

    wyslijPytanie(-1); // do wszystkich
    return true;
}

bool InstancjaQuizu::wyslijPytanie(int fd){
    if(tworcaQuizu.getfd()==fd){
        std::string wiadomosc = "QUESTION ";
        if(stan == Stan::TRWAJACY) wiadomosc += std::to_string(this->biezacePytanie) + "\n";
        else wiadomosc += "-1\n";
        return tworcaQuizu.wyslijWiadomosc(wiadomosc);
    }

    for(auto it : uczestnicy){
        if(it.first.getfd()!=fd && fd >= 0) continue;

        if(stan != Stan::TRWAJACY) it.first.wyslijWiadomosc("QUESTION -1\n");
        else{
            Pytanie bPytanie = quiz.getPytanie(biezacePytanie);
            std::string wiadomosc = "QUESTION "+ std::to_string(biezacePytanie) + " \"" + bPytanie.getTresc() + "\" ";
            for(auto odp : bPytanie.getOdpowiedzi()) wiadomosc += "\""+odp.first+"\" ";
            wiadomosc += "\n";
            it.first.wyslijWiadomosc(wiadomosc);
            wyslijRanking(it.first.getfd());
            if(fd >= 0) return true;
        }
    }
    return fd < 0;
}

std::string InstancjaQuizu::pobierzPytanieZOdpowiedziami(){
    return quiz.getPytanie(biezacePytanie).getTrescZOdpowiedziami();
}

bool InstancjaQuizu::zarejestrujOdpowiedz(int fd, unsigned int nrPytania, std::set<unsigned int> odpowiedzi){
    if(nrPytania!=biezacePytanie) return false; // czas na to pytanie przeminął
    for(auto it : ktoOdpowiedzial) if(it==fd) return false; // ty już odpowiedziałeś!

    ktoOdpowiedzial.insert(fd);

    Pytanie pyt = quiz.getPytanie(nrPytania);
    unsigned int i =0;
    unsigned int poprawne = 0, niepoprawne = 0, wszystkiePoprawne = 0, zdobytePunkty = 0;
    for(auto it : pyt.getOdpowiedzi()){
        if(it.second) wszystkiePoprawne++;

        if(it.second && odpowiedzi.count(i) > 0) poprawne++;
        else if(!it.second && odpowiedzi.count(i) == 0) poprawne++;
        else niepoprawne++;
        i++;
    }

    if(wszystkiePoprawne > 0 && niepoprawne < poprawne){
        zdobytePunkty = 100 * (poprawne - niepoprawne) / wszystkiePoprawne;
        zdobytePunkty *= uczestnicy.size();
        zdobytePunkty /= (1 + ktoOdpowiedzial.size());
    }

    for(auto &it : uczestnicy) if(it.first.getfd()==fd) it.second += zdobytePunkty;

    if(ktoOdpowiedzial.size() >= 2 * uczestnicy.size() / 3) kolejnePytanie(false);
    return true;
}

std::string InstancjaQuizu::getRanking(int fd){
    std::string wynik = "YOURRANK ";
    if(tworcaQuizu.getfd()==fd) wynik = getRanking();
    else{
        for(auto it : uczestnicy) if(it.first.getfd()==fd){
            wynik += std::to_string(it.second);
            wynik += "\n";
        }
    }
    return wynik;
}

std::string InstancjaQuizu::getRanking(){
    std::string wynik = "";
    for(auto it : uczestnicy){
        wynik += it.first.getNick();
        wynik += " ";
        wynik += std::to_string(it.second);
        wynik += "\n";
    }
    return wynik;
}

void InstancjaQuizu::zakoncz(){
    if(stan == Stan::TRWAJACY){
        stan = Stan::ZAKONCZONY;
        std::string ranking = getRanking();
        unsigned int dlugosc = ranking.length();

        tworcaQuizu.wyslijWiadomosc("RANK " + std::to_string(dlugosc) + "\n" + ranking);
        for(auto it : uczestnicy){
            it.first.wyslijWiadomosc("RANK " + std::to_string(dlugosc) + "\n" + ranking);
            shutdown(it.first.getfd(), SHUT_RDWR);
            close(it.first.getfd());
            MenegerQuizow::the().dodajKlientaDoRozlaczenia(it.first.getfd());
        }
        if(!czyTworcaOpuscil){
            shutdown(tworcaQuizu.getfd(), SHUT_RDWR);
            close(tworcaQuizu.getfd());
            MenegerQuizow::the().dodajKlientaDoRozlaczenia(tworcaQuizu.getfd());
        }
    }
    MenegerQuizow::the().usunInstancjeQuizu(id);
}

void InstancjaQuizu::przewinPytanie(unsigned int wystartowanoDlaPytania){
    std::this_thread::sleep_for(std::chrono::seconds(quiz.getPytanie(wystartowanoDlaPytania).getLimitCzasu()));
    if(biezacePytanie==wystartowanoDlaPytania) kolejnePytanie(false);
    return;
}

bool InstancjaQuizu::usun(int fd){
    if(tworcaQuizu.getfd()==fd){
        czyTworcaOpuscil = true;
        if(uczestnicy.size()==0) zakoncz();
        else wystartuj(fd);
        return true;
    }

    for(unsigned int i=0;i<uczestnicy.size();i++){
        if(uczestnicy[i].first.getfd()==fd){
            uczestnicy.erase(uczestnicy.begin()+i);
            if(uczestnicy.size()==0) zakoncz();
            return true;
        } 
    }
    return false;
}

bool InstancjaQuizu::setNick(int fd, std::string nick){
    if(tworcaQuizu.getfd()==fd) return false;

    int i=0, ind=-1;
    for(auto it : uczestnicy){
        if(it.first.getNick()==nick) return false;
        if(it.first.getfd()==fd) ind = i;
        i++;
    }
    if(ind==-1) return false;
    uczestnicy.at(ind).first.setNick(nick);
    return true;
}

void InstancjaQuizu::wyslijRanking(int fd){
    std::string wynik = "";
    if(fd==tworcaQuizu.getfd()){
        std::string ranking = getRanking();
        wynik = "RANK " + std::to_string(ranking.length()) +"\n"+ ranking;
        tworcaQuizu.wyslijWiadomosc(wynik);
        return;
    }
    wynik = getRanking(fd);

    for(auto it : uczestnicy) if(it.first.getfd()==fd) it.first.wyslijWiadomosc(wynik);
}
//---------------------------------------------------------------------------------------------------