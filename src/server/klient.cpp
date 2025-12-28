#include "klient.h"

Klient::Klient(int fd){
    this->fd = fd;
}

Klient::~Klient(){
    std::string nazwaPlikuDoUsuniecia = "quizSource/temporary"+std::to_string(fd)+".qcf";
    unlink(nazwaPlikuDoUsuniecia.c_str());
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

    if(cmd == "LIST"){
        std::cout<<"LIST"<< std::endl;
        std::string lista = BazaQuizow::the().getListaQuizow();
        size_t dlugosc = lista.size();
        wyslijWiadomosc("LIST " + std::to_string(dlugosc) + "\n" + lista);
    }

    else if(cmd == "POST"){
        size_t len;
        iss >> len;
        std::cout<<"POST "<< len << std::endl;
        zacznijPobieracPlik(len);
    }
    
    else if(cmd == "JOIN"){
        unsigned int id;
        iss >> id;
        std::cout<<"JOIN "<< id << std::endl;

        if(MenegerPokoi::the().znajdzPokoj(fd)==0 && MenegerQuizow::the().getInstancjaQuizu(id).dodajUczestnika(fd)){
            MenegerPokoi::the().dodajDoPokoju(fd, id);
            wyslijWiadomosc("OK JOIN\n");
        }
        else wyslijWiadomosc("FAIL JOIN\n");
    }
    
    else if(cmd == "NICK"){
        std::string nick;
        iss >> nick;
        std::cout<<"NICK "<< nick << std::endl;

        if(MenegerPokoi::the().znajdzPokoj(fd)!=0 && MenegerQuizow::the().getInstancjaQuizu(MenegerPokoi::the().znajdzPokoj(fd)).setNick(fd, nick)) wyslijWiadomosc("OK NICK\n");
        else wyslijWiadomosc("FAIL NICK\n");
    }

    else if(cmd == "LAUNCH"){
        std::cout<<"LAUNCH"<< std::endl;

        if(MenegerQuizow::the().getInstancjaQuizu(MenegerPokoi::the().znajdzPokoj(fd)).wystartuj(fd)) wyslijWiadomosc("OK LAUNCH\n");
        else wyslijWiadomosc("FAIL LAUNCH\n");
    }
    
    else if(cmd == "SETUP"){
        std::string nazwa;
        std::getline(iss >> std::ws, nazwa);
        std::cout<<"SETUP "<< nazwa << std::endl;
        if(MenegerPokoi::the().znajdzPokoj(fd)==0){
            unsigned int idInstancji = MenegerQuizow::the().dodajInstancjeQuizu(nazwa, fd);
            if(idInstancji!=0){
                MenegerPokoi::the().dodajDoPokoju(fd, idInstancji);
                std::string odp = "YOURID " + std::to_string(idInstancji) + "\n";
                wyslijWiadomosc(odp);
            }
            else wyslijWiadomosc("FAIL SETUP\n");
        }
        else wyslijWiadomosc("FAIL SETUP\n");
    }
    
    else if(cmd == "ANSWER"){
        int nr, nr_pytania;
        std::set<unsigned int> odp;
        iss >> nr_pytania;
        while (iss >> nr) odp.insert(nr);
        std::cout<<"ANSWER to "<< nr_pytania << " is: ";
        for(auto it : odp) std::cout<< it << " ";
        std::cout<<std::endl;
        
        if(MenegerQuizow::the().getInstancjaQuizu(MenegerPokoi::the().znajdzPokoj(fd)).zarejestrujOdpowiedz(fd, nr_pytania, odp)){
            std::string odp = MenegerQuizow::the().getInstancjaQuizu(MenegerPokoi::the().znajdzPokoj(fd)).getRanking(fd);
            wyslijWiadomosc(odp);
        }
        else wyslijWiadomosc("FAIL ANSWER\n");
    }

    else if(cmd == "GETRANK" || cmd == "MYSCORE"){
        std::cout<<cmd<< std::endl;
        MenegerQuizow::the().getInstancjaQuizu(MenegerPokoi::the().znajdzPokoj(fd)).wyslijRanking(fd);
    }
    
    else if(cmd == "STATUS"){
        std::cout<<cmd<<std::endl;
        if(!MenegerQuizow::the().getInstancjaQuizu(MenegerPokoi::the().znajdzPokoj(fd)).wyslijStatus(fd)) wyslijWiadomosc("FAIL STATUS\n");
    }

    else if(cmd == "EXIT"){
        std::cout<<"EXIT"<< std::endl;
        
        MenegerQuizow::the().getInstancjaQuizu(MenegerPokoi::the().znajdzPokoj(fd)).usun(fd);
        MenegerPokoi::the().usunGoscia(fd);
        MenegerQuizow::the().dodajKlientaDoRozlaczenia(fd);
    }

    else wyslijWiadomosc("FAIL " + cmd + "\n");
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

    std::string nazwaPlikuDoUsuniecia = "quizSource/temporary" + std::to_string(fd) + ".qcf";
    unlink(nazwaPlikuDoUsuniecia.c_str());
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

    std::string nazwaPlikuTymczasowego = "quizSource/temporary" + std::to_string(fd) + ".qcf";
    int fdQuizu = open(nazwaPlikuTymczasowego.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0644);
    if(fd==-1){
        wyslijWiadomosc("FAIL POST : nie udało się stworzyć temporary.qcf\n");
        return;
    }
    ssize_t bajty = write(fdQuizu, plik.data(), plik.size());
    if(bajty<0){
        wyslijWiadomosc("FAIL POST : write nie powiodło się\n");
        return;
    }
    close(fdQuizu);

    Quiz quiz;
    ParserQuizu parser(nazwaPlikuTymczasowego, &quiz);
    if(!parser.parsujQuiz()){
        wyslijWiadomosc("FAIL POST : błąd składniowy w quizie\n");
        return;
    }
    if(quiz.getNazwa().substr(0, 9)=="temporary"){
        wyslijWiadomosc("FAIL POST : nie można nazwać pliku zaczynającego się od temporary\n");
        return;
    }

    if(renameat2(AT_FDCWD, nazwaPlikuTymczasowego.c_str(), AT_FDCWD, ("quizSource/" + quiz.getNazwa() + ".qcf").c_str(), RENAME_NOREPLACE)==-1){
        wyslijWiadomosc("FAIL POST : Quiz o tej nazwie już istnieje\n");
        return;
    }

    wyslijWiadomosc("OK POST\n");
    return;
}

