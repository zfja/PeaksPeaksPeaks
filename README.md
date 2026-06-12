# PeaksPeaksPeaks

[![Doxygen GitHub Pages](https://github.com/zfja/PeaksPeaksPeaks/actions/workflows/doxygen.yml/badge.svg)](https://github.com/zfja/PeaksPeaksPeaks/actions/workflows/doxygen.yml)

Aplikacja Qt do analizy i wizualizacji widm. Wczytuje dane widmowe z plików `.txt`,
wygładza je filtrem Savitzky'ego-Golaya i pozwala interaktywnie zmierzyć położenie
oraz szerokość maksymalnie dwóch pików.

## Dokumentacja

Pełna dokumentacja (Doxygen): **https://zfja.github.io/PeaksPeaksPeaks/**

Aby wygenerować ją lokalnie:

```bash
doxygen Doxyfile
open docs/html/index.html
```

## Budowanie

Wymagania: CMake (>= 3.16) oraz Qt 5/6 z modułem **Charts**.

```bash
cmake -B build
cmake --build build
```

Plik wykonywalny pojawi się w katalogu `build/`.

## Obsługa

- **Choose folder…** — wybór folderu z plikami `.txt`; wybór jest zapamiętywany między uruchomieniami
- strzałki **góra/dół** — przełączanie widm z listy
- ruch myszą po wykresie — ustawienie środka piku
- **klik** — zatwierdzenie środka i przejście do ustawiania szerokości
- strzałki **lewo/prawo** — regulacja szerokości piku
- **Enter** — zatwierdzenie szerokości
- **R** — cofnięcie kroku
- **Ctrl/Cmd + R** — wyczyszczenie pomiaru dla bieżącego pliku
- **Save PNG** / **Ctrl/Cmd + S** — zapis bieżącego wykresu do PNG (z tytułem = temperatura + °C oraz zaznaczonymi pikami)

## Zapisywane pliki

Wszystkie wyniki trafiają do podfolderu **`peakspeakspeaks`** wewnątrz wybranego folderu z danymi
(tworzonego automatycznie, jeśli nie istnieje):

- `data.csv` — pomiary pików; wiersze są sortowane po temperaturze malejąco (od najwyższej do najniższej)
- eksporty PNG (pojedyncze oraz **Save all**)

> **Uwaga (Windows):** zapis/odczyt `data.csv` korzysta ze standardowego strumienia plików, który na Windows
> interpretuje ścieżkę w lokalnym codepage. Jeśli ścieżka do folderu zawiera znaki spoza ASCII (np. polskie
> znaki w nazwie folderu), zapis CSV może się nie powieść — najlepiej trzymać dane w folderze o nazwie ASCII.
> Eksport PNG nie ma tego ograniczenia.

## Ustawienia

Okno **Settings** pozwala dostosować i **globalnie zapamiętać** (między uruchomieniami, niezależnie od folderu):

- **kolory** serii surowej i wygładzonej
- **tytuły osi** X i Y
- parametry filtra Savitzky'ego-Golaya:
  - **Smoothing window (points, odd)** — długość okna (3–151, wartości parzyste są zaokrąglane w górę do nieparzystych)
  - **Polynomial order** — stopień dopasowywanego wielomianu (2–6)
- **Save all** — eksport wykresów PNG dla **wszystkich** widm z listy naraz do folderu `peakspeakspeaks`

Parametry wygładzania odświeżają wykres na żywo. Filtr używa zadanego okna, a przy brzegach
zmniejsza je symetrycznie, więc skrajne punkty też są wygładzane (bez zaszumionych „ogonów").

## Struktura projektu

Nagłówki (`.h`) są w katalogu `include/`, a pliki źródłowe (`.cpp`, `.ui`) w `src/`:

| Moduł | Opis |
|-------|------|
| `peakspeakspeaks` | główne okno, interakcja z wykresem, zapis pomiarów i eksport PNG |
| `SpectrumLoader`  | wczytywanie danych widmowych z pliku |
| `MathEngine`      | wygładzanie Savitzky'ego-Golaya |
| `FileManager`     | skanowanie katalogu i sortowanie plików |
| `src/main.cpp`    | punkt wejścia aplikacji |

## Autor

Zofia Tryznowska ([@zfja](https://github.com/zfja))
