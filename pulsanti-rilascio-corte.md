>[Torna all'indice](indexpulsanti.md)   >[versione in Python](pulsanti-rilascio-corte-mpy.md)

# Pulsanti: versioni corte, solo azione al rilascio

Queste versioni hanno solo la callback del **rilascio**: `doOnFall()` nel pull-down,
`doOnRise()` nel pull-up. Senza azione alla pressione il codice si accorcia.

> Versioni complete, con azione alla pressione e al rilascio: [pulsanti-rilascio.md](pulsanti-rilascio.md)
> Esempi con due task concorrenti, toggle e blink (pull-down): [toggle-blink-pulldown.md](toggle-blink-pulldown.md)

Per riconoscere il rilascio bisogna comunque ricordare che prima il pulsante era premuto.
Nella maggior parte delle tecniche questa memoria c'è già, senza variabili booleane in più:

| Tecnica | Dove sta la memoria "era premuto" | Bool in più |
| --- | --- | --- |
| Bloccante | nel punto del programma: si è dentro l'`if` | no |
| millis() | variabile `attivo` | sì |
| DiffTimer | `timerState`, dentro la libreria | no |
| waitUntilNB() | `timerState`, dentro la libreria (serve `premuto` per il reset) | no |
| Schedulatore | `precval`, il campione precedente | no |

## 1. Versione bloccante

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

void waitUntilInputLow(int btn, unsigned t) {
  // come in urutils.h: attende il LOW,
  // poi altri t ms, e ricontrolla:
  // esce solo con un rilascio confermato
  do {
    do {
      delay(t);
    } while (digitalRead(btn) != LOW);
    delay(t);  // fine rimbalzi
  } while (digitalRead(btn) != LOW);
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
    waitUntilInputLow(P1, 50);
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

void waitUntilInputHigh(int btn, unsigned t) {
  // come in urutils.h: attende l'HIGH,
  // poi altri t ms, e ricontrolla:
  // esce solo con un rilascio confermato
  do {
    do {
      delay(t);
    } while (digitalRead(btn) != HIGH);
    delay(t);  // fine rimbalzi
  } while (digitalRead(btn) != HIGH);
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
    waitUntilInputHigh(P1, 50);
    doOnRise();
  }
}
```

</td>
</tr>
</table>

## 2. Versione non bloccante con millis()

Qui la variabile `attivo` serve: senza, a pulsante rilasciato `millis() - t0 > 50` resterebbe
vera per sempre e l'azione scatterebbe a ogni giro di `loop()`.

<p align="center">
  <img src="img/debounce-millis-pulldown.gif" alt="Animazione del debounce con millis() (pull-down)" width="900">
</p>

Con il pull-up il comportamento è identico con i livelli invertiti: il pulsante premuto legge LOW, la pressione è un fronte di discesa e il rilascio un fronte di salita.

L'animazione mostra la versione completa della dispensa principale, con `doOnRise()` e il controllo su `attivo` alla pressione; qui quella parte manca, ma rimbalzi, disturbo e conferma del rilascio si comportano allo stesso modo.

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
bool attivo = false;

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
    attivo = true;
    t0 = millis();
  } else if (attivo &&
             millis() - t0 > 50) {
    attivo = false;
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
bool attivo = false;

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
    attivo = true;
    t0 = millis();
  } else if (attivo &&
             millis() - t0 > 50) {
    attivo = false;
    doOnRise();
  }
}
```

</td>
</tr>
</table>

## 3. Versione non bloccante con DiffTimer

`start()` non fa nulla se il timer è già attivo, quindi si può chiamare a ogni giro. A timer
fermo e azzerato `get()` restituisce 0: non serve controllare `timerState`.

<p align="center">
  <img src="img/debounce-difftimer-pulldown.gif" alt="Animazione del debounce con DiffTimer (pull-down)" width="900">
</p>

Con il pull-up il comportamento è identico con i livelli invertiti: il pulsante premuto legge LOW, la pressione è un fronte di discesa e il rilascio un fronte di salita.

L'animazione mostra la versione completa della dispensa principale, con `doOnRise()` e il controllo su `timerState` alla pressione; qui quella parte manca, ma rimbalzi, disturbo e conferma del rilascio si comportano allo stesso modo.

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
    debt.start();  // no-op se attivo
    debt.reset();
  } else if (debt.get() > 50) {
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
    debt.start();  // no-op se attivo
    debt.reset();
  } else if (debt.get() > 50) {
    debt.stop();
    debt.reset();
    doOnRise();
  }
}
```

</td>
</tr>
</table>

## 4. Versione non bloccante con waitUntilNB()

Da sola `waitUntilNB()` conta dalla prima lettura "premuto": al rilascio scatterebbe subito,
senza debounce, e un rimbalzo del contatto la farebbe scattare una seconda volta. Per questo,
a ogni lettura "premuto", si chiama `debt.reset()`: i 50 ms si contano dall'ultima lettura
"premuto", come nella versione con DiffTimer.

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
  bool premuto = digitalRead(P1);
  if (waitUntilNB(debt,
        !premuto, 50)) {
    doOnFall();
  }
  if (premuto) debt.reset();
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
  bool premuto = !digitalRead(P1);
  if (waitUntilNB(debt,
        !premuto, 50)) {
    doOnRise();
  }
  if (premuto) debt.reset();
}
```

