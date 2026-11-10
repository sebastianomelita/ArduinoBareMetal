[← Indice delle dispense](indice.md)

# Dispensa: insertion sort

9 novembre 2026

<a id="s1"></a>

## 1. L'idea, dall'animazione

L'animazione mostra l'insertion sort al lavoro sulla sequenza **6 5 3 1 8 7 2 4**. Guardandola si nota che l'array è sempre diviso in **due insiemi**:

<p align="center">
  <img src="img/insertion_sort_animazione.gif" alt="Animazione dell'insertion sort sulla sequenza 6 5 3 1 8 7 2 4">
</p>

- l'**insieme ordinato**, in basso (a sinistra, indici piccoli): i valori già sistemati, in ordine crescente fra loro;
- l'**insieme non ordinato**, in alto (a destra, indici grandi): i valori non ancora esaminati, nell'ordine in cui sono arrivati.

All'inizio l'insieme ordinato contiene solo il primo elemento, `6`: un solo valore è ordinato per definizione. A ogni passo l'algoritmo prende il **primo elemento dell'insieme non ordinato** e lo fa scendere fino al suo posto nell'insieme ordinato. Così l'insieme ordinato cresce di uno e quello non ordinato cala di uno.

Quando l'insieme non ordinato è vuoto, l'intero array è ordinato.

**Definizione.** L'*insertion sort* (ordinamento per inserimento) ordina un array inserendo, uno alla volta, ciascun elemento dell'insieme non ordinato nell'insieme ordinato, con la tecnica dell'*inserimento ordinato*.

<a id="s2"></a>

## 2. Passo per passo

Ogni riga mostra l'array **dopo** l'inserimento dell'elemento `x = v[i]`. La barra `|` separa l'insieme ordinato (a sinistra) da quello non ordinato (a destra).

