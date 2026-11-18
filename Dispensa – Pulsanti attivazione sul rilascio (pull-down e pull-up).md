# Dispensa – Pulsanti: attivazione sul rilascio (pull-down e pull-up)

Nov 18, 2026 · @Emilio

## Concetti di base

In entrambe le versioni l'azione parte quando il pulsante viene **rilasciato**. Cambia solo il livello logico che il pin legge, perché dipende dal collegamento della resistenza.

| Collegamento | Pulsante a riposo | Pulsante premuto | Pressione | Rilascio |
| --- | --- | --- | --- | --- |
| Pull-down (resistenza verso GND) | LOW | HIGH | fronte di salita | fronte di discesa |
| Pull-up (resistenza verso Vcc, o `INPUT_PULLUP`) | HIGH | LOW | fronte di discesa | fronte di salita |

Un **fronte** è il passaggio da un livello all'altro: di salita da LOW a HIGH, di discesa da HIGH a LOW.

I contatti meccanici **rimbalzano**: per pochi millisecondi dopo la pressione o il rilascio il segnale oscilla. Senza un filtro (debounce), un solo rilascio verrebbe letto come più fronti. Nei due programmi il filtro è un'attesa di 50 ms.

## Versione pull-down: azione sul fronte di discesa

Con la resistenza di pull-down il pin legge HIGH a pulsante premuto. Il programma riconosce la pressione e aspetta che il pin torni LOW. Solo allora esegue l'azione.

```cpp
// ATTIVAZIONE SUL FRONTE DI DISCESA (PULL-DOWN)
const int P1  = 2;    // pulsante
const int LED = 13;   // uscita da comandare
int stato = LOW;

void waitUntilInputLow(int pin, unsigned t) {
  while (digitalRead(pin) == HIGH) {
    delay(t);         // ricontrolla ogni t ms
  }
}

void updateOutputs(int s) {
  digitalWrite(LED, s);
}

void doOnRise() {
  // pressione: qui eventuali azioni immediate
}

void doOnFall() {
  stato = !stato;     // nuovoStato = inverso dello stato attuale
  updateOutputs(stato);
}

void setup() {
  pinMode(P1, INPUT);       // resistenza di pull-down esterna
  pinMode(LED, OUTPUT);
}

void loop() {
  if (digitalRead(P1) == HIGH) {   // pulsante premuto
    doOnRise();
    waitUntilInputLow(P1, 50);     // attesa del rilascio
    doOnFall();                    // azione sul rilascio
  }
}
```

1. `digitalRead(P1) == HIGH` riconosce la pressione.
2. `doOnRise()` esegue le eventuali azioni alla pressione.
3. `waitUntilInputLow()` blocca il programma finché il pulsante resta premuto.
4. Al rilascio `doOnFall()` cambia lo stato e aggiorna l'uscita.

## Versione pull-up: azione sul fronte di salita

Con la resistenza di pull-up il pin legge LOW a pulsante premuto. La logica è la stessa della versione pull-down, con i livelli invertiti. Si può usare la resistenza interna con `INPUT_PULLUP`, senza componenti esterni.

```cpp
// ATTIVAZIONE SUL FRONTE DI SALITA (PULL-UP)
const int P1  = 2;    // pulsante tra pin e GND
const int LED = 13;
int stato = LOW;

void waitUntilInputHigh(int pin, unsigned t) {
  while (digitalRead(pin) == LOW) {
    delay(t);
  }
}

void updateOutputs(int s) {
  digitalWrite(LED, s);
}

void doOnFall() {
  // pressione: qui eventuali azioni immediate
}

void doOnRise() {
  stato = !stato;
  updateOutputs(stato);
}

void setup() {
  pinMode(P1, INPUT_PULLUP);  // pull-up interno
  pinMode(LED, OUTPUT);
}

void loop() {
  if (digitalRead(P1) == LOW) {    // pulsante premuto
    doOnFall();
    waitUntilInputHigh(P1, 50);    // attesa del rilascio
    doOnRise();                    // azione sul rilascio
  }
}
```

Rispetto alla versione pull-down cambiano solo tre cose: il livello testato nell'`if`, la funzione di attesa e il ruolo di `doOnFall()` e `doOnRise()`.

## Le funzioni di attesa e i loro limiti

