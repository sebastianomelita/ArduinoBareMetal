>[Torna all'indice](indexpulsanti.md)   >[versione in Python](toggle-blink-pulldown-mpy.md)

###  **PULSANTE TOGGLE**

Si vuole realizzare un pulsante con una memoria dello stato che possa essere modificata ad ogni pressione. Pressioni successive in sequenza accendono e spengono un led. Quindi, a seconda da quando si comincia, pressioni in numero pari accendono mentre quelle in numero dispari spengono, oppure al contrario, pressioni in numero pari spengono mentre quelle in numero dispari accendono.

Inizialmente si potrebbe essere tentati di provare seguente soluzione, adattando la strategia del pulsante precedente introducendo una variabile che conservi lo stato del pulsante che chiameremo **closed**.

```C++
byte in;
byte pulsante =2;
boolean closed=false; // stato pulsante
void setup()
{
	pinMode(pulsante, INPUT);
}

void loop()
{
	in = digitalRead(pulsante);
	if(in==HIGH){ // selezione del livello alto (logica di comando)
		closed = !closed;
		digitalWrite(led,closed);  //scrittura uscita
	}
}
```
Purtroppo questa soluzione ha un paio di **problemi** che ne pregiudicano il **funzionamento corretto**.

**Il primo** è relativo alla **selezione del tipo di evento in ingresso**. In questa soluzione viene rilevata la pressione del pulsante **sul livello** dell’ingresso. Il problema è che il livello viene rilevato per tutto il tempo che il tasto è premuto mediante una lettura per ogni loop(). Il numero di queste letture è imprevedibile sia perché sono moltissime al secondo e sia perché la durata della pressione dipende dall’utente. In più, ad ogni lettura viene modificato lo stato del pulsante con l’istruzione closed=!closed, ne consegue che lo stato finale del pulsante è il risultato di una catena di commutazioni che termina dopo un tempo casuale: abbiamo realizzato una slot machine!.

###  **RILEVATORE DI TRANSITO**

Si vuole realizzare un rilevatore del transito di un oggetto su una fotocellula. Poichè la fotocellula legge l'assenza del fascio luminoso di ritorno, la sua uscita è ripetutamente vera fino a che l'oggetto non ha completato il suo passaggio. Un passaggio temporalmente lungo genera letture ripetute del transito segnalando più oggetti in corrispondenza di uno solo.

## **SOLUZIONE**

La **soluzione** è **modificare il tipo di selezione** **dell’evento** in ingresso, che deve stavolta avvenire sui **fronti** piuttosto che sui livelli. I fronti sono due, **salita** o **discesa**. Se si rileva il fronte di **salita** si cattura l’evento di **pressione** del pulsante, se si rileva il fronte di **discesa** si cattura l’evento di **rilascio** del pulsante. Ma come facciamo a capire quando siamo in presenza di un **evento di fronte**? Di salita o di discesa?

![fronti](fronti.gif)

Una **evento di fronte** si può rilevare in due modi:

- **Polling degli ingressi.** Lo scopo è rilevare una **transizione di livello**. Si tratta di campionare periodicamente il livello di un ingresso con l’obiettivo di rilevare un fronte. Un fronte può essere **rilevato** sostanzialmente in **due maniere**:

     - analisi su **due loop consecutivi**, memorizzando sempre l’ultimo campione misurato (il **valore corrente**) su una **variabile globale**. Al **loop successivo** questo valore sarà diventato il **valore precedente** da **confrontare** col nuovo valore corrente, alla ricerca di eventuali variazioni (**fronti**). Il polling, cioè il campionamento, può essere effettuato ad ogni loop, o può essere **decimato**, cioè eseguito periodicamente **ogni tot loop scartati** (decimati). La **misura** in questo metodo **non è mai bloccante,** non essendo necessarie attese per rilevare un fronte.

     - analisi su un **unico loop**, sfruttando le **attese bloccanti** di un **thread** fino al valore **opposto** a quello che ha attivato la misura, ciò garantisce che la prima lettura di un ingresso è sempre il **primo fronte** (ad es., di salita al valore HIGH). Appena l’attesa sul valore opposto (WaitUntil LOW) termina, allora accade la **rilevazione** certa del secondo **fronte** (discesa). La **misura** in questo metodo **è sempre bloccante,** essendo necessaria almeno una attesa per rilevare un fronte.

- **Interrupt o eventi.** Su alcune porte di un microcontrollore può essere abilitato il riconoscimento di un evento di interrupt, segnalabile, a scelta, su fronte di salita, fronte di discesa, entrambi i fronti. L’interrupt determina la **chiamata asincrona** di una funzione di servizio che esegue il codice con la logica di comando in risposta all’evento. La funzione è definita esternamente al loop() principale e la sua esecuzione è del tutto indipendente da questo. 

In figura sono evidenziati i **campionamenti** eseguibili, ad esempio, ad ogni loop(), oppure ad intervalli di tempo preordinati stabiliti da uno schedulatore.

![transizioni](transizioni.png)

Si nota chiaramente che in corrispondenza di ogni fronte, prima e dopo, si misurano differenze di livello. In particolare si può avere:

- Un **fronte di salita** se la **transizione** avviene dal livello basso a quello alto

- Un **fronte di discesa** se la **transizione** avviene dal livello alto a quello basso

Se un rilevatore si limita a segnalare un **generico fronte**, allora per stabilire in quale ci troviamo, basta determinare, al momento della segnalazione, in **quale livello** si trova **l’ingresso**: se è al livello alto è un fronte di salita, se è a quello basso è un fronte di discesa.

Il **secondo problema** è costituito dal fenomeno dei **rimbalzi**. Si palesano come una sequenza di rapide oscillazioni che hanno sia fronti di salita che di discesa. Se l’accensione di un led è associata ad un fronte e il suo spegnimento a quello successivo, allora la pressione di un pulsante realizza, di fatto, la solita slot machine…è necessario un algoritmo di debouncing.

Gli esempi successivi sono stati esposti relativamente al problema del **pulsante toggle** premendo il quale, ad ogni lettura di un fornte, si genera la modifica di una opportuna **variabile di stato** (```stato``` negli esempi).

A ben vedere, le soluzioni proposte di seguito possono essere tutte facilmente adattate alla realizzazione di un **sensore di transito** sostituendo la pressione del pulsante con la lettura del sensore e interpretando la variabile ```stato``` con la **variabile di stato** che deve tenere traccia del passaggio rilevato.

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
Da sola `waitUntilNB()` conta dalla prima lettura "premuto": al rilascio scatterebbe subito,
senza debounce, e un rimbalzo del contatto la farebbe scattare una seconda volta. Per questo,
a ogni lettura "premuto", si chiama `debt.reset()`: i 50 ms si contano dall'ultima lettura
"premuto", come nella versione con DiffTimer.

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

## 6. Versione con interrupt: la ISR registra i fronti, il loop() decide

La ISR è armata su entrambi i fronti (`CHANGE`): salva l'istante di ogni fronte, rimbalzi
compresi, e alza `segnale` quando il pulsante risulta premuto. Il `loop()` chiama la callback
della pressione e, quando il pin è rilasciato e da più di 50 ms non ci sono fronti, quella del
rilascio. Le callback girano nel `loop()`; la pressione viene registrata anche se il `loop()`
è occupato. Il pulsante va su un pin con interrupt (Arduino Uno: 2 o 3).

```cpp
const byte P1       = 2;    // pulsante (pull-down), pin con interrupt
const byte LED      = 13;   // task 1: LED comandato dal pulsante (toggle)
const byte LEDBLINK = 12;   // task 2: LED lampeggiante
bool stato = LOW;
volatile bool segnale = false;    // pressione segnalata dalla ISR
volatile unsigned long tFronte;   // istante dell'ultimo fronte (anche rimbalzi)
bool attivo = false;              // pressione in corso
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

void isrChange() {
  tFronte = millis();             // registra ogni fronte
  if (digitalRead(P1) == HIGH) {
    segnale = true;               // pressione
  }
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(LEDBLINK, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(P1), isrChange, CHANGE);
}

void loop() {
  // task 1: pulsante (eventi registrati dalla ISR)
  if (segnale && !attivo) {
    attivo = true;
    doOnRise();
  }
  if (attivo) {
    noInterrupts();               // copia protetta di tFronte
    unsigned long t = tFronte;
    interrupts();
    if (digitalRead(P1) == LOW && millis() - t > 50) {
      attivo = false;
      segnale = false;
      doOnFall();
    }
  }

  // task 2: blink ogni 1000 ms
  if (millis() - precBlink >= 1000) {
    precBlink = millis();
    blink(LEDBLINK);
  }
}
```

## 7. Versione con interrupt: tutto nella ISR, fronti alternati

La ISR è armata su un fronte alla volta: prima la pressione, poi il rilascio. Ogni fronte
viene accettato solo se dall'ultimo accettato sono passati più di 50 ms. Le callback girano
nella ISR e devono restare brevi; il `loop()` contiene solo il blink, e il toggle funziona anche
se il `loop()` è bloccato.

```cpp
const byte P1       = 2;    // pulsante (pull-down), pin con interrupt
const byte LED      = 13;   // task 1: LED comandato dal pulsante (toggle)
const byte LEDBLINK = 12;   // task 2: LED lampeggiante
volatile bool stato = LOW;
unsigned long tUltimo = 0;        // ultimo fronte accettato (solo nelle ISR)
unsigned long precBlink = 0;

void doOnRise() {
  // PRESSIONE (fronte di salita): qui vanno le azioni immediate alla pressione.
  // Eseguita nella ISR: breve!
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa): qui vanno le azioni al rilascio.
  // Eseguita nella ISR: breve!
  // Esempio: toggle, inverte lo stato del LED a ogni rilascio.
  stato = !stato;
  digitalWrite(LED, stato);
}

void blink(byte pin) {
  digitalWrite(pin, !digitalRead(pin));   // inverte il LED lampeggiante
}

void isrRel();

void isrPress() {                 // armata sulla pressione
  if (millis() - tUltimo > 50) {
    tUltimo = millis();
    doOnRise();
    attachInterrupt(digitalPinToInterrupt(P1), isrRel, FALLING);
  }
}

void isrRel() {                   // armata sul rilascio
  if (millis() - tUltimo > 50) {
    tUltimo = millis();
    doOnFall();
    attachInterrupt(digitalPinToInterrupt(P1), isrPress, RISING);
  }
}

void setup() {
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(LEDBLINK, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(P1), isrPress, RISING);
}

void loop() {
  // task 1: gestito interamente dalle ISR

  // task 2: blink ogni 1000 ms
  if (millis() - precBlink >= 1000) {
    precBlink = millis();
    blink(LEDBLINK);
  }
}
```

Per il pull-up, nelle sezioni 6 e 7 oltre al `!` davanti a `digitalRead(P1)` vanno scambiati
`RISING` e `FALLING`.

## Riferimenti

- [urutils.h](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/urutils.h): sorgente della libreria
- [toggle.md](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/toggle.md): esempi di pulsante toggle
- [pulsanti-rilascio.md](pulsanti-rilascio.md): versioni complete, pull-down e pull-up
- [pulsanti-rilascio-corte.md](pulsanti-rilascio-corte.md): versioni corte, solo azione al rilascio
