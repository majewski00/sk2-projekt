# Sprawozdanie z projektu - Gra Reversi (Othello)

**Przedmiot:** Sieci Komputerowe  
**Temat:** Implementacja gry logicznej w architekturze klient-serwer  
**Data:** Styczeń 2026

---

## Co udało się zrobić

Projekt polega na stworzeniu gry Reversi (znana też jako Othello) działającej w architekturze klient-serwer. Główny cel to umożliwienie dwóm graczom rozgrywki przez sieć, przy czym serwer pilnuje wszystkich zasad gry i obsługuje wielu graczy jednocześnie.

System składa się z trzech podstawowych części:

- **Serwer** - napisany w C pod Linuksa, obsługuje wielu klientów naraz
- **Klient** - również w C, działa w terminalu z prostym interfejsem tekstowym
- **Protokół komunikacji** - tekstowy, łatwy do debugowania

## Jak to działa

Serwer nasłuchuje na wybranym porcie (domyślnie 8080) i czeka aż podłączą się gracze. Gdy dwóch graczy się połączy, serwer automatycznie ich paruje i tworzy oddzielny proces dla ich gry (używając `fork()`). To dobre rozwiązanie, bo każda rozgrywka jest całkowicie niezależna - nie przeszkadzają sobie nawzajem, a jak coś pójdzie nie tak w jednej grze, inne działają dalej.

Klient ma prosty interfejs tekstowy - wyświetla planszę 8x8 z literkami B (czarny) i W (biały). Kiedy jest twoja tura, wpisujesz współrzędne typu "2 3" i klient wysyła to do serwera. Serwer sprawdza czy ruch jest legalny, odwraca odpowiednie pionki i odsyła nowy stan planszy. Dość wygodne, nie trzeba niczego instalować poza kompilacją.

## Protokół komunikacji

Zdecydowałem się na tekstowy protokół zamiast binarnego. Może nie jest super wydajny, ale za to znacznie ułatwia debugowanie - można spokojnie podłączyć się `netcatem` i zobaczyć co leci po sieci. Komunikaty wyglądają tak:

```
WELCOME|BLACK
BOARD|................................................................
MOVE|2|3
YOUR_TURN
GAME_OVER|win|BLACK|34|30
```

Wszystko oddzielone pionowymi kreskami, zakończone znakiem nowej linii. Proste i działa.

## Struktura kodu

Starałem się to sensownie podzielić żeby nie było spagetti. Serwer to kilka modułów:

- `main.c` - setup socketa, przyjmowanie połączeń
- `matchmaking.c` - kolejka oczekujących graczy, parowanie, tworzenie procesów gier
- `game.c` - czysta logika Reversi (walidacja ruchów, odwracanie pionków, sprawdzanie końca gry)
- `network.c` - wysyłanie i odbieranie komunikatów

Klient podobnie - oddzielny plik na komunikację z serwerem i oddzielny na wyświetlanie planszy.

Wszystkie wspólne definicje (komendy protokołu, stałe) trzymam w katalogu `common/` żeby i serwer i klient używały tego samego.

## Co było najtrudniejsze

Najtrudniejsze było ogarnięcie obsługi rozłączeń. Jak gracz się nagle rozłączy w trakcie gry, trzeba poinformować drugiego gracza i porządnie posprzątać. Skończyło się na ustawianiu timeout'ów na `select()` i obsłudze `SIGCHLD` żeby zombie procesy nie zostały.

Poza tym logika Reversi jest trochę zawiła - trzeba sprawdzić 8 kierunków od każdego ruchu i zobaczyć czy da się cokolwiek odwrócić. Napisałem do tego testy jednostkowe żeby mieć pewność że działa.

## Testy

Projekt zawiera zestaw testów:

- **Testy jednostkowe** - sprawdzają samą logikę gry (walidacja ruchów, odwracanie pionków, wykrywanie końca gry)
- **Testy manualne** - skrypty które symulują różne scenariusze rozgrywek
- **Testy pamięci** - leaks na macOS i valgrind na Linuksie, żeby mieć pewność że nic nie wycieka

Wszystkie testy przechodzą, co mnie szczerze ucieszyło bo debugowanie problemów z pamięcią w C potrafi być koszmarem.

## Uruchomienie

Kompilacja jest prosta:

```bash
make
```

Potem serwer:

```bash
./server_bin 8080
```

I dwóch klientów w osobnych terminalach:

```bash
./client_bin localhost 8080
```

Gra startuje automatycznie jak dwóch graczy się podłączy.

## Podsumowanie

Projekt spełnia wszystkie wymagania - architektura klient-serwer, serwer napisany w C pod Linuksa używający BSD sockets, współbieżność dzięki `fork()`, walidacja ruchów po stronie serwera. Działa stabilnie, nie wycieka pamięć, obsługuje wiele jednoczesnych gier.

Najbardziej satysfakcjonujące było zobaczyć jak to wszystko działa razem - klienci się łączą, grają, serwer wszystko obsługuje bez żadnej ingerencji. Kod jest dość czysty (bez komentarzy jak planowałem - funkcje i zmienne mają sensowne nazwy), więc powinno się łatwo czytać.

Całość jest na GitHubie, dokumentacja w katalogu `docs/` opisuje szczegóły architektury i protokołu.
