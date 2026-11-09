# Dispensa: gli shift negli array

9 novembre 2026

## 1. Che cos'è uno shift

Uno **shift** (scorrimento) sposta tutti i valori di un array di una posizione, verso destra o verso sinistra. Ogni valore viene copiato nella cella vicina: `v[i]` finisce in `v[i+1]` (shift a destra) oppure in `v[i-1]` (shift a sinistra).

In questa dispensa usiamo queste convenzioni:

- **basso** = inizio dell'array, indici piccoli (`v[0]`, `v[1]`, …);
- **alto** = fine dell'array, indici grandi (`v[N-1]`, `v[N-2]`, …);
- **destra** = verso l'alto; **sinistra** = verso il basso.

Quando si fa uno shift succedono sempre due cose:

1. un valore **esce** dall'array dal lato verso cui si sposta (va perso);
2. un posto dal lato opposto resta **scoperto**, cioè senza un nuovo valore.

Cosa mettere nei posti scoperti distingue i due tipi di shift:

| Tipo | Posti scoperti | Valori che escono |
| --- | --- | --- |
| Shift con valore di default | riempiti con un valore fisso (ad es. 0) | persi |
| Shift circolare | riempiti con i valori usciti dall'altro lato | rientrano, nessuno si perde |

In tutti gli esempi l'array è `int v[N]` con `N = 8`.

## 2. Shift a destra con valore di default

**Regola: lo shift a destra si fa partendo dall'alto** (dall'ultimo indice verso il primo).

Esempio: shift a destra di `k = 1` posizione, posto scoperto riempito con 0.

| Indice | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Prima | 6 | 5 | 3 | 1 | 8 | 7 | 2 | 4 |
| Dopo | **0** | 6 | 5 | 3 | 1 | 8 | 7 | 2 |

<p align="center">
  <img src="img/shift_destra_1.gif" alt="Animazione: shift a destra di 1 con zero, partendo dall'alto">
</p>

Il valore 4 esce dall'alto e va perso; il posto 0 è scoperto e riceve 0.

### Perché si parte dall'alto

Ogni copia `v[i] = v[i-1]` **sovrascrive** `v[i]`. Partendo dall'alto, la cella sovrascritta è già stata copiata più su, quindi il suo valore non serve più.

Se invece si partisse dal basso, `v[1] = v[0]` cancellerebbe il 5 prima che venga copiato in `v[2]`. Poi `v[2] = v[1]` copierebbe di nuovo il 6, e così via fino in cima. Alla fine il 6 sarebbe copiato su tutto l'array: 0 6 6 6 6 6 6 6.

L'animazione mostra l'errore: le celle in rosso alla fine sono i valori sbagliati.

<p align="center">
  <img src="img/shift_destra_1_errore.gif" alt="Animazione dell'errore: shift a destra partendo dal basso">
</p>

### Codice C++

```cpp
// 1) copia dall'alto verso il basso
for (int i = N - 1; i > 0; i--)
    v[i] = v[i - 1];

// 2) riempimento del posto scoperto
v[0] = 0;
```

Il ciclo si ferma a `i = 1`: con `i = 0` si leggerebbe `v[-1]`, fuori dall'array.

## 3. Shift a sinistra con valore di default

