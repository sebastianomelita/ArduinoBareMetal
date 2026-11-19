# Correzioni alle tre versioni con polling di toggle.md

Ogni sezione sostituisce quella con lo stesso titolo in `toggle.md`. La correzione è sempre la
stessa: **mentre il pulsante è premuto il timer riparte a ogni lettura**, così i 50 ms si contano
dall'ultima lettura "premuto" e non dalla pressione. Prima il timer era già scaduto al momento
del rilascio, e un rimbalzo del rilascio veniva preso per una nuova pressione.

Le righe aggiunte o cambiate sono segnate con `// <<<`.

---

## **Attivazione immediata di una logica qualsiasi su un fronte**

```C++
// APPROCCIO CON TIMER - FRONTE SALITA                     | // APPROCCIO CON TIMER - FRONTE DISCESA
                                                           |
                                                           |
int stato, nuovoStato;                                     | int stato, nuovoStato;
int P1 = 2;                                                | int P1 = 2;
DiffTimer debt;                                   // <<<   | DiffTimer debt;                                   // <<<
                                                           |
void loop() {                                              | void loop() {
  if (digitalRead(P1)) {                                   |   if (!digitalRead(P1)) {
    if(!debt.timerState){                                  |     if(!debt.timerState){
      debt.start();                                        |       debt.start();
      doOnRise();                                          |       doOnFall();
    }                                                      |     }
    debt.reset(); // ancora premuto: riparte      // <<<   |     debt.reset(); // ancora premuto: riparte      // <<<
  } else if(debt.get() > 50) {                             |   } else if(debt.get() > 50) { // disarmo del timer al timeout
    debt.stop(); // disarmo del timer                      |     debt.stop(); // disarmo del timer
    debt.reset();                                          |     debt.reset();
    doOnFall();                                            |     doOnRise();
    // Altre operazioni possibili                          |     // Altre operazioni possibili
  }                                                        |   }
}                                                          | }
```
- la rilevazione del **primo fronte** è immediata, dato che non deve attendere mai il periodo stabilito da un timer.
- la rilevazione del **secondo fronte** (opposto al precedente) avviene dopo **50 ms di livello stabile**: il timer riparte a ogni lettura del primo livello, quindi anche a ogni rimbalzo, e scade solo quando i rimbalzi del rilascio sono finiti.
- **```debt.reset()```** nel primo ramo: è la correzione. Senza, il timer misurava il tempo dalla pressione e al rilascio era già scaduto: la prima lettura in mezzo ai rimbalzi faceva scattare il secondo fronte, e il rimbalzo successivo veniva preso per un nuovo primo fronte.
- **```debt.get()```**: rappresenta il tempo trascorso dall'ultima lettura del primo livello.
- nel codice originale il timer era dichiarato ```deb``` ma usato come ```debt```: ora il nome è lo stesso.

## Debounce con millis()

| Fronte di SALITA | Fronte di DISCESA |
|---|---|
| Esegue `doOnRise()` subito sul fronte, `doOnFall()` dopo 50 ms di stato basso stabile | Esegue `doOnFall()` subito sul fronte, `doOnRise()` dopo 50 ms di stato alto stabile |

### Approccio con millis() – fronte di salita

```cpp
int P1 = 2;
unsigned long t0 = 0;   // ultima lettura "premuto"
bool attivo = false;    // equivalente di deb.timerState

void loop() {
  if (digitalRead(P1)) {
    if (!attivo) {             // timer non attivo
      attivo = true;           // deb.start()
      doOnRise();
    }
    t0 = millis();             // ancora premuto: il conteggio riparte   // <<<
  } else if (attivo && millis() - t0 > 50) { // deb.get() > 50
    attivo = false;            // deb.stop() + deb.reset()
    doOnFall();
    // Altre operazioni possibili
  }
}
```

### Approccio con millis() – fronte di discesa

```cpp
int P1 = 2;
unsigned long t0 = 0;   // ultima lettura "premuto"
bool attivo = false;    // equivalente di deb.timerState

void loop() {
  if (!digitalRead(P1)) {
    if (!attivo) {             // timer non attivo
      attivo = true;           // deb.start()
      doOnFall();
    }
    t0 = millis();             // ancora premuto: il conteggio riparte   // <<<
  } else if (attivo && millis() - t0 > 50) { // deb.get() > 50
    attivo = false;            // deb.stop() + deb.reset()
    doOnRise();
    // Altre operazioni possibili
  }
}
```

### Corrispondenze DiffTimer → millis()

| DiffTimer | millis() |
|---|---|
| `deb.start()` | `attivo = true;` |
| `deb.reset()` a ogni lettura "premuto" | `t0 = millis();` a ogni lettura "premuto" |
| `deb.timerState` | `attivo` |
| `deb.get()` | `millis() - t0` |
| `deb.stop(); deb.reset();` | `attivo = false;` |

> **Nota:** `millis() - t0` va sempre scritto come sottrazione tra `unsigned long`.
> Così il conteggio resta corretto anche quando `millis()` riparte da zero, dopo circa 49 giorni.

### **Toggle con polling del rilascio temporizzato con get()**

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
#include "urutils.h"
int led = 13;
int led2 = 27;
byte pulsante = 12;
bool stato = LOW;
DiffTimer t1;
DiffTimer debt;

void blink(byte led) {
  digitalWrite(led, !digitalRead(led));
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(pulsante, INPUT);
  t1.start();// attivazione blink
}

// loop principale
void loop() {
  if (digitalRead(pulsante)) {// polling pulsante premuto
    debt.start();
    debt.reset();// ancora premuto: il conteggio riparte   // <<<
  }else if (debt.get() > 50) { // disarmo del timer al timeout
    debt.stop(); // disarmo del timer
    debt.reset();
    stato = !stato;
    digitalWrite(led2, stato);
  }
  if (t1.get() > 1000) { // polling timer blink
    t1.reset(); // riarmo timer blink
    blink(led);
  }
  delay(10);
}
```

Il commento `// polling pulsante non premuto` nell'originale era sbagliato: con il pull-down
`digitalRead()` vale HIGH quando il pulsante è **premuto**.

---

**Verifica.** Le tre versioni corrette sono state simulate con due clic, rimbalzi da 1 a 45 ms
e 21 istanti di rilascio diversi (168 casi per versione, pull-down e pull-up): lo stato cambia
sempre esattamente una volta per clic. L'ultima è corretta anche togliendo il `delay(10)`.
