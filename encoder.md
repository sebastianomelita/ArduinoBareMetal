>[Torna all'indice](indexpulsanti.md)

## **ENCODER ROTATIVO**

Esistono varie tipologie di encoder con numeri di step variabili e meccanismi interni differenti.
I più comuni ed economici reperibili in giro, basano il loro funzionamento su due switch interni che mutano il loro stato sulla base di una ruota dentata.

<img src="img\encoder2.jpg" alt="alt text" width="300">

L’ordine con il quale questi switch vengono aperti o chiusi, cioè chi si apre prima o dopo, determina il senso di rotazione.

Un esempio di encoder rotativo potrebbe essere la rotellina del mouse, o la manopola del volume di alcuni impianti HiFi digitali.

<img src="img\encoder.svg" alt="alt text" width="300">

Pins:
- **CLK.**	Pin che utilizzeremo per conteggiare le **variazioni di stato**
- **DT.**	Pin che utilizzeremo per calcolare la **direzione**
- **SW.**	Switch interno attivabile alla **pressione** del pomello
- **+5V.** 	di Arduino
- **GND.**	GND di Arduino

Il modulo KY-040 include due **resistori pull-up** interni che collegano i pin CLK e DT a VCC. La simulazione solleva sempre questi pin, anche se si lascia fluttuante il pin VCC.

<img src="img\wokwi-ky-040-timing-cd6fe446378352c1416ef8817f1e5abf.webp" alt="alt text" width="600">

L'encoder rotativo offre due **modalità di interazione**:
- **Rotazione**: è possibile ruotare la manopola facendo clic sulle frecce. La freccia superiore lo ruota di un passo in senso orario, mentre la freccia inferiore lo ruota di un passo in senso antiorario. La rotazione della manopola produrrà segnali digitali sui pin DT e CLK, come spiegato di seguito.
- **Pulsante**: fare clic sulla manopola per premere il pulsante. Mentre è premuto, il pulsante collega il pin SW con il pin GND.
Ogni volta che l'utente ruota la manopola, viene prodotto un segnale LOW sui pin DT e CLK:

**Verso** di rotazione:
- La rotazione in **senso orario** fa sì che il pin **CLK** si abbassi **prima**, quindi anche il pin DT si abbassi.
- La rotazione in **senso antiorario** fa sì che il pin **DT** si abbassi **prima**, quindi il pin CLK si abbassi.


### **Encoder rotativo mediante polling del segnale CK**

In questo esempio, l'encoder rotativo è stato gestito con l'algoritmo con cui tipicamente si possono discriminare i fronti di un interruttore a partire dal polling dei livelli dei segnali da questo generato. In questo caso il **fronte** che viene rilevato **direttamente** è quello di **discesa** del piedino **CK**. L'avvenuto passaggio del fronte sul segnale **DT** è valutato **indirettamente** osservando il **livello** letto sul piedino corrisposndente tramite l'istruzione ```digitalRead(ENCODER_DT)```.

```C++
// KY-040 Rotary Encoder Example
// Taken from: https://docs.wokwi.com/parts/wokwi-ky-040
// Copyright (C) 2021, Uri Shaked

#define ENCODER_CLK 2
#define ENCODER_DT  3
int prevClk = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
}

void loop() {
  int clk = digitalRead(ENCODER_CLK); // polling di CK attuale
  if (prevClk == HIGH && clk == LOW) { // selezione del FALLING di CK
    int dtValue = digitalRead(ENCODER_DT);// polling di DT
    if (dtValue == HIGH) { // se DT non è ancora andato in FALLING
      Serial.println("Rotated clockwise ⏩");
    }
    if (dtValue == LOW) { // se DT è già andato in FALLING
      Serial.println("Rotated counterclockwise ⏪");
    }
  }
  prevClk = clk; // il polling del CK attuale diventa il polling del CK precedente
}
```
- Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/389913527165282305


### **Encoder rotativo mediante interrupt sul segnale CK**

In questo esempio, l'encoder rotativo è stato gestito con l'algoritmo con cui tipicamente si possono discriminare i fronti di un interruttore mediante interrupt. In questo caso il **fronte** che viene rilevato **direttamente** è quello di **discesa** del piedino **CK**. L'avvenuto passaggio del fronte sul segnale **DT** è valutato **indirettamente** osservando il **livello** letto sul piedino corrisposndente tramite l'istruzione ```digitalRead(ENCODER_DT)```.


```C++
/* KY-040 Rotary Encoder Counter

   Rotate clockwise to count up, counterclockwise to counter done.

   Press to reset the counter.

   Copyright (C) 2021, Uri Shaked
*/

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define ENCODER_CLK 2
#define ENCODER_DT  3
#define ENCODER_SW  4

volatile int counter = 0;

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize encoder pins
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoder, FALLING);
}

void readEncoder() {
  int dtValue = digitalRead(ENCODER_DT);
  if (dtValue == HIGH) {// DT dopo
    counter++; // Clockwise
  }
  if (dtValue == LOW) {// DT prima
    counter--; // Counterclockwise
  }
}

// Get the counter value, disabling interrupts.
// This make sure readEncoder() doesn't change the value
// while we're reading it.
int getCounter() {
  int result;
  noInterrupts(); // inizio corsa critica
  result = counter;
  interrupts();
  return result;  // fine corsa critica
}

void resetCounter() {
  noInterrupts(); // inizio corsa critica
  counter = 0;
  interrupts();   // fine corsa critica
}

void loop() {
  lcd.setCursor(3, 0);
  lcd.print("Counter:");
  lcd.setCursor(7, 1);
  lcd.print(getCounter());
  lcd.print("        ");

  if (digitalRead(ENCODER_SW) == LOW) {
    resetCounter();
  }
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/389923052943921153

Le variabili **condivise** tra una ISR e il loop() andrebbero protette, da accessi **paralleli** e concorrenti da parte di entrambe, tramite delle **corse critiche** che rendano l'accesso **strettamente sequenziale**. Inoltre le variabili condivise devono sempre essere dichiarate con il qualificatore ```volatile``` per forzarne la modifica istantanea anche sui registri della CPU. 

Gli **accessi paralleli** non sono un problema quando le **istruzioni** sono **atomiche**, cioè non interrompibili. Le istruzioni atomiche o sono eseguite per intero o non sono eseguite affatto. In questo caso gli **accessi**, sia in lettura che in scrittura, sono in realtà, a basso livello, **intrinsecamente sequenziali**.

Nei microcontrollori attuali, in genere **nessuna istruzione** gode della proprietà di essere **atomica** con una sola eccezione per la lettura e scrittura delle **variabili ad 8 bit**. Per le variabili codificate con **8 bit** l'accesso a basso livello (linguaggio macchina) è intrinsecamente garantito essere **atomico**. Per queste variabili rimane comunque la necessita dell'uso del qualificatore ```volatile```.

Le **modifiche** a valori con codifiche **maggiori di 8 bit** sono in genere **non atomiche**, pertanto le variabili a 16 o 32 bit andrebbero gestite con gli interrupt disabilitati (sezione critica). Tuttavia, gli interrupt vengono disabilitati di default durante una routine di servizio di interrupt, quindi, non potendo verificarsi il danneggiamento di una variabile multibyte in una ISR, le **sezioni critiche** vanno inserite soltanto nel ```loop()```.

Le variabile **condivisa** tra ISR e loop() e **8 bit** è ```stato``` ed è stata dichiarata come ```volatile``` e ogni suo accesso (in lettura e scrittura) è  stato racchiuso in una  **sezione critica** dove l'acccesso parallelo è stato disabilitato in favore di uno **strettamente sequenziale** che garantisce l'atomicità delle istruzioni in essa racchiuse.

Per quanto riguarda le sezioni critiche si può approfondire in [sezioni critiche](intsezionicritiche.md)

### **Encoder rotativo e rimbalzi**

<img src="img\rotary_2.png" alt="alt text" width="1000">

In realtà, i decoder rotativi, essendo realizzati con interruttori, sono soggetti al fenomeno dei rimbalzi tipici di tutti gli interruttori. 

I debouncer èpossono essere realizzati utilizzando dei filtri:
- basati sul tempo (timers) analoghi a quanto già visto per i normali pulsanti
- macchine a stati finiti

Il più grande vantaggio dell'utilizzo di una macchina a stati rispetto ad altri algoritmi è che questo ha un antirimbalzo intrinseco integrato. Altri algoritmi emettono output spuri con rimbalzo dell'interruttore, ma questo passerà semplicemente da uno stato all'altro finché il rimbalzo non si stabilizza, quindi continuerà lungo la macchina a stati .

Un effetto collaterale del rimbalzo è che le rotazioni veloci possono far saltare i passaggi. Non richiedendo ritardi, è possibile misurare con precisione le rotazioni veloci.

Un altro vantaggio è la capacità di gestire correttamente stati errati, come quelli dovuti a EMI, ecc.

https://github.com/buxtronix/arduino/tree/master/libraries/Rotary

### **Encoder rotativo mediante polling con debouncer basato sul tempo**

```C++
// KY-040 Rotary Encoder Example
// Taken from: https://docs.wokwi.com/parts/wokwi-ky-040
// Copyright (C) 2021, Uri Shaked

#define ENCODER_CLK 2
#define ENCODER_DT  3
int prevClk = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10;

void setup() {
  Serial.begin(115200);
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
}

void loop() {
  int clk = digitalRead(ENCODER_CLK); // polling di CK attuale
  if (prevClk == HIGH && clk == LOW) { // selezione del FALLING di CK
    // If enough time has passed since the last state change
    if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis(); // riarmo il timer
      int dtValue = digitalRead(ENCODER_DT);// polling di DT
      if (dtValue == HIGH) { // se DT non è ancora andato in FALLING
        Serial.println("Rotated clockwise ⏩");
      }
      if (dtValue == LOW) { // se DT è già andato in FALLING
        Serial.println("Rotated counterclockwise ⏪");
      }
    }
  }
  prevClk = clk; // il polling del CK attuale diventa il polling del CK precedente
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/389969556548332545

### **Encoder rotativo mediante interrupt con debouncer basato sul tempo**

```C++
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define ENCODER_CLK 2
#define ENCODER_DT  3
#define ENCODER_SW  4
#define DEBOUNCE_DELAY 50

int counter = 0;
volatile unsigned long lastDebounceTime = 0;

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize encoder pins
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoder, FALLING);
}

void readEncoder() {
  // Perform debounce
  if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
    lastDebounceTime = millis(); // riarma il timer

    int dtValue = digitalRead(ENCODER_DT);
    if (dtValue == HIGH) {// DT dopo
      counter++; // Clockwise
    }
    if (dtValue == LOW) {// DT prima
      counter--; // Counterclockwise
    }
  }
}

// Get the counter value, disabling interrupts.
// This make sure readEncoder() doesn't change the value
// while we're reading it.
int getCounter() {
  int result;
  noInterrupts(); // inizio corsa critica
  result = counter;
  interrupts();
  return result;  // fine corsa critica
}

void resetCounter() {
  noInterrupts(); // inizio corsa critica
  counter = 0;
  interrupts();   // fine corsa critica
}

void loop() {
  lcd.setCursor(3, 0);
  lcd.print("Counter:");
  lcd.setCursor(7, 1);
  lcd.print(getCounter());
  lcd.print("        ");

  if (digitalRead(ENCODER_SW) == LOW) {
    resetCounter();
  }
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/389966992054192129

Con tabella e polling metodo tracking

Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/389979138658609153

Con tabella e interrupts metodo array

https://wokwi.com/projects/389999046461201409

Sitografia:
- https://docs.wokwi.com/parts/wokwi-ky-040
- https://www.lombardoandrea.com/utilizzare-un-encoder-rotativo-con-arduino/
- https://www.pinteric.com/rotary.html
- http://www.technoblogy.com/show?1YHJ
- https://github.com/fryktoria/FR_RotaryEncoder/blob/master/src/FR_RotaryEncoder.cpp

>[Torna all'indice](indexpulsanti.md)

