# Łachut
## Wymagania funkcjonalne
- Użytkownik (twórca quizu) łączy się z serwerem i wybiera quiz do wystartowania. Otrzymuje wówczas kod quizu. Jeden użytkownik może mieć w danym momencie uruchomiony jeden quiz. Nie może być jednocześnie uczestnikiem i twórcą. Serwer obsługuje wiele quizów na raz.
- Użytkownik (uczestnik) łączy się z serwerem, podaje kod quizu i do niego dołącza, podając nick. Jeżeli w quizie ktoś inny podał taki nick, serwer prosi o jego zmianę. Jeśli quiz jest już uruchomiony serwer odrzuca zapytanie.
- Gdy pojawi się co najmniej 1 uczestnik quizu, twórca quizu może uruchomić zadawanie pytań.
- Uczestnicy quizu odpowiadają na pytania. Punkty przyznawane są za poprawność i prędkość odpowiedzi. Użytkownik może odpowiedzieć na pytanie tylko, jeżeli odpowiedziało na nie mniej niż 2/3 uczestników lub nie upłynął czas przeznaczony na to pytanie.
- Twórca quizu widzi punktację zarówno bieżącą, jak i ostateczną. Uczestnicy quizu mogą zobaczyć jedynie swoją bieżącą punktację i ostateczną punktację wszystkich uczestników. Po zakończeniu ostatniego pytania uczestnikom quizu i jego twórcy prezentowany jest ranking uczestników.
- Użytkownik może wgrać własny quiz na serwer, przesyłając plik o określonej strukturze.
Jeśli plik przejdzie proces walidacji, zapisany w nim quiz staje się dostępny do wystartowania.
W przeciwnym wypadku użytkownik zostanie powiadomiony o błędzie.
- Po zakończeniu quizu zarówno twórca quizu, jak i jego uczestnicy są rozłączani.

## Jak skomunikować serwer i klienta?
### Klient

Na razie przewidujemy, że:
1. a) Twórca quizu może wysłać do serwera komunikaty o następującej strukturze:
    * `POST <długość_pliku>`  
    `<plik>` -> by przesłać na serwer plik z quizem.
    * `LIST` -> by poprosić o listę quizów do wystartowania.
    * `SETUP <nazwa_quizu>` -> by otrzymać kod do quizu. Nazwa quizu powinna być podana dokładnie tak, jak została wysłana: ze spacjami i dziwnymi znakami, bez cudzysłowów itp.
    * `LAUNCH` -> by wystartować wybrany quiz.
    * `GETRANK` -> by poprosić o podanie całego rankingu.  uczestnik jedynie liczbę swoich punktów.
    * `STATUS` -> by uzyskać w odpowiedzi komunikat o numerze aktualnego pytania i liczbie uczestników.
    * `EXIT` -> by poinformować o tym, że się rozłącza.

    b) Uczestnik:
    * `JOIN <id_oczekującego_quizu>` -> by dołączyć do trwającego quizu.
    * `NICK <nick>` -> by ustawić sobie nick.
    * `ANSWER <nr_pytania> <nr_odpowiedzi> <nr_odpowiedzi>...` -> by udzielić odpowiedzi na pytanie w quizie.
    * `MYSCORE` -> by otrzymać swój wynik.
    * `EXIT` -> by poinformować o tym, że się rozłącza.
2. Przy czym wszystkie te wiadomości (z wyjatkiem pliku z POST) są wysyłane jako jedna linia.

### Serwer
1. Wysyła do klienta następujące komunikaty:
    * `FAIL <komenda_która_nawaliła> <treść_błędu> `-> jeśli żądana operacja się nie powiedzie.
    * `OK <komenda_która_powiodła_się>` -> potwierdzenie, że udało się zmienić nick / wczytać przesłany quiz / wystartować quiz itp.
    * `YOURID <id_quizu_do_wystartowania>` -> wiadomość z id quizu, który należy podać uczestnikom quizu, by mogli do niego dołączyć.
    * `YOURRANK` <liczba_punktow> -> bieżąca liczba punktów w quizie.
    * `LIST <długość_pliku>`  
    `<plik>` -> plik z listą quizów do wyboru.
    * `RANK <długość_pliku>`  
    `<plik>` -> plik z całkowitą punktacją wszystkich graczy. Wysyłany po zakończeniu quizu do wszytkich, a do twórcy za każdym razem, gdy o niego poprosi.
    * `QUESTION <nr_pytania> "Treść pytania" "odp 1" "odp 2"...` -> treść pytania z listą odpowiedzi do wyboru.
    * `STATUS` <nr_pytania> <liczba_uczestników> -> nr bieżącego pytania z aktualną liczbą uczestników.
2. Serwer również wysyła wszystkie wiadomości jako jedna linia (oprócz plików).
3. W LIST pierwsza linia zawiera komendę i długość pliku w bajtach. Każda kolejna zawiera unikalną nazwę quziu.
4. W RANK pierwsza linia zawiera RANK i długość pliku w bajtach. Każda kolejna zawiera nick gracza i liczbę punktów przez niego zdobytych. Lista może być nieposortowana.

### Format pliku z quizem
    "Nazwa quizu w pierwszej linii i w nawiasach"

    pytanie = "pytanie znak równości i w nawiasach treść pytania."
    P = "Poprawna odpowiedź"
    F = "Fałyszwa odpowiedź"
    F = "Fałszywych odpowiedzi może być od 0 do 4"
    F = "Poprawnych też może być od 0 do 4. Suma poprawnych i fałszywych odpowiedzi musi być nie większa niż 4 i większa od 0."
    limit = 10

    pytanie= "Jedna informacja w linii"
    P = "odp 1"
    F = "odp 2"
    F = "odp 3"
    limit = 0

    pytanie= "Pytanie 3"
    P="odp 1"
    F ="odp 2"

    pytanie ="Pytanie 4"
    P="odp 1"
    P="odp 2"
    P="odp 3"
    P="odp 4"

1. Białe znaki nie powinny wpływać na parsowanie.
2. Jeżeli limit=0, to bez ograniczenia czasowego dla pytania.
3. Limit nie może być ujemny.
4. Treści pytań i odpowiedzi w cudzysłowie. Cudzysłowów nie wolno zagnieżdzać.
5. Odpowiedzi (poprawnych i niepoprawnych) może być dowolna liczba od 0 do 4.
6. Mogą być same poprawne odpowiedzi i same fałszywe.

### Obsługa wyjątkowych sytuacji
1. Jeżeli quiz nie został wystartowany, a twórca quizu rozłączył się, quiz zostanie wystartowany automatycznie.
2. Jeżeli wszyscy uczestnicy quizu rozłączą się w jego trakcie, quiz zostanie natychmiast zakończony.
3. Uczestnicy, którzy rozłączyli się przed zakończeniem quizu, nie zostaną uwzględnieni w końcowym rankingu.
4. Warunek: "jeżeli na pytanie odpowiedziało mniej niż 2/3 uczestników" tyczy się liczby obecnej liczby uczestników (nie początkowej).
5. Jeżeli uczestnik nie poda nicku, zostanie mu przydzielony nick rodzaju NONAME<jakaś liczba>.