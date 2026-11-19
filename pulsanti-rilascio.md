>[Torna all'indice](indexpulsanti.md)   >[versione in Python](pulsanti-rilascio-mpy.md)

# Pulsanti: attivazione sul rilascio (pull-down e pull-up)

In entrambe le versioni l'azione principale parte quando il pulsante viene **rilasciato**.
Cambia solo il livello letto dal pin, che dipende dal collegamento della resistenza.

| Collegamento | Riposo | Premuto | Pressione | Rilascio |
| --- | --- | --- | --- | --- |
| Pull-down (resistenza verso GND) | LOW | HIGH | fronte di salita | fronte di discesa |
| Pull-up (resistenza verso Vcc o `INPUT_PULLUP`) | HIGH | LOW | fronte di discesa | fronte di salita |

In ogni esempio ci sono due callback: in quella della **pressione** vanno le azioni immediate
alla pressione (facoltative, qui lasciata vuota); in quella del **rilascio** vanno le azioni
al rilascio (qui, come esempio, un toggle del LED).

> Versioni corte, con la sola azione al rilascio: [pulsanti-rilascio-corte.md](pulsanti-rilascio-corte.md)
> Esempi con due task concorrenti, toggle e blink (pull-down): [toggle-blink-pulldown.md](toggle-blink-pulldown.md)

## 1. Versione bloccante con waitUntilInputLow / waitUntilInputHigh

Le funzioni di attesa sono definite nello sketch, uguali a quelle di `urutils.h`:
se includete la libreria, toglietele per non definirle due volte.
Sono le versioni aggiornate di `urutils.h`: dopo il primo LOW attendono altri `t` ms e ricontrollano
il pin, quindi escono solo con un rilascio confermato. Così né un rimbalzo del rilascio né un'interruzione
breve del contatto a pulsante premuto vengono presi per un rilascio.
Finché il pulsante resta premuto, `loop()` è fermo.

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
    doOnRise();
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
    doOnFall();
    waitUntilInputHigh(P1, 50);
    doOnRise();
  }
}
```

</td>
</tr>
</table>

## 2. Versione non bloccante con millis()

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
bool armato = false;

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
    if (!armato) {
      armato = true;
      doOnRise();
    }
    t0 = millis();
  } else if (armato &&
             millis() - t0 > 50) {
    armato = false;
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
bool armato = false;

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
    if (!armato) {
      armato = true;
      doOnFall();
    }
    t0 = millis();
  } else if (armato &&
             millis() - t0 > 50) {
    armato = false;
    doOnRise();
  }
}
```

</td>
</tr>
</table>

`t0` si aggiorna a ogni lettura "premuto": un rimbalzo al rilascio azzera il conteggio
e non produce un falso fronte.

## 3. Versione non bloccante con DiffTimer

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

## 4. Versione non bloccante con waitUntilNB()

Il secondo argomento è la condizione "pulsante rilasciato", cioè `!premuto`.
Da sola `waitUntilNB()` conta dalla prima lettura "premuto": al rilascio scatterebbe subito,
senza debounce, e un rimbalzo del contatto la farebbe scattare una seconda volta. Per questo,
a ogni lettura "premuto", si chiama `debt.reset()`: i 50 ms si contano dall'ultima lettura
"premuto", come nella versione con DiffTimer.
`waitUntilNB()` segnala solo il rilascio: la callback della pressione si chiama controllando
`debt.timerState` prima della chiamata. Il timer è disarmato solo al primo giro di pressione,
perché subito dopo lo arma `waitUntilNB()`.

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
  bool premuto = digitalRead(P1);
  if (premuto &&
      !debt.timerState) {
    doOnRise();
  }
  // c = "rilasciato"
  if (waitUntilNB(debt,
        !premuto, 50)) {
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
  bool premuto = !digitalRead(P1);
  if (premuto &&
      !debt.timerState) {
    doOnFall();
  }
  // c = "rilasciato"
  if (waitUntilNB(debt,
        !premuto, 50)) {
    doOnRise();
  }
}
```

</td>
</tr>
</table>

## 5. Versione con schedulatore non bloccante

Il codice di gestione del pulsante gira una volta ogni `TBASE` ms. Leggere il pin a
intervalli regolari fa già da debounce: se `TBASE` è più lungo dei rimbalzi, tra due
letture il segnale si è stabilizzato. I fronti si riconoscono confrontando la lettura
attuale `val` con quella precedente `precval`.

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

In tutte le altre versioni la callback della pressione scatta subito, al primo giro di
`loop()` in cui il pin risulta premuto. Con lo schedulatore invece il pin viene letto solo
ogni `TBASE` ms: la pressione viene riconosciuta con un ritardo fino a `TBASE`, e una
pressione più breve di `TBASE` può cadere tra due campioni e non essere vista.

## Riferimenti

- [urutils.h](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/urutils.h): sorgente della libreria
- [toggle.md](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/toggle.md): esempi di pulsante toggle
- [pulsanti-rilascio-corte.md](pulsanti-rilascio-corte.md): versioni corte, solo azione al rilascio
- [toggle-blink-pulldown.md](toggle-blink-pulldown.md): toggle e blink come due task concorrenti