`waitUntilInputLow()` e `waitUntilInputHigh()` ricontrollano il pin ogni 50 ms. Restano nel ciclo `while` finché il pin non cambia livello. La pausa tra due letture fa da debounce: i rimbalzi durano meno di 50 ms e quasi sempre vengono saltati.

Le due funzioni sono già in [urutils.h](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/urutils.h), con la stessa firma `(int btn, unsigned t)`. La versione della libreria usa un ciclo `do { delay(t); } while (…)`: aspetta sempre almeno `t` ms prima di leggere il pin. Se includete `urutils.h`, togliete dagli esempi le definizioni di `waitUntilInputLow()` e `waitUntilInputHigh()`: definirle due volte dà errore di compilazione.

Il limite è che il codice è **bloccante**. Finché il pulsante resta premuto, `loop()` è fermo:

- non si leggono altri ingressi;
- non si aggiornano altre uscite, per esempio un LED lampeggiante;
- non si gestisce la seriale.

Va bene per esercizi con un solo pulsante. Con più compiti in parallelo serve la versione con `millis()` dell'ultima sezione.

## Confronto ed errori comuni

|  | Pull-down | Pull-up |
| --- | --- | --- |
| `pinMode` | `INPUT` + resistenza esterna verso GND | `INPUT_PULLUP` (o resistenza esterna verso Vcc) |
| Test di pressione | `digitalRead(P1) == HIGH` | `digitalRead(P1) == LOW` |
| Attesa del rilascio | `waitUntilInputLow()` | `waitUntilInputHigh()` |
| Funzione alla pressione | `doOnRise()` | `doOnFall()` |
| Funzione al rilascio | `doOnFall()` | `doOnRise()` |

Errori da evitare, già corretti nel codice sopra:

- **Tipo di ritorno mancante.** `doOnFall(){ … }` non compila: serve `void doOnFall()`.
- **Funzione non definita.** Se `loop()` chiama sia `doOnRise()` sia `doOnFall()`, vanno definite entrambe, anche vuote.
- **Stato mai calcolato.** `stato = nuovoStato` non cambia nulla se `nuovoStato` non viene mai aggiornato. Per un interruttore a levetta basta `stato = !stato`.
- **Pin flottante.** Senza resistenza di pull-down o pull-up l'ingresso legge valori casuali.
- **Livelli scambiati.** Con `INPUT_PULLUP` il pulsante premuto legge LOW, non HIGH.

## Alternativa non bloccante con millis()

Con `millis()` il programma non si ferma mai. Registra l'istante della pressione e controlla a ogni giro di `loop()` se sono passati 50 ms di pulsante rilasciato.

**Pull-down: azione sul fronte di discesa**

```cpp
const int P1 = 2;
unsigned long t0 = 0;   // istante dell'ultima lettura "premuto"
bool armato = false;    // true = pressione rilevata, rilascio in attesa

void loop() {
  if (digitalRead(P1) == HIGH) {   // premuto
    if (!armato) {
      armato = true;
      doOnRise();
    }
    t0 = millis();                 // finché è premuto il timer riparte
  } else if (armato && millis() - t0 > 50) {
    armato = false;                // rilasciato da più di 50 ms
    doOnFall();
  }
  // qui altri compiti, mai bloccati
}
```

**Pull-up: azione sul fronte di salita**

```cpp
const int P1 = 2;
unsigned long t0 = 0;
bool armato = false;

void loop() {
  if (digitalRead(P1) == LOW) {    // premuto
    if (!armato) {
      armato = true;
      doOnFall();
    }
    t0 = millis();
  } else if (armato && millis() - t0 > 50) {
    armato = false;
    doOnRise();
  }
}
```

`t0` si aggiorna a ogni lettura "premuto", quindi conta il tempo dall'ultimo istante in cui il pulsante era premuto. Un rimbalzo durante il rilascio azzera il conteggio e non produce un falso fronte. Scrivete sempre `millis() - t0 > 50` con variabili `unsigned long`: così il calcolo resta corretto anche quando `millis()` riparte da zero, dopo circa 49 giorni.

## Versione con urutils.h (DiffTimer)

`DiffTimer` di [urutils.h](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/urutils.h) incapsula il calcolo con `millis()` visto sopra. La logica non cambia: il timer si arma alla pressione e l'azione parte dopo 50 ms di pulsante rilasciato. Gli esempi seguono lo stile di [toggle.md](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/toggle.md).

