# Łachut
## Jak skomunikować serwer i klienta?
### Klient

Na razie przewidujemy, że:
1. Klient może wysłać do serwera komunikaty o następującej strukturze:
    * LIST  -> by poprosić o listę quizów do wystartowania.
    * PICK <id_quizu> -> by wybrać quiz z listy.
    * LAUNCH -> by wystartować wybrany quiz.
    * JOIN <id_trwającego_quizu> -> by dołączyć do trwającego quizu.
    * NICK <nick> -> by ustawić sobie nick.
    * ANSWER <nr_pytania> <nr_odpowiedzi> <nr_odpowiedzi>... -> by udzielić odpowiedzi na pytanie w quizie.
    * POST <długość_pliku> <plik> -> by przesłać na serwer plik z quizem.
2. Przy czym wszystkie te wiadomości (z wyjatkiem pliku z POST) są wysyłane jako jedna linia.

### Serwer
1. Wysyła do klienta następujące komunikaty:
    * FAIL <komenda_która_nawaliła> -> jeśli żądana operacja się nie powiedzie.
    * OK <komenda_która_powiodła_się> -> potwierdzenie, że udało się zmienić nick / wczytać przesłany quiz / wystartować quiz itp.
    * YOURRANK <liczba_punktow> -> bieżąca liczba punktów w quizie.
    * LIST <długość_pliku> <plik> -> plik z listą quizów do wyboru.
    * RANK <długość_pliku> <plik> -> plik z całkowitą punktacją wszystkich graczy.
    * QUESTION "Treść pytania" "odp 1" "odp 2"... -> treść pytania z listą odpowiedzi do wyboru.
2. Serwer również wysyła wszystkie wiadomości jako jedna linia (oprócz plików).
3. W LIST pierwsza linia zawiera komendę i długość pliku w bajtach. Każda kolejna zawiera nazwę id quizu i jego nazwę.
4. W RANK pierwsza linia zawiera RANK i długość pliku w bajtach. Każda kolejna zawiera nick gracza i liczbę punktów przez niego zdobytych. Lista może być nieposortowana.

### Format pliku z quizem
    "Nazwa quizu w pierwszej linii i w nawiasach"

    pytanie = "pytanie znak równości i w nawiasach treść pytania."
    P = "Poprawna odpowiedź"
    F = "Fałyszwa odpowiedź"
    F = "Fałszywych odpowiedzi może być dowolna liczba"
    F = "Poprawnych też może być dowolna liczba"
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
5. Odpowiedzi (poprawnych i niepoprawnych) może być dowolna liczba większa od 0.
6. Mogą być same poprawne odpowiedzi i same fałszywe.