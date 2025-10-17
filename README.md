# PWrRacingTeam
Zadania na rekrutacje

## Zadanie 2
### 1 - Otoczka
Po namyśle (bezowocnym) wykorzystałem pomocy ludzi mądrzejszych (internetu) i zaimplementowałem algorytm QuickHull. Wydawał się najbardziej rozsądny do zrozumienia dla mnie. Z początku chciałem uniknąć rekurencji, ale ostatecznie uległem i użyłem pomocniczej funkcji OtoczkaHelper, by rekurencyjnie wykonać algorytm.
Pierwszym moim pomysłem było wybranie najwyższego punktu i „owijanie” wszystkich punktów, ale albo jest to „średnie” w implementacji, albo ewidentnie brakowało mi wiedzy matematycznej.
Implementacja algorytmu raczej była bezproblemowa. Funkcja zwraca listę punktów należących do otoczki, ponieważ okazały się przydatne do następnego podpunktu.
### 2 - Proste
Jako wejście podaję listę punktów należących do otoczki i następnie je sortuję. Wpadłem na pomysł, że można wykorzystać już znane granice zbioru i lecąc po kolei po bokach, konstruować proste i porównywać z najdalszym punktem (nie trzeba drugiej prostej, gdyż wiadomo, że ma być równoległa i będzie tak samo oddalona po całej długości).
Skonsultowałem pomysł z Claude AI, czy moja „intuicja” mnie nie oszukuje (o dziwo nie), a następnie napisałem kod.
Przez jakiś czas kod miał buga, iterowałem od pierwszego punktu do przedostatniego, biorąc pary punktów, lecz nie uwzględniłem pary pierwszego i ostatniego punktu, czyli pary, która domyka moje „koło”. Po krótkim, ostatecznym teście udało mi się to zauważyć.
### 3 - Najbliższe Punkty
Klasyczne podejście dla wielu problemów wymagających interakcji „każdy z każdym”, czyli dziel i rządź. Tutaj też uległem rekurencji. Nie jestem pewien, czy jest to jakoś bardzo zoptymalizowane, ale powinno być lepsze od O(n²). Problemem okazało się nie zaliczanie punktów pomiędzy podzielonymi sektorami. Troszkę mi to zajęło, żeby zrobić to w sensowny sposób, czyli branie punktów tylko takich które mogłyby być kandydatami na podstawie wcześniej znalezionych punktów.
