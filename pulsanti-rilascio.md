# Pulsanti: attivazione sul rilascio (pull-down e pull-up)

In entrambe le versioni l'azione parte quando il pulsante viene **rilasciato**.
Cambia solo il livello letto dal pin, che dipende dal collegamento della resistenza.

| Collegamento | Riposo | Premuto | Pressione | Rilascio |
| --- | --- | --- | --- | --- |
| Pull-down (resistenza verso GND) | LOW | HIGH | fronte di salita | fronte di discesa |
| Pull-up (resistenza verso Vcc o `INPUT_PULLUP`) | HIGH | LOW | fronte di discesa | fronte di salita |

## 1. Versione bloccante con waitUntilInputLow / waitUntilInputHigh

Le funzioni di attesa sono in `urutils.h`: non ridefinitele se includete la libreria.
Finché il pulsante resta premuto, `loop()` è fermo.

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

void doOnRise() {
  // pressione
}

void doOnFall() {
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
#include "urutils.h"

const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;

void doOnFall() {
  // pressione
}

void doOnRise() {
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
const byte P1 = 2;
unsigned long t0 = 0;
bool armato = false;

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
const byte P1 = 2;
unsigned long t0 = 0;
bool armato = false;

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

const byte P1 = 2;
DiffTimer debt;

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

const byte P1 = 2;
DiffTimer debt;

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

Il secondo argomento è la condizione "pulsante rilasciato". Il conteggio parte dalla
prima pressione: un rimbalzo lungo al rilascio può far scattare l'azione due volte.

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```cpp
#include "urutils.h"

const byte P1 = 2;
DiffTimer debt;

void setup() {
  pinMode(P1, INPUT);
}

void loop() {
  // rilasciato = LOW
  if (waitUntilNB(debt,
        !digitalRead(P1), 50)) {
    doOnFall();
  }
}
```

</td>
<td>

```cpp
#include "urutils.h"

const byte P1 = 2;
DiffTimer debt;

void setup() {
  pinMode(P1, INPUT_PULLUP);
}

void loop() {
  // rilasciato = HIGH
  if (waitUntilNB(debt,
        digitalRead(P1), 50)) {
    doOnRise();
  }
}
```

</td>
</tr>
</table>

## Riferimenti

- [urutils.h](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/urutils.h): sorgente della libreria
- [toggle.md](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/toggle.md): esempi di pulsante toggle
