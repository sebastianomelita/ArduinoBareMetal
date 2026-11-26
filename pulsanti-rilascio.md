>[Torna all'indice](indexpulsanti.md)   >[versione in Python](pulsanti-rilascio-mpy.md)

# Pulsanti: attivazione sul rilascio, tecniche non bloccanti (pull-down e pull-up)

In entrambe le versioni l'azione principale parte quando il pulsante viene **rilasciato**.
Cambia solo il livello letto dal pin, che dipende dal collegamento della resistenza.

| Collegamento | Riposo | Premuto | Pressione | Rilascio |
| --- | --- | --- | --- | --- |
| Pull-down (resistenza verso GND) | LOW | HIGH | fronte di salita | fronte di discesa |
| Pull-up (resistenza verso Vcc o `INPUT_PULLUP`) | HIGH | LOW | fronte di discesa | fronte di salita |

In ogni esempio ci sono due callback: in quella della **pressione** vanno le azioni immediate
alla pressione (facoltative, qui lasciata vuota); in quella del **rilascio** vanno le azioni
al rilascio (qui, come esempio, un toggle del LED).

> Perché si agisce al rilascio, le eccezioni e quale metodo usare nelle situazioni reali: [debounce-fenomeni.md, sezioni 5 e 6](debounce-fenomeni.md#5-pressione-o-rilascio-quando-agire)
> Le quattro situazioni critiche di un debouncer (rimbalzi alla pressione e al rilascio, buchi, spike) e come si sceglie il tempo di antirimbalzo: [debounce-fenomeni.md](debounce-fenomeni.md)
> Versioni corte, con la sola azione al rilascio: [pulsanti-rilascio-corte.md](pulsanti-rilascio-corte.md)
> Esempi con due task concorrenti, toggle e blink (pull-down): [toggle-blink-pulldown.md](toggle-blink-pulldown.md)
> Tecniche bloccanti (waitUntilInputLow / waitUntilInputHigh), dalla più semplice alla più robusta: [pulsanti-bloccanti.md](pulsanti-bloccanti.md)

## 1. Versione non bloccante con millis()

> **Rileva:** rimbalzi alla pressione e al rilascio, buco a pulsante premuto. **Non rileva:** spike a riposo (pressione e rilascio falsi).
>
> **Prontezza** sulla pressione, riconosciuta alla prima lettura alta. **Sicurezza** sul rilascio, confermato da 50 ms di livello basso: l'azione arriva 50 ms dopo l'ultimo rimbalzo.
>
> **Applicazioni tipiche:** pulsanti di pannelli e menu dove serve una risposta pronta alla pressione, toggle con pulsanti in buono stato e cavi corti: è la scelta di base delle esercitazioni. Nelle tastiere con firmware QMK esiste come modalità asimmetrica: pressione subito, rilascio confermato.

<p align="center">
  <img src="img/debounce-millis-pulldown.gif" alt="Animazione del debounce con millis() (pull-down)" width="900">
</p>

Il programma usa due variabili:
- `t0`, l'istante dell'**ultima** lettura "premuto";
- `premutoRecente`, vera se c'è stata una lettura "premuto" **negli ultimi 50 ms**. Diventa vera alla
  prima lettura "premuto" (ed è lì che parte la callback della pressione), resta vera finché le letture
  "premuto" continuano ad arrivare, anche separate da rimbalzi o da buchi brevi, e torna falsa solo
  dopo 50 ms senza letture "premuto" (ed è lì che parte la callback del rilascio). Non vuol dire
  "premuto adesso": durante un rimbalzo o un buco il pin può leggere "rilasciato" mentre
  `premutoRecente` è ancora vera. Chi conosce i PLC la riconosce: è l'uscita di un **timer TOF**
  (ritardo alla disattivazione) da 50 ms con il pin come ingresso.

Con il pull-up il comportamento è identico con i livelli invertiti: il pulsante premuto legge LOW, la pressione è un fronte di discesa e il rilascio un fronte di salita.

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```cpp
const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
unsigned long t0 = 0;
bool premutoRecente = false;

void doOnRise() {
  // PRESSIONE (fronte di salita):
  // qui vanno le azioni immediate
  // alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa):
  // qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
}

void loop() {
  if (digitalRead(P1) == HIGH) {
    if (!premutoRecente) {
      premutoRecente = true;
      doOnRise();
    }
    t0 = millis();
  } else if (premutoRecente &&
             millis() - t0 > 50) {
    premutoRecente = false;
    doOnFall();
  }
}
```

</td>
<td>

```cpp
const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
unsigned long t0 = 0;
bool premutoRecente = false;

void doOnFall() {
  // PRESSIONE (fronte di discesa):
  // qui vanno le azioni immediate
  // alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnRise() {
  // RILASCIO (fronte di salita):
  // qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

void loop() {
  if (digitalRead(P1) == LOW) {
    if (!premutoRecente) {
      premutoRecente = true;
      doOnFall();
    }
    t0 = millis();
  } else if (premutoRecente &&
             millis() - t0 > 50) {
    premutoRecente = false;
    doOnRise();
  }
}
```

</td>
</tr>
</table>

`t0` si aggiorna a ogni lettura "premuto": un rimbalzo al rilascio azzera il conteggio
e non produce un falso fronte.

## 2. Versione con schedulatore non bloccante

> **Rileva:** rimbalzi alla pressione e al rilascio. **Solo in parte:** buco e spike, filtrati solo se nessuna lettura ci cade dentro.
>
> Né prontezza né conferma: pressione e rilascio arrivano alla prima lettura che li vede, in media 25 ms dopo. **Sicurezza** statistica, che migliora chiedendo N letture uguali.
>
> **Applicazioni tipiche:** molti pulsanti o ingressi letti insieme a intervalli fissi, programmi già organizzati a tempo base; i videogiochi su PC e console leggono i tasti allo stesso modo, una volta per fotogramma, ma su segnali già filtrati dal firmware del controller: il confronto con il fotogramma precedente serve solo a trovare i fronti.

Il codice di gestione del pulsante gira una volta ogni `TBASE` ms. Leggere il pin a
intervalli regolari fa già da debounce: se `TBASE` è più lungo dei rimbalzi, tra due
letture il segnale si è stabilizzato. I fronti si riconoscono confrontando la lettura
attuale `val` con quella precedente `precval`.

<p align="center">
  <img src="img/debounce-schedulatore-pulldown.gif" alt="Animazione del debounce con schedulatore (pull-down)" width="900">
</p>

Con il pull-up il comportamento è identico con i livelli invertiti: il pulsante premuto legge LOW, la pressione è un fronte di discesa e il rilascio un fronte di salita.

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```cpp
#define TBASE 50  // ms, > rimbalzi

const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
unsigned long precm = 0;
byte precval = LOW;  // riposo
byte val;

void doOnRise() {
  // PRESSIONE (fronte di salita):
  // qui vanno le azioni immediate
  // alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa):
  // qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
}

void loop() {
  if (millis() - precm >= TBASE) {
    precm = millis();
    val = digitalRead(P1);
    if (precval == LOW &&
        val == HIGH) {   // salita
      doOnRise();
    } else if (precval == HIGH &&
               val == LOW) { // discesa
      doOnFall();
    }
    precval = val;
  }
}
```

</td>
<td>

```cpp
#define TBASE 50  // ms, > rimbalzi

const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
unsigned long precm = 0;
byte precval = HIGH; // riposo
byte val;

void doOnFall() {
  // PRESSIONE (fronte di discesa):
  // qui vanno le azioni immediate
  // alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnRise() {
  // RILASCIO (fronte di salita):
  // qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

void loop() {
  if (millis() - precm >= TBASE) {
    precm = millis();
    val = digitalRead(P1);
    if (precval == HIGH &&
        val == LOW) {    // discesa
      doOnFall();
    } else if (precval == LOW &&
               val == HIGH) { // salita
      doOnRise();
    }
    precval = val;
  }
}
```

</td>
</tr>
</table>

Nelle versioni da 1 a 3 la callback della pressione scatta subito, al primo giro di
`loop()` in cui il pin risulta premuto. Con lo schedulatore invece il pin viene letto solo
ogni `TBASE` ms: la pressione viene riconosciuta con un ritardo fino a `TBASE`, e una
pressione più breve di `TBASE` può cadere tra due campioni e non essere vista.

## 3. Versione non bloccante a livello stabile con changed()

> **Rileva:** tutte e quattro le situazioni: rimbalzi alla pressione e al rilascio, buco a pulsante premuto, spike a riposo.
>
> **Sicurezza** su pressione e rilascio, tutti e due confermati da 50 ms di livello fermo. Nessuna prontezza: ogni evento arriva 50 ms dopo l'ultimo rimbalzo.
>
> **Applicazioni tipiche:** toggle e contatori, start di una macchina, pulsanti su cavi lunghi o vicino a motori, sensori a 24 V che contano pezzi (nei PLC il doppio conteggio si evita con due timer, uno all'attivazione e uno alla disattivazione: è lo stesso principio). Funzionano così anche il filtro dei moduli di ingresso dei PLC e del debounce predefinito (*defer*) delle tastiere con firmware QMK.

Le versioni 1 e 2 accettano la **pressione** alla prima lettura che la vede, senza confermarla.
La 1 conferma solo il **rilascio**. Questa invece accetta un nuovo livello, in **tutte e due** le direzioni, solo quando è
rimasto fermo per più di 50 ms. È la macchina a stati di [debouncerFSM.md](debouncerFSM.md) (forma
"prima gli ingressi"):
- `val0` è l'ultima lettura: ogni variazione (rimbalzo, disturbo o fronte vero) la aggiorna e fa
  ripartire il conteggio da `last`;
- `val00` è il livello accettato: cambia solo se la lettura è diversa da lui e ferma da più di 50 ms;
- `changed()` restituisce `true` una sola volta, nel giro in cui il nuovo livello viene accettato:
  guardando `val00` si sa se è una pressione o un rilascio.

Così viene filtrato anche uno **spike alto a pulsante rilasciato**, ad esempio un disturbo sui cavi
con il pull-down, che la versione 1 prenderebbe sempre per una pressione seguita da un rilascio (la 2 solo se una lettura ci cade sopra).
Il prezzo è che anche la pressione viene riconosciuta con 50 ms di ritardo, e una pressione più
breve di 50 ms viene scartata come se fosse un disturbo.

<p align="center">
  <img src="img/debounce-changed-pulldown.gif" alt="Animazione del debounce a livello stabile con changed() (pull-down)" width="900">
</p>

Con il pull-up il comportamento è identico con i livelli invertiti: riposo HIGH, premuto LOW.

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```cpp
const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
byte val0  = LOW;  // ultima lettura
byte val00 = LOW;  // livello accettato
unsigned long last = 0; // ultima variazione

bool changed() {
  // vero solo quando il pin ha un
  // nuovo livello fermo da più di 50 ms
  byte val = digitalRead(P1);
  if (val != val0) {
    // variazione: rimbalzo, disturbo
    // o fronte vero, si riparte
    val0 = val;
    last = millis();
  } else if (val != val00 &&
             millis() - last > 50) {
    val00 = val;  // livello stabile
    return true;
  }
  return false;
}

void doOnRise() {
  // PRESSIONE (fronte di salita):
  // qui vanno le azioni immediate
  // alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa):
  // qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
}

void loop() {
  if (changed()) {
    if (val00 == HIGH) {
      doOnRise();  // pressione confermata
    } else {
      doOnFall();  // rilascio confermato
    }
  }
}
```

</td>
<td>

```cpp
const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
byte val0  = HIGH;  // ultima lettura
byte val00 = HIGH;  // livello accettato
unsigned long last = 0; // ultima variazione

bool changed() {
  // vero solo quando il pin ha un
  // nuovo livello fermo da più di 50 ms
  byte val = digitalRead(P1);
  if (val != val0) {
    // variazione: rimbalzo, disturbo
    // o fronte vero, si riparte
    val0 = val;
    last = millis();
  } else if (val != val00 &&
             millis() - last > 50) {
    val00 = val;  // livello stabile
    return true;
  }
  return false;
}

void doOnFall() {
  // PRESSIONE (fronte di discesa):
  // qui vanno le azioni immediate
  // alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnRise() {
  // RILASCIO (fronte di salita):
  // qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

void loop() {
  if (changed()) {
    if (val00 == LOW) {
      doOnFall();  // pressione confermata
    } else {
      doOnRise();  // rilascio confermato
    }
  }
}
```

</td>
</tr>
</table>

**La stessa logica con DiffTimer.** Come nella sezione 5 rispetto alla 1, `last` e `millis()`
possono essere sostituiti da un oggetto `DiffTimer`: `debt.reset()` prende il posto di
`last = millis()` e `debt.get()` quello di `millis() - last`. Il comportamento è identico: cambia
solo la scrittura, che nasconde il calcolo del tempo dentro il timer. Il timer va avviato una volta
in `setup()`, altrimenti `get()` resta a zero e nessun livello verrebbe mai accettato.

```cpp
#include "urutils.h"
DiffTimer debt;  // tempo dall'ultima variazione

bool changed() {
  byte val = digitalRead(P1);
  if (val != val0) {
    // variazione: rimbalzo, disturbo
    // o fronte vero, si riparte
    val0 = val;
    debt.reset();   // al posto di last = millis()
  } else if (val != val00 &&
             debt.get() > 50) {
    val00 = val;    // livello stabile
    return true;
  }
  return false;
}

void setup() {
  pinMode(P1, INPUT);   // pull-up: INPUT_PULLUP, val0 e val00 = HIGH
  pinMode(LED, OUTPUT);
  debt.start();         // il timer conta sempre
}
```

Il `loop()` resta quello di sopra.

**La stessa logica è nella libreria.** In `urutils.h` c'è la struct `Debouncer`, con le stesse
variabili `val0`, `val00` e `last` e lo stesso metodo `changed()`: un oggetto per pulsante, creato con
il livello di riposo.

```cpp
#include "urutils.h"
Debouncer deb(LOW);              // pull-down: riposo LOW (con il pull-up: Debouncer deb(HIGH);)

void loop() {
  if (deb.changed(P1, 50)) {     // nuovo livello fermo da più di 50 ms
    if (deb.val00 == HIGH) {
      doOnRise();                // pressione confermata
    } else {
      doOnFall();                // rilascio confermato
    }
  }
}
```

## 4. Senza conferma: il primo cambio è creduto subito

> **Rileva:** rimbalzi alla pressione e al rilascio. **Non rileva:** buco a pulsante premuto (rilascio e pressione falsi) e spike a riposo (pressione e rilascio falsi).
>
> **Prontezza** su pressione e rilascio, tutti e due creduti al primo cambio, senza ritardo. Nessuna sicurezza: non c'è conferma.
>
> **Applicazioni tipiche:** giochi su microcontrollore e altri comandi dove conta la prontezza e un errore raro è tollerato. Nelle tastiere con firmware QMK è la modalità *eager*: bassa latenza, ma nessuna resistenza ai disturbi.

Le versioni 1 e 3 confermano almeno il rilascio, la 2 legge solo ogni 50 ms. Questa non conferma **nessuna** delle due
transizioni: è la tecnica *lockout*, o "trust the first change" del
[Makeability Lab](https://makeabilitylab.github.io/physcomp/arduino/debouncing.html). Ogni cambio di
livello è creduto **alla prima lettura**, l'evento parte subito, e per i 50 ms successivi l'ingresso
viene **ignorato**: i rimbalzi cadono in questa finestra e non vengono visti.

- `stabile` è il livello accettato, come `val00` in `changed()`;
- `t0` è l'istante dell'ultimo cambio accettato;
- il pin è letto **a ogni giro** di `loop()`; un cambio rispetto a `stabile` è creduto **nello stesso
  giro in cui viene letto**, purché siano passati più di 50 ms dall'ultimo cambio accettato.

**Non è uno schedulatore e non è `changed()`.** La condizione `millis() - t0 > 50` sembra quella di uno
schedulatore, ma `t0` non viene aggiornato a ogni lettura: viene aggiornato **solo quando si accetta un
cambio**. A riposo, o a pulsante premuto da tempo, la condizione è sempre vera e il pin è letto a ogni
giro: il primo cambio è visto e creduto **subito**. La finestra di 50 ms si apre solo **dopo** l'evento,
per non vedere i rimbalzi che lo seguono.

| | Quando si legge il pin | Quando un cambio diventa un evento | Ruolo dei 50 ms |
| --- | --- | --- | --- |
| Schedulatore | una volta ogni 50 ms, sempre | alla prima lettura che lo vede (in media 25 ms dopo) | intervallo fra due letture |
| `changed()` (livello stabile) | a ogni giro | dopo 50 ms di livello fermo, cioè 50 ms dopo l'ultimo rimbalzo | **prima** dell'evento: conferma |
| Lockout (questa sezione) | a ogni giro, tranne nei 50 ms dopo un evento | **subito**, alla prima lettura | **dopo** l'evento: finestra in cui i cambi sono ignorati |

<p align="center">
  <img src="img/debounce-confronto-lockout.png" alt="Confronto fra lockout, schedulatore e changed() sullo stesso segnale" width="900">
</p>

La figura mette a confronto le tre tecniche sullo stesso segnale. Il lockout crede subito a ogni cambio e poi si blocca per 50 ms (in grigio): è il più pronto, ma sbaglia sul buco e sullo spike. Lo schedulatore legge una volta ogni 50 ms: riconosce gli eventi veri con un ritardo di 25 ms, e qui evita buco e spike solo perché nessuna lettura ci cade sopra. `changed()` aspetta 50 ms di livello fermo (in azzurro) **prima** di ogni evento: è in ritardo di 50 ms sull'ultimo rimbalzo, ma scarta buco e spike.

<p align="center">
  <img src="img/debounce-lockout-pulldown.gif" alt="Animazione della tecnica lockout: il primo cambio è creduto subito, poi 50 ms di blocco (pull-down)" width="900">
</p>

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```cpp
const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
byte stabile = LOW;      // livello accettato
unsigned long t0 = 0;    // ultimo cambio accettato

void doOnRise() {
  // PRESSIONE (fronte di salita):
  // creduta alla prima lettura HIGH.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa):
  // creduto alla prima lettura LOW.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
}

void loop() {
  byte val = digitalRead(P1);  // letto
                               // a ogni giro
  // un cambio fuori dalla finestra
  // di blocco è creduto SUBITO
  if (val != stabile &&
      millis() - t0 > 50) {
    stabile = val;
    t0 = millis();  // per 50 ms i
                    // cambi sono ignorati
    if (stabile == HIGH) {
      doOnRise();
    } else {
      doOnFall();
    }
  }
}
```

</td>
<td>

```cpp
const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
byte stabile = HIGH;      // livello accettato
unsigned long t0 = 0;    // ultimo cambio accettato

void doOnFall() {
  // PRESSIONE (fronte di discesa):
  // creduta alla prima lettura LOW.
  // Lasciatela vuota se non serve.
}

void doOnRise() {
  // RILASCIO (fronte di salita):
  // creduto alla prima lettura HIGH.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

void loop() {
  byte val = digitalRead(P1);  // letto
                               // a ogni giro
  // un cambio fuori dalla finestra
  // di blocco è creduto SUBITO
  if (val != stabile &&
      millis() - t0 > 50) {
    stabile = val;
    t0 = millis();  // per 50 ms i
                    // cambi sono ignorati
    if (stabile == LOW) {
      doOnFall();
    } else {
      doOnRise();
    }
  }
}
```

</td>
</tr>
</table>

**Cosa garantisce.** I rimbalzi, alla pressione e al rilascio, sono filtrati se durano meno di 50 ms:
in simulazione 0 errori. Ed è la tecnica **più pronta**: pressione e rilascio sono riconosciuti al
primo fronte, senza alcun ritardo (in simulazione il rilascio è arrivato in media 0 ms dopo il primo
fronte).

**Cosa non garantisce.** Non c'è nessuna conferma, quindi:
- un **buco** a pulsante premuto, se cade fuori dalla finestra di blocco, è creduto come un rilascio, e
  50 ms dopo la lettura HIGH è creduta come una nuova pressione: circa 4 errori su 5 in simulazione;
- uno **spike** a riposo è creduto come una pressione, e 50 ms dopo la lettura LOW come un rilascio:
  in simulazione succede sempre (5000 errori su 5000).

**Dove va bene.** Dove conta soprattutto la **prontezza**, in entrambe le direzioni, e un errore raro è
accettabile: i **videogiochi** sono l'esempio tipico, perché non c'è nessun ritardo fra il gesto del
giocatore e la risposta, e se un tasto scatta due volte il giocatore ripete il gesto. Non va bene per
contare pezzi, comandare macchine o dove un pulsante è vecchio o i cavi sono disturbati.

**La stessa logica è nella libreria.** In `urutils.h` c'è la struct `Lockout`, con le stesse
variabili `stato` e `t0` e il metodo `changed()`: un oggetto per pulsante, creato con il livello di
riposo.

```cpp
#include "urutils.h"
Lockout lk(LOW);                 // pull-down: riposo LOW (con il pull-up: Lockout lk(HIGH);)

void loop() {
  if (lk.changed(P1, 50)) {      // cambio creduto subito, poi 50 ms di blocco
    if (lk.stato == HIGH) {
      doOnRise();                // pressione
    } else {
      doOnFall();                // rilascio
    }
  }
}
```

Un uso tipico è il **tasto di un gioco**, dove l'azione deve partire alla pressione senza ritardo:

```cpp
Lockout lk(LOW);                 // un oggetto per pulsante; pull-up: lk(HIGH)

void loop() {
  if (lk.changed(P1, 50)) {      // cambio creduto subito, poi 50 ms di blocco
    if (lk.stato == HIGH) {
      salta();                   // pressione: nessun ritardo
    }
  }
}
```

Per un toggle o un contatore, invece, conviene `Debouncer` (sezione 3): lì un buco o uno spike
lascerebbero un errore permanente.

La variante con `delay(50)` al posto di `t0`, proposta dal Makeability Lab, è **bloccante**: si trova
nella sezione 0 di [pulsanti-bloccanti.md](pulsanti-bloccanti.md).
## 5. Versione non bloccante con DiffTimer

> **Rileva:** rimbalzi alla pressione e al rilascio, buco a pulsante premuto. **Non rileva:** spike a riposo (pressione e rilascio falsi).
>
> **Prontezza** sulla pressione, riconosciuta alla prima lettura alta. **Sicurezza** sul rilascio, confermato da 50 ms di livello basso: l'azione arriva 50 ms dopo l'ultimo rimbalzo.
>
> **Applicazioni tipiche:** pulsanti di pannelli e menu dove serve una risposta pronta alla pressione, toggle con pulsanti in buono stato e cavi corti: è la scelta di base delle esercitazioni. Nelle tastiere con firmware QMK esiste come modalità asimmetrica: pressione subito, rilascio confermato.

<p align="center">
  <img src="img/debounce-difftimer-pulldown.gif" alt="Animazione del debounce con DiffTimer (pull-down)" width="900">
</p>

Con il pull-up il comportamento è identico con i livelli invertiti: il pulsante premuto legge LOW, la pressione è un fronte di discesa e il rilascio un fronte di salita.

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```cpp
#include "urutils.h"

const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
DiffTimer debt;

void doOnRise() {
  // PRESSIONE (fronte di salita):
  // qui vanno le azioni immediate
  // alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa):
  // qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
}

void loop() {
  if (digitalRead(P1) == HIGH) {
    if (!debt.timerState) {
      debt.start();
      doOnRise();
    }
    debt.reset();
  } else if (debt.timerState &&
             debt.get() > 50) {
    debt.stop();
    debt.reset();
    doOnFall();
  }
}
```

</td>
<td>

```cpp
#include "urutils.h"

const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
DiffTimer debt;

void doOnFall() {
  // PRESSIONE (fronte di discesa):
  // qui vanno le azioni immediate
  // alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnRise() {
  // RILASCIO (fronte di salita):
  // qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

void loop() {
  if (digitalRead(P1) == LOW) {
    if (!debt.timerState) {
      debt.start();
      doOnFall();
    }
    debt.reset();
  } else if (debt.timerState &&
             debt.get() > 50) {
    debt.stop();
    debt.reset();
    doOnRise();
  }
}
```

</td>
</tr>
</table>

## 6. Versione non bloccante con waitUntilNB()

> **Rileva:** rimbalzi alla pressione e al rilascio, buco a pulsante premuto. **Non rileva:** spike a riposo (pressione e rilascio falsi).
>
> **Prontezza** sulla pressione, riconosciuta alla prima lettura alta. **Sicurezza** sul rilascio, confermato da 50 ms di livello basso: l'azione arriva 50 ms dopo l'ultimo rimbalzo.
>
> **Applicazioni tipiche:** pulsanti di pannelli e menu dove serve una risposta pronta alla pressione, toggle con pulsanti in buono stato e cavi corti: è la scelta di base delle esercitazioni. Nelle tastiere con firmware QMK esiste come modalità asimmetrica: pressione subito, rilascio confermato.

Il secondo argomento è la condizione "pulsante rilasciato", cioè `!premutoOra`.
`premutoOra` è la lettura di questo giro, grezza: durante un rimbalzo o un buco può valere
"rilasciato" anche se la pressione non è finita. Il ruolo di `premutoRecente` della versione 1
(una lettura "premuto" negli ultimi 50 ms) lo svolge qui `debt.timerState`.
`waitUntilNB()` di `urutils.h` fa ripartire il timer a ogni chiamata con la condizione falsa,
cioè a ogni lettura "premuto": scatta solo dopo 50 ms di pulsante rilasciato senza
interruzioni, quindi i rimbalzi del rilascio vengono ignorati. Non serve chiamare `debt.reset()`
nello sketch, come invece nella versione con DiffTimer: l'azzeramento è già dentro la funzione.
`waitUntilNB()` segnala solo il rilascio: la callback della pressione si chiama controllando
`debt.timerState` prima della chiamata. Il timer non è attivo solo al primo giro di pressione,
perché subito dopo lo attiva `waitUntilNB()`.

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```cpp
#include "urutils.h"

const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
DiffTimer debt;

void doOnRise() {
  // PRESSIONE (fronte di salita):
  // qui vanno le azioni immediate
  // alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa):
  // qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
}

void loop() {
  // pull-down: premuto = HIGH
  bool premutoOra = digitalRead(P1);
  if (premutoOra &&
      !debt.timerState) {
    doOnRise();
  }
  // c = "rilasciato"
  if (waitUntilNB(debt,
        !premutoOra, 50)) {
    doOnFall();
  }
}
```

</td>
<td>

```cpp
#include "urutils.h"

const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
DiffTimer debt;

void doOnFall() {
  // PRESSIONE (fronte di discesa):
  // qui vanno le azioni immediate
  // alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnRise() {
  // RILASCIO (fronte di salita):
  // qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo
  // stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

void loop() {
  // pull-up: premuto = LOW
  bool premutoOra = !digitalRead(P1);
  if (premutoOra &&
      !debt.timerState) {
    doOnFall();
  }
  // c = "rilasciato"
  if (waitUntilNB(debt,
        !premutoOra, 50)) {
    doOnRise();
  }
}
```

</td>
</tr>
</table>


## Considerazioni finali: quale tecnica scegliere

**Il limite di fondo, comune a tutte le tecniche.** Ogni antirimbalzo stabilisce un tempo minimo, ad esempio 50 msec, oltre il quale un'apertura del contatto viene considerata un rilascio vero. Un disturbo che apre il contatto per più di quel tempo non si distingue da un rilascio breve: vale per il polling, per lo schedulatore e per gli interrupt. Nel seguito si dà per scontato che questo limite sia superato scegliendo un tempo adatto alla qualità del pulsante: abbastanza lungo da coprire i suoi rimbalzi e i suoi disturbi, abbastanza corto da non perdere le pressioni rapide.

**Il polling veloce con il timer che riparte è la tecnica più pronta fra quelle robuste ai rimbalzi.** (Contro uno spike a pulsante rilasciato serve invece il livello stabile, `changed()` o `Debouncer` di `urutils`.) È la versione in cui il ```loop()``` legge il pulsante a ogni giro e il timer riparte a ogni lettura "premuto" (```t0 = millis()``` fuori dall'```if```):
- guarda il pulsante in continuazione: un disturbo breve a pulsante premuto viene visto di sicuro, ma fa solo ripartire il conteggio;
- il rilascio è accettato solo se il pin è rimasto basso per **tutto** il tempo di antirimbalzo: la decisione non dipende da dove cade il disturbo;
- la pressione è riconosciuta alla prima lettura alta, il rilascio dopo il tempo di antirimbalzo.

La condizione è che il ```loop()``` resti veloce: con ```delay()``` o funzioni lente le letture si diradano e il polling si comporta come uno schedulatore.

**Lo schedulatore (time tick)** legge il pulsante solo ogni ```TBASE```. È immune ai rimbalzi perché tra una lettura e l'altra non li vede, e per lo stesso motivo di solito non vede nemmeno i disturbi brevi a pulsante premuto; se però una lettura cade proprio dentro un disturbo, il toggle scatta due volte (la probabilità è circa la durata del disturbo divisa per ```TBASE```). Si può ridurre il rischio accettando un nuovo livello solo dopo N letture uguali consecutive: il tempo minimo di un rilascio vero diventa N × ```TBASE```. È un'approssimazione, con il campionamento rado, di ciò che il polling veloce fa con il campionamento continuo.

**Il livello stabile (`changed()`)** fa la stessa cosa del polling veloce, ma in **tutte e due** le direzioni: anche la pressione viene accettata solo dopo il tempo di antirimbalzo di livello fermo. Il polling veloce, gli interrupt e le tecniche bloccanti accettano la pressione alla prima lettura alta, quindi uno spike alto a pulsante rilasciato (tipico del pull-down con cavi lunghi, perché a riposo il pin è tenuto basso solo dalla resistenza) diventa una pressione e un rilascio falsi; il livello stabile lo scarta. Il prezzo è un ritardo pari al tempo di antirimbalzo anche sulla pressione, e la perdita delle pressioni più brevi di quel tempo. È la scelta giusta quando l'ingresso arriva da lontano o in un ambiente disturbato, ad esempio i sensori di un nastro trasportatore.

| Tecnica | Rimbalzi | Disturbo breve a pulsante premuto | Spike alto a pulsante rilasciato |
| --- | --- | --- | --- |
| millis(), DiffTimer, waitUntilNB(), interrupt, tecniche bloccanti | filtrati | filtrato | **non filtrato** |
| Schedulatore | filtrati | filtrato solo se nessuna lettura ci cade dentro | filtrato solo se nessuna lettura ci cade dentro |
| Livello stabile (`changed()`) | filtrati | filtrato | filtrato |
| Senza conferma (lockout, sezione 4) | filtrati | **non filtrato** | **non filtrato** |

**Gli interrupt su entrambi i fronti** fanno la stessa cosa del polling veloce: vedono ogni fronte, rimbalzi e disturbi compresi, e fanno ripartire il conteggio del tempo di antirimbalzo. In più funzionano anche quando il ```loop()``` è lento: sono la scelta giusta quando il ciclo principale ha altro di pesante da fare.

## Riferimenti

- [urutils.h](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/urutils.h): sorgente della libreria
- [toggle.md](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/toggle.md): esempi di pulsante toggle
- [pulsanti-rilascio-corte.md](pulsanti-rilascio-corte.md): versioni corte, solo azione al rilascio
- [pulsanti-bloccanti.md](pulsanti-bloccanti.md): tecniche bloccanti
- [toggle-blink-pulldown.md](toggle-blink-pulldown.md): toggle e blink come due task concorrenti