| Membro | Effetto |
| --- | --- |
| `start()` | se il timer è disarmato lo arma e memorizza l'istante di partenza; se è già armato non fa nulla |
| `get()` | timer armato: tempo accumulato + ms dall'ultimo `start()` o `reset()`; timer disarmato: solo il tempo accumulato |
| `reset()` | azzera il tempo accumulato e riparte da adesso; non cambia `timerState` |
| `stop()` | disarma il timer e aggiunge al tempo accumulato i ms trascorsi |
| `timerState` | `true` se il timer è armato |

**Pull-down: azione sul fronte di discesa**

```cpp
#include "urutils.h"

const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
DiffTimer debt;          // timer di debounce

void doOnRise() {
  // pressione: eventuali azioni immediate
}

void doOnFall() {
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT);    // resistenza di pull-down esterna
  pinMode(LED, OUTPUT);
}

void loop() {
  if (digitalRead(P1) == HIGH) {          // premuto
    if (!debt.timerState) {               // primo fronte: armo il timer
      debt.start();
      doOnRise();
    }
    debt.reset();                         // finché è premuto il conteggio riparte
  } else if (debt.timerState && debt.get() > 50) {
    debt.stop();                          // disarmo del timer al timeout
    debt.reset();
    doOnFall();                           // azione sul rilascio
  }
  // qui altri compiti, mai bloccati
}
```

**Pull-up: azione sul fronte di salita**

```cpp
#include "urutils.h"

const byte P1  = 2;
const byte LED = 13;
bool stato = LOW;
DiffTimer debt;

void doOnFall() {
  // pressione: eventuali azioni immediate
}

void doOnRise() {
  stato = !stato;
  digitalWrite(LED, stato);
}

void setup() {
  pinMode(P1, INPUT_PULLUP);  // pull-up interno
  pinMode(LED, OUTPUT);
}

void loop() {
  if (digitalRead(P1) == LOW) {           // premuto
    if (!debt.timerState) {
      debt.start();
      doOnFall();
    }
    debt.reset();
  } else if (debt.timerState && debt.get() > 50) {
    debt.stop();
    debt.reset();
    doOnRise();
  }
}
```

**Toggle con LED lampeggiante in parallelo (pull-down)**

Sullo schema della sezione “Toggle con polling del rilascio temporizzato con get()” di toggle.md: un secondo `DiffTimer` fa lampeggiare un LED mentre il pulsante viene gestito, a dimostrazione che il codice non è bloccante.

```cpp
#include "urutils.h"

const byte led      = 13;   // lampeggia ogni secondo
const byte led2     = 27;   // comandato dal pulsante
const byte pulsante = 12;
bool stato = LOW;
DiffTimer t1, debt;

void blink(byte pin) {
  digitalWrite(pin, !digitalRead(pin));
}

void setup() {
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(pulsante, INPUT);
  t1.start();                  // timer del lampeggio
}

void loop() {
  if (digitalRead(pulsante) == HIGH) {
    if (!debt.timerState) debt.start();
    debt.reset();
  } else if (debt.timerState && debt.get() > 50) {
    debt.stop();
    debt.reset();
    stato = !stato;            // azione sul rilascio
    digitalWrite(led2, stato);
  }

  if (t1.get() > 1000) {       // compito parallelo
    t1.reset();
    blink(led);
  }
}
```

I pin 27 e 12 sono quelli di toggle.md, pensati per ESP32; su Arduino Uno usate pin che esistono sulla scheda.

`urutils.h` offre anche `waitUntilNB(d, c, t)`, che fa lo stesso controllo in una sola chiamata: arma il timer quando `c` è falsa e restituisce `true` una volta sola, quando `c` è vera e sono passati più di `t` ms. Per il pull-down si scrive `if (waitUntilNB(debt, !digitalRead(P1), 50)) doOnFall();`. A differenza degli esempi sopra, conta i 50 ms dalla prima pressione e non dall'ultima lettura di pulsante premuto: un rimbalzo lungo al rilascio può riarmare il timer e far scattare l'azione una seconda volta. Per questo negli esempi sopra `debt.reset()` viene chiamato a ogni lettura di pulsante premuto.

## Riferimenti

- [urutils.h](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/urutils.h): sorgente della libreria con `DiffTimer`, `waitUntilInputLow()`, `waitUntilInputHigh()` e `waitUntilNB()`
- [toggle.md](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/toggle.md): esempi di pulsante toggle da cui riprendono lo stile gli esempi con `DiffTimer`