bool Klient::wyslijWiadomosc(std::string tresc){
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

void Klient::usun(){
    MenegerQuizow::the().getInstancjaQuizu(MenegerPokoi::the().znajdzPokoj(fd)).wystartuj(fd);
    MenegerQuizow::the().getInstancjaQuizu(MenegerPokoi::the().znajdzPokoj(fd)).usun(fd);
    MenegerPokoi::the().usunGoscia(fd);
}
//------------------------------------------------------------------------------------------------
MenegerPokoi& MenegerPokoi::the(){
    static MenegerPokoi instance;
    return instance;
}

unsigned int MenegerPokoi::znajdzPokoj(int fd){
    if(pokoje.count(fd)==0) return 0;
    return pokoje.at(fd);
}

bool MenegerPokoi::dodajDoPokoju(int fd, unsigned int idQuizu){
    pokoje[fd] = idQuizu;
    return true;
}

bool MenegerPokoi::usunPokoj(unsigned int idQuizu){
    bool wynik = false;
    for (auto it = pokoje.begin(); it != pokoje.end(); ) {
        if(it->second == idQuizu) {
            it = pokoje.erase(it);
            wynik = true;
        } 
        else ++it;
    }
    return wynik;
}

bool MenegerPokoi::usunGoscia(int fd){
    if(pokoje.erase(fd)==0){
        int idPokoju = znajdzPokoj(fd);
        if(znajdzGosci(idPokoju).size()==0) usunPokoj(idPokoju);
        return false;
    }
    return true;
}

std::set<int> MenegerPokoi::znajdzGosci(unsigned int idQuizu){
    std::set<int> wynik;
    for (auto it = pokoje.begin(); it != pokoje.end(); ) {
        if (it->second == idQuizu) wynik.insert(it->first);
        else ++it;
    }
    return wynik;
}