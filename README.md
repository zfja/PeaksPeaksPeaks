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

- strzałki **góra/dół** — przełączanie widm z listy
- ruch myszą po wykresie — ustawienie środka piku
- **klik** — zatwierdzenie środka i przejście do ustawiania szerokości
- strzałki **lewo/prawo** — regulacja szerokości piku
- **Enter** — zatwierdzenie szerokości
- **Esc** — cofnięcie kroku
- **Ctrl/Cmd + Esc** — wyczyszczenie pomiaru dla bieżącego pliku

Pomiary zapisują się automatycznie do pliku `data.csv` obok aplikacji.

## Struktura projektu

| Plik | Opis |
|------|------|
| `peakspeakspeaks.*` | główne okno, interakcja z wykresem i zapis pomiarów |
| `SpectrumLoader.*`  | wczytywanie danych widmowych z pliku |
| `MathEngine.*`      | wygładzanie Savitzky'ego-Golaya |
| `FileManager.*`     | skanowanie katalogu i sortowanie plików |