**Regola: lo shift a sinistra si fa partendo dal basso** (dal primo indice verso l'ultimo). È la regola simmetrica di quella dello shift a destra.

Esempio: shift a sinistra di `k = 1` posizione, posto scoperto riempito con 0.

| Indice | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Prima | 6 | 5 | 3 | 1 | 8 | 7 | 2 | 4 |
| Dopo | 5 | 3 | 1 | 8 | 7 | 2 | 4 | **0** |

<p align="center">
  <img src="img/shift_sinistra_1.gif" alt="Animazione: shift a sinistra di 1 con zero, partendo dal basso">
</p>

Il valore 6 esce dal basso e va perso; il posto 7 è scoperto e riceve 0.

Anche qui il motivo è lo stesso: la copia `v[i] = v[i+1]` sovrascrive `v[i]`, e partendo dal basso quella cella è già stata copiata più giù (oppure è il valore che deve uscire).

Se invece si partisse dall'alto, `v[6] = v[7]` cancellerebbe il 2 prima che venga copiato in `v[5]`. Poi `v[5] = v[6]` copierebbe di nuovo il 4, e così via fino in fondo. Alla fine il 4 sarebbe copiato su tutto l'array: 4 4 4 4 4 4 4 0.

<p align="center">
  <img src="img/shift_sinistra_1_errore.gif" alt="Animazione dell'errore: shift a sinistra partendo dall'alto">
</p>

### Codice C++

```cpp
// 1) copia dal basso verso l'alto
for (int i = 0; i < N - 1; i++)
    v[i] = v[i + 1];

// 2) riempimento del posto scoperto
v[N - 1] = 0;
```

Il ciclo si ferma a `i = N - 2`: con `i = N - 1` si leggerebbe `v[N]`, fuori dall'array.

### Regola da ricordare

| Shift | Da dove parte il ciclo | Copia |
| --- | --- | --- |
| A destra | dall'alto (`i = N-1` e scende fino a 1) | `v[i] = v[i-1]` |
| A sinistra | dal basso (`i = 0` e sale fino a N-2) | `v[i] = v[i+1]` |

In entrambi i casi **si parte dal lato verso cui i valori si spostano**.

## 4. Shift di più posizioni

Uno shift a destra di 3 posizioni, con i posti scoperti riempiti con 0, trasforma 6 5 3 1 8 7 2 4 in **0 0 0 6 5 3 1 8**. Si può ottenere in due modi.

### Primo modo: tre shift di una posizione

Uno shift di `k` posizioni equivale a `k` shift di una posizione eseguiti uno dopo l'altro. Si riusa così il codice già visto, chiudendolo in un ciclo che lo ripete 3 volte.

| Situazione | Array |
| --- | --- |
| Iniziale | 6 5 3 1 8 7 2 4 |
| Dopo il 1° shift di 1 | 0 6 5 3 1 8 7 2 |
| Dopo il 2° shift di 1 | 0 0 6 5 3 1 8 7 |
| Dopo il 3° shift di 1 | 0 0 0 6 5 3 1 8 |

<p align="center">
  <img src="img/shift_destra_3_come_3_shift_di_1.gif" alt="Animazione: shift a destra di 3 come tre shift di 1">
</p>

```cpp
for (int volta = 0; volta < 3; volta++) {
    for (int i = N - 1; i > 0; i--)   // shift di una posizione, dall'alto
        v[i] = v[i - 1];
    v[0] = 0;                         // posto scoperto
}
```

È semplice da capire, ma lavora molto: a ogni ripetizione ricopia quasi tutto l'array. Con `N = 8` fa 3 × 7 = 21 copie più 3 zeri, cioè **24 assegnamenti**.

### Secondo modo: shift diretto di k posizioni

Questa è la soluzione usata nei testi di algoritmi: ogni valore salta **direttamente** di `k` posizioni con un'unica copia, `v[i] = v[i-k]`. La regola resta la stessa: lo shift a destra parte dall'alto.

```cpp
const int K = 3;

// 1) ogni valore salta di K posizioni, partendo dall'alto
for (int i = N - 1; i >= K; i--)
    v[i] = v[i - K];

// 2) i K posti scoperti in basso ricevono 0
for (int i = 0; i < K; i++)
    v[i] = 0;
```

<p align="center">
  <img src="img/shift_destra_3_diretto.gif" alt="Animazione: shift a destra di 3 diretto">
</p>

Con `N = 8` fa 5 copie più 3 zeri, cioè **8 assegnamenti**: uno per cella. Lo shift a sinistra di `K` è simmetrico: si parte dal basso con `v[i] = v[i+K]` per `i` da 0 a `N-K-1`, poi `v[N-K] … v[N-1]` ricevono 0.

| Metodo | Assegnamenti (N = 8, K = 3) | In generale |
| --- | --- | --- |
| K shift di una posizione | 24 | circa K × N |
| Shift diretto di K | 8 | N, qualunque sia K |

Il costo dello shift diretto non cresce con `K`: per questo è il metodo preferito. Se `K >= N` tutti i valori escono e l'array diventa tutto 0.

### E lo shift circolare di k posizioni?

Nella rotazione nessun valore deve andare perso, quindi la copia diretta `v[i] = v[i-k]` non basta: i `k` valori che escono andrebbero salvati in un array d'appoggio. La tecnica classica che evita l'array d'appoggio è quella delle **tre inversioni**, resa celebre da Jon Bentley in *Programming Pearls* (colonna 2). Per ruotare a destra di `K`:

1. si inverte tutto l'array;
2. si invertono i primi `K` elementi;
3. si invertono i restanti `N - K`.

| Passo | Array |
| --- | --- |
| Iniziale | 6 5 3 1 8 7 2 4 |
| 1) Inverto tutto | 4 2 7 8 1 3 5 6 |
| 2) Inverto i primi 3 | 7 2 4 8 1 3 5 6 |
| 3) Inverto gli altri 5 | 7 2 4 6 5 3 1 8 |