</td>
</tr>
</table>

## 5. Versione con schedulatore non bloccante

Il rilascio viene riconosciuto con un ritardo fino a `TBASE` ms.

<p align="center">
  <img src="img/debounce-schedulatore-pulldown.gif" alt="Animazione del debounce con schedulatore (pull-down)" width="900">
</p>

Con il pull-up il comportamento è identico con i livelli invertiti: il pulsante premuto legge LOW, la pressione è un fronte di discesa e il rilascio un fronte di salita.

L'animazione mostra la versione completa della dispensa principale, che riconosce anche il fronte di salita con `doOnRise()`; qui si controlla solo quello di discesa, il campionamento è lo stesso.

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
#define TBASE 50
unsigned long precm = 0;
byte precval = LOW, val;

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
    if (precval == HIGH &&
        val == LOW) {
      doOnFall();
    }
    precval = val;
  }
}
```

</td>
<td>

```cpp
const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
#define TBASE 50
unsigned long precm = 0;
byte precval = HIGH, val;

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
    if (precval == LOW &&
        val == HIGH) {
      doOnRise();
    }
    precval = val;
  }
}
```

</td>
</tr>
</table>

## Considerazioni finali: quale tecnica scegliere

**Il limite di fondo, comune a tutte le tecniche.** Ogni antirimbalzo stabilisce un tempo minimo, ad esempio 50 msec, oltre il quale un'apertura del contatto viene considerata un rilascio vero. Un disturbo che apre il contatto per più di quel tempo non si distingue da un rilascio breve: vale per il polling, per lo schedulatore e per gli interrupt. Nel seguito si dà per scontato che questo limite sia superato scegliendo un tempo adatto alla qualità del pulsante: abbastanza lungo da coprire i suoi rimbalzi e i suoi disturbi, abbastanza corto da non perdere le pressioni rapide.

**Il polling veloce con il timer che riparte è la tecnica più robusta e più pronta.** È la versione in cui il ```loop()``` legge il pulsante a ogni giro e il timer riparte a ogni lettura "premuto" (```t0 = millis()``` fuori dall'```if```):
- guarda il pulsante in continuazione: un disturbo breve a pulsante premuto viene visto di sicuro, ma fa solo ripartire il conteggio;
- il rilascio è accettato solo se il pin è rimasto basso per **tutto** il tempo di antirimbalzo: la decisione non dipende da dove cade il disturbo;
- la pressione è riconosciuta alla prima lettura alta, il rilascio dopo il tempo di antirimbalzo.

La condizione è che il ```loop()``` resti veloce: con ```delay()``` o funzioni lente le letture si diradano e il polling si comporta come uno schedulatore.

**Lo schedulatore (time tick)** legge il pulsante solo ogni ```TBASE```. È immune ai rimbalzi perché tra una lettura e l'altra non li vede, e per lo stesso motivo di solito non vede nemmeno i disturbi brevi a pulsante premuto; se però una lettura cade proprio dentro un disturbo, il toggle scatta due volte (la probabilità è circa la durata del disturbo divisa per ```TBASE```). Si può ridurre il rischio accettando un nuovo livello solo dopo N letture uguali consecutive: il tempo minimo di un rilascio vero diventa N × ```TBASE```. È un'approssimazione, con il campionamento rado, di ciò che il polling veloce fa con il campionamento continuo.

**Gli interrupt su entrambi i fronti** fanno la stessa cosa del polling veloce: vedono ogni fronte, rimbalzi e disturbi compresi, e fanno ripartire il conteggio del tempo di antirimbalzo. In più funzionano anche quando il ```loop()``` è lento: sono la scelta giusta quando il ciclo principale ha altro di pesante da fare.

## Riferimenti

- [urutils.h](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/urutils.h): sorgente della libreria
- [pulsanti-rilascio.md](pulsanti-rilascio.md): versioni complete, con azione alla pressione e al rilascio
- [toggle-blink-pulldown.md](toggle-blink-pulldown.md): toggle e blink come due task concorrenti
