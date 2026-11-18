# Pulsante toggle e blink: due task concorrenti (pull-down)

Ogni sketch gestisce due task insieme:

1. **Pulsante toggle:** a ogni rilascio il LED `LED` cambia stato.
2. **Blink:** il LED `LEDBLINK` lampeggia con periodo di 2 s (1000 ms acceso, 1000 ms spento).

Il pulsante è gestito a eventi: la callback `doOnRise()` scatta alla pressione,
`doOnFall()` al rilascio. Il blink è un evento periodico. Tutti gli esempi sono per
il collegamento **pull-down** (pulsante premuto = HIGH).

> **Per il pull-up** basta usare `pinMode(P1, INPUT_PULLUP)` e mettere un `!` davanti a
> ogni `digitalRead(P1)`; dove c'è già un `!`, si toglie. Così "premuto" torna a valere 1
> e il resto non cambia: `doOnRise()` resta la pressione e `doOnFall()` il rilascio.
> Unica eccezione la versione bloccante: `waitUntilInputLow()` legge il pin direttamente,
> quindi nel pull-up va sostituita con `waitUntilInputHigh()`.

> Versioni con un solo pulsante, pull-down e pull-up affiancati: [pulsanti-rilascio.md](pulsanti-rilascio.md) (complete) e [pulsanti-rilascio-corte.md](pulsanti-rilascio-corte.md) (solo azione al rilascio)

## 1. Versione bloccante (waitUntilInputLow)

Mentre il pulsante è premuto `loop()` resta dentro `waitUntilInputLow()`, quindi il blink
si ferma. Questo esempio mostra il limite del codice bloccante con più task.

```cpp
const byte P1       = 2;    // pulsante (pull-down esterno)
const byte LED      = 13;   // task 1: LED comandato dal pulsante (toggle)
const byte LEDBLINK = 12;   // task 2: LED lampeggiante
bool stato = LOW;
unsigned long precBlink = 0;

void waitUntilInputLow(int btn, unsigned t) {
  // come in urutils.h: attende t ms,
  // poi ripete finché il pin non è LOW
  do {
    delay(t);
  } while (digitalRead(btn) != LOW);
}

void doOnRise() {
  // PRESSIONE (fronte di salita): qui vanno le azioni immediate alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa): qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void blink(byte pin) {
  digitalWrite(pin, !digitalRead(pin));   // inverte il LED lampeggiante
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(LEDBLINK, OUTPUT);
}

void loop() {
  // task 1: pulsante (bloccante)
  if (digitalRead(P1) == HIGH) {
    doOnRise();
    waitUntilInputLow(P1, 50);    // loop() fermo finché il pulsante è premuto
    doOnFall();
  }

  // task 2: blink ogni 1000 ms
  if (millis() - precBlink >= 1000) {
    precBlink = millis();
    blink(LEDBLINK);
  }
}
```

## 2. Versione non bloccante con millis()

```cpp
const byte P1       = 2;    // pulsante (pull-down esterno)
const byte LED      = 13;   // task 1: LED comandato dal pulsante (toggle)
const byte LEDBLINK = 12;   // task 2: LED lampeggiante
bool stato = LOW;
unsigned long t0 = 0;          // ultima lettura "premuto"
bool armato = false;
unsigned long precBlink = 0;

void doOnRise() {
  // PRESSIONE (fronte di salita): qui vanno le azioni immediate alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa): qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void blink(byte pin) {
  digitalWrite(pin, !digitalRead(pin));   // inverte il LED lampeggiante
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(LEDBLINK, OUTPUT);
}

void loop() {
  // task 1: pulsante
  if (digitalRead(P1) == HIGH) {
    if (!armato) {
      armato = true;
      doOnRise();
    }
    t0 = millis();
  } else if (armato && millis() - t0 > 50) {
    armato = false;
    doOnFall();
  }

  // task 2: blink ogni 1000 ms
  if (millis() - precBlink >= 1000) {
    precBlink = millis();
    blink(LEDBLINK);
  }
}
```

## 3. Versione non bloccante con DiffTimer

```cpp
#include "urutils.h"

const byte P1       = 2;    // pulsante (pull-down esterno)
const byte LED      = 13;   // task 1: LED comandato dal pulsante (toggle)
const byte LEDBLINK = 12;   // task 2: LED lampeggiante
bool stato = LOW;
DiffTimer debt, t1;            // debounce e blink

void doOnRise() {
  // PRESSIONE (fronte di salita): qui vanno le azioni immediate alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa): qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void blink(byte pin) {
  digitalWrite(pin, !digitalRead(pin));   // inverte il LED lampeggiante
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(LEDBLINK, OUTPUT);
  t1.start();
}

void loop() {
  // task 1: pulsante
  if (digitalRead(P1) == HIGH) {
    if (!debt.timerState) {
      debt.start();
      doOnRise();
    }
    debt.reset();
  } else if (debt.timerState && debt.get() > 50) {
    debt.stop();
    debt.reset();
    doOnFall();
  }

  // task 2: blink ogni 1000 ms
  if (t1.get() > 1000) {
    t1.reset();
    blink(LEDBLINK);
  }
}
```