<p align="center">
  <img src="img/rotazione_3_tre_inversioni.gif" alt="Animazione: rotazione a destra di 3 con tre inversioni">
</p>

```cpp
void inverti(int v[], int da, int a) {
    for (int i = da, j = a; i < j; i++, j--) {
        int t = v[i];
        v[i] = v[j];
        v[j] = t;
    }
}

inverti(v, 0, N - 1);   // 1) tutto
inverti(v, 0, K - 1);   // 2) primi K
inverti(v, K, N - 1);   // 3) restanti
```

Ogni elemento viene scambiato al massimo due volte, quindi anche qui il costo è proporzionale a `N` e non dipende da `K`.

## 5. Shift circolari

Nello **shift circolare** (o rotazione) i valori che escono da un lato rientrano dall'altro: nessun valore si perde e i posti scoperti vengono riempiti proprio con loro.

La tecnica per una posizione ha tre passi:

1. **salvare** in una variabile il valore che sta per uscire;
2. fare lo **shift normale** di una posizione, con la regola della sezione precedente;
3. **rimettere** il valore salvato nel posto scoperto.

### Shift circolare a destra di 1

Esce `v[N-1]`, che rientra in `v[0]`.

| Indice | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Prima | 6 | 5 | 3 | 1 | 8 | 7 | 2 | 4 |
| Dopo | **4** | 6 | 5 | 3 | 1 | 8 | 7 | 2 |

<p align="center">
  <img src="img/shift_circolare_destra.gif" alt="Animazione: shift circolare a destra di 1">
</p>

```cpp
int ultimo = v[N - 1];          // 1) salvo chi esce
for (int i = N - 1; i > 0; i--) // 2) shift dall'alto
    v[i] = v[i - 1];
v[0] = ultimo;                  // 3) rientra in basso
```

### Shift circolare a sinistra di 1

Esce `v[0]`, che rientra in `v[N-1]`.

| Indice | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Prima | 6 | 5 | 3 | 1 | 8 | 7 | 2 | 4 |
| Dopo | 5 | 3 | 1 | 8 | 7 | 2 | 4 | **6** |

<p align="center">
  <img src="img/shift_circolare_sinistra.gif" alt="Animazione: shift circolare a sinistra di 1">
</p>

```cpp
int primo = v[0];               // 1) salvo chi esce
for (int i = 0; i < N - 1; i++) // 2) shift dal basso
    v[i] = v[i + 1];
v[N - 1] = primo;               // 3) rientra in alto
```

## 6. Applicazione: l'inserimento ordinato