| Passo | i | x | Array dopo l'inserimento | Shift a destra |
| --- | --- | --- | --- | --- |
| inizio | – | – | 6 \| 5 3 1 8 7 2 4 | – |
| 1 | 1 | 5 | 5 6 \| 3 1 8 7 2 4 | 1 (il 6) |
| 2 | 2 | 3 | 3 5 6 \| 1 8 7 2 4 | 2 (5, 6) |
| 3 | 3 | 1 | 1 3 5 6 \| 8 7 2 4 | 3 (3, 5, 6) |
| 4 | 4 | 8 | 1 3 5 6 8 \| 7 2 4 | 0 (8 è già al suo posto) |
| 5 | 5 | 7 | 1 3 5 6 7 8 \| 2 4 | 1 (l'8) |
| 6 | 6 | 2 | 1 2 3 5 6 7 8 \| 4 | 5 (3, 5, 6, 7, 8) |
| 7 | 7 | 4 | 1 2 3 4 5 6 7 8 | 4 (5, 6, 7, 8) |

Con `N = 8` elementi servono `N - 1 = 7` passi: il primo elemento non va inserito. Il numero di shift dipende da quanto il valore è piccolo rispetto a quelli già ordinati: l'8 non ne richiede nessuno, il 2 ne richiede 5.

<a id="s3"></a>

## 3. Il loop esterno: l'indice separatore

Il loop esterno mantiene un indice `i` che fa da **separatore** tra i due insiemi:

- `v[0] … v[i-1]` è l'**insieme ordinato** (in basso);
- `v[i] … v[N-1]` è l'**insieme non ordinato** (in alto);
- `v[i]` è il prossimo elemento da inserire.

```cpp
for (int i = 1; i < N; i++) {
    // qui: v[0..i-1] ordinato, v[i..N-1] non ordinato
    // inserimento ordinato di v[i] in v[0..i-1]
}
```

Il ciclo parte da `i = 1` perché l'insieme formato dal solo `v[0]` è già ordinato. A ogni iterazione l'inserimento allarga l'insieme ordinato di un elemento; l'incremento `i++` sposta il separatore di una posizione verso l'alto.

All'uscita `i = N`: l'insieme non ordinato è vuoto e tutto l'array è ordinato.

La proprietà "*prima di ogni iterazione, `v[0..i-1]` è ordinato*" si chiama **invariante del ciclo**: è vera all'inizio, ogni iterazione la conserva, e alla fine garantisce il risultato.

<a id="s4"></a>

## 4. Il loop interno: l'inserimento ordinato

Il loop interno esegue l'**inserimento ordinato** di `x = v[i]` nell'insieme ordinato `v[0..i-1]`. Inserire in modo ordinato richiede due cose:

1. **cercare il posto giusto**: la posizione dopo l'ultimo elemento minore o uguale a `x`;
2. **fare uno shift a destra** di una posizione degli elementi maggiori di `x`, per lasciare libero quel posto al nuovo valore.

Lo shift a destra va fatto **partendo dall'alto**, altrimenti ogni copia cancellerebbe un valore non ancora spostato. Per questo la ricerca parte da `j = i - 1`, cioè dal valore più alto dell'insieme ordinato, e scende: ogni elemento maggiore di `x` viene subito spostato in su di una posizione. Ricerca e shift avvengono così nello stesso ciclo.

```cpp
int x = v[i];          // salvo il valore: v[i] verrà sovrascritto
int j;                 // dichiarata fuori: serve anche dopo il for
for (j = i - 1; j >= 0 && v[j] > x; j--)
    v[j + 1] = v[j];   // shift a destra di v[j], la ricerca scende con j--
v[j + 1] = x;          // inserisco nel posto liberato
```

Il `for` riunisce in una riga i tre elementi del ciclo: partenza dall'alto (`j = i - 1`), condizione (`j >= 0 && v[j] > x`) e discesa (`j--`). La condizione ha due parti, con ruoli diversi:

- `j >= 0` è la **guardia di fine corsa**: scendendo, impedisce di andare sotto `v[0]` (in letteratura si parla di *condizione di guardia*, o di *controllo dei limiti*; in inglese *bounds check*);
- `v[j] > x` è la **condizione di avanzamento** (o di permanenza nel ciclo): finché il valore la rispetta, viene spostato in su e `j` scende.

All'uscita `j` è il primo indice che, scendendo, rompe la condizione di avanzamento (o di permanenza), oppure -1 se nessuno la rompe (fine corsa). In entrambi i casi `x` va subito sopra, in `v[j + 1]`.

La variabile `j` va dichiarata **prima** del `for`: se fosse dichiarata dentro, non esisterebbe più nella riga `v[j + 1] = x`, dove serve per sapere dove inserire.

### Simulazione completa

L'animazione mostra l'ordinamento completo di 6 5 3 1 8 7 2 4 con questo codice: tutti i 7 inserimenti, uno dopo l'altro. Per ogni passo si vede `x` salvato a parte, il buco in arancione con l'etichetta `v[j + 1]`, la freccia su `j` e in azzurro, con l'etichetta `v[j]`, l'elemento confrontato con `x`. La linea verticale separa l'insieme ordinato da quello non ordinato.

<p align="center">
  <img src="img/insertion_sort_j_piu_1.gif" alt="Animazione: insertion sort completo con j e j+1, solo shift">
</p>

In tutto servono 16 shift. Nei passi 1, 2 e 3 `x` è il più piccolo dell'insieme ordinato: `j` scende fino a -1, fuori dall'array, e `x` va in `v[0]`. Il passo 6 è analizzato riga per riga nell'esempio che segue.

### Esempio: il passo 6 dell'animazione

Prima del passo l'array è `1 3 5 6 7 8 | 2 4`, con `i = 6` e `x = 2`.

| j | v\[j\] | v\[j\] > 2? | Azione | Array |
| --- | --- | --- | --- | --- |
| 5 | 8 | sì | `v[6] = 8` | 1 3 5 6 7 8 8 4 |
| 4 | 7 | sì | `v[5] = 7` | 1 3 5 6 7 7 8 4 |
| 3 | 6 | sì | `v[4] = 6` | 1 3 5 6 6 7 8 4 |
| 2 | 5 | sì | `v[3] = 5` | 1 3 5 5 6 7 8 4 |
| 1 | 3 | sì | `v[2] = 3` | 1 3 3 5 6 7 8 4 |
| 0 | 1 | no | stop, posto trovato: `j + 1 = 1` |  |
| – | – | – | `v[1] = 2` | 1 2 3 5 6 7 8 4 |

Durante lo shift un valore compare due volte (ad esempio `8 8`): è normale, la copia in basso verrà sovrascritta al passo successivo o dall'inserimento finale. Il valore `x` non va perso perché è stato salvato all'inizio.

### Come fa `j` a diventare -1 se la condizione è `j >= 0`?

Proprio perché `j >= 0` è la condizione per **continuare**, il ciclo si ferma sul primo valore di `j` che la rende falsa, cioè -1. In un `for` il controllo viene fatto **prima** di ogni giro, il `j--` **alla fine**:

```
inizializzazione → controllo → corpo → j-- → controllo → corpo → j-- → controllo → ...
```

Con `j = 0` il controllo è ancora vero; se anche `v[0] > x`, si esegue lo shift e poi `j--` porta `j` a -1. Il controllo successivo è falso e il ciclo esce. Esempio, passo 1 della simulazione (6 | 5, `x = 5`):

| Momento | `j` | Controllo `j >= 0 && v[j] > x` | Cosa succede |
| --- | --- | --- | --- |
| inizializzazione `j = i - 1` | 0 | `0 >= 0` vero, `6 > 5` vero | si entra nel corpo |
| corpo | 0 | | `v[1] = v[0]`: shift del 6 |
| `j--` | -1 | | |
| controllo | -1 | `-1 >= 0` falso | **stop**, `v[-1]` non viene letto |
| dopo il ciclo | -1 | | `v[j + 1] = v[0] = 5` |

`j = -1` non è un errore: è il valore con cui il ciclo esce, e serve, perché `v[j + 1]` diventa `v[0]`, il posto giusto per il valore più piccolo.

### La valutazione a corto circuito

La guardia va scritta **prima** della condizione di avanzamento: `v[-1]` non viene mai letto grazie alla **valutazione a corto circuito** (*short-circuit evaluation*) degli operatori logici `&&` e `||`: il secondo operando viene valutato solo se serve a decidere il risultato.

- con `a && b`, se `a` è falso il risultato è sicuramente falso e `b` **non viene valutato**;
- con `a || b`, se `a` è vero il risultato è sicuramente vero e `b` **non viene valutato**.

Per questo l'ordine delle condizioni conta:

```cpp
j >= 0 && v[j] > x    // corretto: con j = -1 si ferma prima di leggere v[-1]
v[j] > x && j >= 0    // sbagliato: con j = -1 legge v[-1], fuori dall'array
j >= 0 & v[j] > x     // sbagliato: & (bit a bit) valuta sempre entrambi i lati
```

### Focus: perché si inserisce in `v[j + 1]`

La chiave è il **buco**, cioè la cella libera in cui finirà `x`. Durante tutto il ciclo interno vale questa regola: **il buco è sempre `v[j + 1]`**.

1. **All'inizio** `j = i - 1`, quindi `j + 1 = i`. Il buco è proprio `v[i]`: il suo valore è stato salvato in `x`, quindi la cella si può sovrascrivere.
2. **A ogni passo** `v[j]` è maggiore di `x` e viene copiato nel buco con `v[j + 1] = v[j]`. Ora è `v[j]` a contenere una copia inutile: è lui il nuovo buco. Con `j--` l'indice scende di uno e il buco torna a essere `v[j + 1]`.
3. **Alla fine** il ciclo si ferma e il buco è ancora in `v[j + 1]`. Lì va `x`.

In altre parole, `j` indica l'elemento che si sta confrontando con `x`, e `j + 1` la cella libera subito sopra di lui. I due scendono insieme.

L'animazione mostra i tre modi in cui il ciclo può fermarsi, presi dai passi 6, 3 e 4 dell'ordinamento. Nel momento del blocco compaiono la freccia rossa su `j` e la condizione calcolata con i valori di quel momento.

<p align="center">
  <img src="img/tre_casi_blocco_insertion_sort.gif" alt="I tre casi di blocco dell'inserimento ordinato: passi 6, 3 e 4">
</p>

### Perché `v[j + 1]` è il posto giusto

Quando il ciclo si ferma, l'insieme ordinato è diviso in due parti attorno al buco:

- sotto il buco, da `v[0]` a `v[j]`, ci sono i valori **minori o uguali** a `x`: non sono stati toccati;
- sopra il buco, da `v[j + 2]` a `v[i]`, ci sono i valori **maggiori** di `x`: sono stati spostati in su di una posizione.

Mettendo `x` nel buco l'ordine è rispettato su entrambi i lati. Il ciclo si può fermare in tre modi, e la stessa istruzione `v[j + 1] = x` funziona in tutti:

| Caso | Esempio dall'animazione | Dove si ferma `j` | Perché si ferma | `x` va in |
| --- | --- | --- | --- | --- |
| Trova un elemento non maggiore di `x` | passo 6, `x = 2` | `j = 0` (`v[0] = 1`) | `1 > 2` è falso | `v[1]` |
| `x` è il più piccolo di tutti | passo 3, `x = 1` | `j = -1` | `j >= 0` è falso | `v[0]` |
| `x` è già al suo posto | passo 4, `x = 8` | `j = 3` (`v[3] = 6`), nessuno shift | `6 > 8` è falso | `v[4]`, la sua cella di partenza |

Nel secondo caso si vede perché non si può scrivere `v[j] = x`: con `j = -1` si scriverebbe fuori dall'array.

### L'errore tipico: `v[j] = x`

Nel passo 6, dopo gli shift l'array è `1 3 3 5 6 7 8 | 4`, con `j = 0` e il buco in `v[1]`. Scrivendo `v[j] = x` il 2 finisce in `v[0]`: l'1 viene cancellato e in `v[1]` resta la copia inutile del 3. Il risultato è `2 3 3 5 6 7 8`, che ha perso l'1. Con `v[j + 1] = x` invece si ottiene `1 2 3 5 6 7 8`.

Regola da ricordare: **`j` si ferma sull'ultimo elemento da lasciare dov'è; `x` va subito sopra, in `j + 1`.**

Per la teoria completa degli shift e dell'inserimento ordinato vedi la *Dispensa: gli shift negli array*.

<a id="s5"></a>

## 5. Il programma completo e le varianti

```cpp
#include <iostream>
using namespace std;

const int N = 8;

void stampa(int v[], int n) {
    for (int i = 0; i < n; i++)
        cout << v[i] << " ";
    cout << endl;
}

int main() {
    int v[N] = {6, 5, 3, 1, 8, 7, 2, 4};

    cout << "Non ordinato: ";
    stampa(v, N);

    for (int i = 1; i < N; i++) {                   // loop esterno: i = separatore
        int x = v[i];
        int j;
        for (j = i - 1; j >= 0 && v[j] > x; j--)    // loop interno: inserimento ordinato
            v[j + 1] = v[j];
        v[j + 1] = x;
    }

    cout << "Ordinato:     ";
    stampa(v, N);
    return 0;
}
```

Output: `6 5 3 1 8 7 2 4` diventa `1 2 3 4 5 6 7 8`.

### Varianti

| Variante | Insieme ordinato | Loop esterno | Loop interno |
| --- | --- | --- | --- |
| j e j+1 (quella sopra) | in basso | `i` da 1 a N-1 | salva `x`, il buco è `v[j+1]`: shift `v[j+1] = v[j]`, poi `v[j+1] = x` |
| j e j-1 | in basso | `i` da 1 a N-1 | salva `a`, il buco è `v[j]`: shift `v[j] = v[j-1]` finché `a < v[j-1]`, poi `v[j] = a` |
| Ordinati in alto | in alto | `i` da N-2 a 0 | parte da `j = i`, scambia `v[j]` e `v[j+1]` finché `v[j] > v[j+1]` |

La variante con gli ordinati in alto, così come è scritta qui, usa gli scambi: è più semplice da scrivere ma fa più assegnamenti, perché ogni scambio ne richiede tre e lo shift uno solo.

Nella variante con gli ordinati in alto i ruoli si invertono: l'insieme ordinato è `v[i+1..N-1]`, il separatore `i` scende, e ogni nuovo elemento viene fatto **salire** al suo posto.

### La variante con j e j-1, solo shift

Anche con `j` e `j-1` si usa lo shift, non gli scambi. La differenza è che qui `j` indica direttamente il **buco**: il buco è `v[j]`, e il valore da inserire, salvato in `a`, si confronta con il vicino di sotto `v[j-1]`.

```cpp
for (i = 1; i < n; i++) {
    a = v[i];                                  // v[i] diventa il buco
    for (j = i; j - 1 >= 0 && a < v[j - 1]; j--)
        v[j] = v[j - 1];                       // shift a destra, il buco scende in v[j-1]
    v[j] = a;                                  // inserisco a nel buco
}
```

<p align="center">
  <img src="img/insertion_sort_j_meno_1.gif" alt="Animazione: insertion sort con j e j-1, solo shift: il buco è v[j]">
</p>

Nell'animazione il buco è in arancione con l'etichetta `v[j]`, la freccia indica `j` e in azzurro, con l'etichetta `v[j - 1]`, c'è il vicino con cui si confronta `a`. Su 6 5 3 1 8 7 2 4 servono 16 shift, come nella versione con `j` e `j+1`.

| | Versione con `j` e `j+1` | Versione con `j` e `j-1` |
| --- | --- | --- |
| Partenza | `j = i - 1` | `j = i` |
| Il buco è | `v[j + 1]` | `v[j]` |
| Confronto | `v[j] > x` | `a < v[j - 1]` |
| Controllo sul limite | `j >= 0` | `j - 1 >= 0`, cioè `j > 0` |
| Shift | `v[j + 1] = v[j]` | `v[j] = v[j - 1]` |
| Inserimento | `v[j + 1] = x` | `v[j] = a` |

È lo stesso algoritmo scritto con un indice diverso: la `j` della versione con `j-1` vale sempre uno in più di quella della versione con `j+1`. Fanno gli stessi confronti e gli stessi shift.

#### Fine corsa e corto circuito nella versione con `j-1`

Qui la condizione per continuare è `j - 1 >= 0`, cioè `j > 0`: il ciclo esce con **`j = 0`**, non con -1. Con `j = 1` il controllo è vero; se anche `a < v[0]`, si esegue lo shift `v[1] = v[0]` e poi `j--` porta `j` a 0. Il controllo successivo `0 - 1 >= 0` è falso e il ciclo esce; l'inserimento `v[j] = a` scrive in `v[0]`.

Anche qui vale la **valutazione a corto circuito**: con `j = 0` la prima condizione è falsa, quindi `a < v[j - 1]` non viene valutata e `v[-1]` non viene mai letto. L'ordine conta allo stesso modo:

```cpp
j - 1 >= 0 && a < v[j - 1]    // corretto: con j = 0 si ferma prima di leggere v[-1]
a < v[j - 1] && j - 1 >= 0    // sbagliato: con j = 0 legge v[-1], fuori dall'array
```

#### I tre casi di blocco nella versione con `j-1`

Anche qui il ciclo si può fermare in tre modi, e la stessa istruzione `v[j] = a` funziona in tutti. Gli esempi sono gli stessi passi 6, 3 e 4 della versione con `j+1`:

| Caso | Esempio | Dove si ferma `j` | Perché si ferma | `a` va in |
| --- | --- | --- | --- | --- |
| Trova un elemento non maggiore di `a` | passo 6, `a = 2` | `j = 1` (`v[0] = 1`) | `2 < 1` è falso | `v[1]` |
| `a` è il più piccolo di tutti | passo 3, `a = 1` | `j = 0` | `0 - 1 >= 0` è falso | `v[0]` |
| `a` è già al suo posto | passo 4, `a = 8` | `j = 4` (`v[3] = 6`), nessuno shift | `8 < 6` è falso | `v[4]`, la sua cella di partenza |

In ogni caso `j` vale uno in più che nella versione con `j+1` (1 invece di 0, 0 invece di -1, 4 invece di 3), perché qui `j` indica il buco e non l'elemento confrontato. L'animazione mostra i tre casi uno dopo l'altro: nel momento del blocco compaiono la freccia rossa su `j` e la condizione `j - 1 >= 0 && a < v[j - 1]` calcolata con i valori.

<p align="center">
  <img src="img/tre_casi_blocco_j_meno_1.gif" alt="I tre casi di blocco nella versione con j e j-1: passi 6, 3 e 4">
</p>