## 4. Versione non bloccante con waitUntilNB()

Qui la callback della pressione viene chiamata controllando `debt.timerState` prima di
`waitUntilNB()`, che da sola segnala solo il rilascio.

```cpp
#include "urutils.h"

const byte P1       = 2;    // pulsante (pull-down esterno)
const byte LED      = 13;   // task 1: LED comandato dal pulsante (toggle)
const byte LEDBLINK = 12;   // task 2: LED lampeggiante
bool stato = LOW;
DiffTimer debt, t1;

void doOnRise() {
  // PRESSIONE (fronte di salita): qui vanno le azioni immediate alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa): qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void blink(byte pin) {
  digitalWrite(pin, !digitalRead(pin));   // inverte il LED lampeggiante
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(LEDBLINK, OUTPUT);
  t1.start();
}

void loop() {
  // task 1: pulsante
  bool premuto = digitalRead(P1);
  if (premuto && !debt.timerState) {
    doOnRise();                         // azione immediata alla pressione
  }
  if (waitUntilNB(debt, !premuto, 50)) {
    doOnFall();                         // azione al rilascio
  }

  // task 2: blink ogni 1000 ms
  if (t1.get() > 1000) {
    t1.reset();
    blink(LEDBLINK);
  }
}
```

## 5. Versione con schedulatore non bloccante

Un solo tempo base `TBASE` scandisce entrambi i task: il pulsante viene campionato a ogni
tick, il blink scatta ogni `NBLINK` tick. Il contatore `step` è circolare: va da 0 a `NSTEP - 1`
e poi riparte, quindi non cresce mai e non va in overflow.

```cpp
#define TBASE 50              // periodo base in ms (> durata dei rimbalzi)
#define NBLINK (1000 / TBASE) // periodo del blink in tick (semiperiodo 1000 ms)
#define NSTEP NBLINK           // lunghezza del ciclo: mcm dei periodi dei task

const byte P1       = 2;    // pulsante (pull-down esterno)
const byte LED      = 13;   // task 1: LED comandato dal pulsante (toggle)
const byte LEDBLINK = 12;   // task 2: LED lampeggiante
bool stato = LOW;
unsigned long precm = 0;
byte precval = LOW, val;
byte step = 0;                // contatore circolare dei tick: 0 .. NSTEP-1

void doOnRise() {
  // PRESSIONE (fronte di salita): qui vanno le azioni immediate alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa): qui vanno le azioni al rilascio.
  // Esempio: toggle, inverte lo stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void blink(byte pin) {
  digitalWrite(pin, !digitalRead(pin));   // inverte il LED lampeggiante
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(LEDBLINK, OUTPUT);
}

void loop() {
  if (millis() - precm >= TBASE) {    // un tick ogni TBASE ms
    precm = millis();
    step = (step + 1) % NSTEP;      // conteggio circolare, al massimo NSTEP-1

    // task 1: pulsante, campionato a ogni tick
    val = digitalRead(P1);
    if (precval == LOW && val == HIGH) {
      doOnRise();
    } else if (precval == HIGH && val == LOW) {
      doOnFall();
    }
    precval = val;

    // task 2: blink ogni NBLINK tick (1000 ms)
    if (step % NBLINK == 0) {
      blink(LEDBLINK);
    }
  }
}
```

**Più task periodici.** Se lo schedulatore deve gestire più task periodici, `NSTEP` va
scelto come minimo comune multiplo dei loro periodi in tick. Ogni task poi controlla
`step % N_task == 0`, dove `N_task` è il suo periodo in tick. Così il ciclo si chiude
senza sfasare nessun task. Per esempio, con un blink ogni 20 tick e un secondo task
ogni 30 tick:

```cpp
#define NBLINK 20                // 1000 ms con TBASE = 50
#define NTASK2 30                // 1500 ms con TBASE = 50
#define NSTEP  60                // mcm(20, 30)

    step = (step + 1) % NSTEP;
    if (step % NBLINK == 0) {    // a step 0, 20, 40
      blink(LEDBLINK);
    }
    if (step % NTASK2 == 0) {    // a step 0, 30
      task2();
    }
```

Con un `NSTEP` che non fosse multiplo di 30, per esempio 40, il secondo task scatterebbe a
step 0 e 30 e poi di nuovo a 0, cioè dopo soli 10 tick: il ciclo si chiuderebbe sfasando il task.

## Riferimenti

- [urutils.h](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/urutils.h): sorgente della libreria
- [toggle.md](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/toggle.md): esempi di pulsante toggle
- [pulsanti-rilascio.md](pulsanti-rilascio.md): versioni complete, pull-down e pull-up
- [pulsanti-rilascio-corte.md](pulsanti-rilascio-corte.md): versioni corte, solo azione al rilascio