**Definizione.** L'*inserimento ordinato* è l'operazione che aggiunge un nuovo valore `x` a una sequenza **già ordinata** di `n` elementi, collocandolo nella posizione corretta, in modo che dopo l'inserimento la sequenza di `n+1` elementi sia ancora ordinata.

L'array deve avere almeno una cella libera in alto (cioè `n < N`), perché la sequenza cresce di un elemento.

### Le fasi

1. **Ricerca del posto giusto**: si trova l'indice `p` del primo elemento maggiore di `x`. Se nessun elemento è maggiore, `p = n` (`x` va in coda).
2. **Shift a destra** di una posizione degli elementi da `v[p]` a `v[n-1]`, **partendo dall'alto**. Così il posto `v[p]` resta libero.
3. **Inserimento**: si scrive `x` nel posto liberato, `v[p] = x`.
4. **Aggiornamento della dimensione**: `n++`.

### Esempio

Sequenza ordinata 1 3 5 6 8 (`n = 5`), valore da inserire `x = 4`.

| Fase | 0 | 1 | 2 | 3 | 4 | 5 |
| --- | --- | --- | --- | --- | --- | --- |
| Situazione iniziale | 1 | 3 | 5 | 6 | 8 | – |
| 1) Ricerca: primo maggiore di 4 è 5, quindi `p = 2` | 1 | 3 | **5** | 6 | 8 | – |
| 2) Shift a destra di `v[2..4]` | 1 | 3 | \_ | 5 | 6 | 8 |
| 3) Inserimento `v[2] = 4` | 1 | 3 | **4** | 5 | 6 | 8 |

<p align="center">
  <img src="img/inserimento_ordinato.gif" alt="Animazione: inserimento ordinato di 4 nella sequenza 1 3 5 6 8">
</p>

### Codice C++ con le fasi separate

```cpp
// 1) ricerca del posto giusto
int p = 0;
while (p < n && v[p] <= x)
    p++;

// 2) shift a destra di una posizione, dall'alto
for (int i = n; i > p; i--)
    v[i] = v[i - 1];

// 3) inserimento
v[p] = x;

// 4) aggiornamento della dimensione
n++;
```

### Versione compatta: ricerca e shift insieme

La ricerca si può fare anche **dall'alto**, scorrendo la sequenza da `v[n-1]` verso il basso. Ogni elemento maggiore di `x` incontrato viene subito spostato a destra; ci si ferma al primo elemento non maggiore. Le fasi 1 e 2 diventano un solo ciclo.

```cpp
int j = n - 1;
while (j >= 0 && v[j] > x) {
    v[j + 1] = v[j];   // shift a destra
    j--;               // ricerca verso il basso
}
v[j + 1] = x;          // inserimento
n++;
```

È esattamente il ciclo interno dell'**insertion sort**: l'ordinamento ripete l'inserimento ordinato per ogni elemento dell'array (vedi la dispensa sull'insertion sort).

Usare `>` e non `>=` rende l'inserimento **stabile**: un valore uguale a uno già presente viene messo dopo di esso.

## 7. Errori tipici

| Errore | Effetto | Correzione |
| --- | --- | --- |
| Shift a destra partendo dal basso | il primo valore viene copiato su tutto l'array | far partire il ciclo da `N-1` e scendere |
| Shift a sinistra partendo dall'alto | l'ultimo valore viene copiato su tutto l'array | far partire il ciclo da `0` e salire |
| Limiti del ciclo sbagliati | lettura di `v[-1]` o `v[N]`, fuori dall'array | a destra `i > 0`, a sinistra `i < N-1` |
| Shift circolare senza salvare il valore | il valore che esce va perso | salvarlo in una variabile **prima** dello shift |
| Inserimento ordinato in un array pieno | scrittura di `v[N]` | controllare `n < N` prima di inserire |
| Condizione `v[j] > x && j >= 0` | legge `v[-1]` quando `j` arriva a -1 | mettere prima il controllo sul limite: `j >= 0 && v[j] > x` |
