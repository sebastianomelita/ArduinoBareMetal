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

Il modulo KY-040 include due **resistori pull-up** interni che collegano i pin a_current e DT a VCC. La simulazione solleva sempre questi pin, anche se si lascia fluttuante il pin VCC.

L'encoder rotativo offre due **modalità di interazione**:
- **Rotazione**: è possibile ruotare la manopola facendo clic sulle frecce. La freccia superiore lo ruota di un passo in senso orario, mentre la freccia inferiore lo ruota di un passo in senso antiorario. La rotazione della manopola produrrà segnali digitali sui pin DT e CLK, come spiegato di seguito.
- **Pulsante**: fare clic sulla manopola per premere il pulsante. Mentre è premuto, il pulsante collega il pin SW con il pin GND.
Ogni volta che l'utente ruota la manopola, viene prodotto un serie di segnali sui pin DT e CLK.

<img src="img\tansitions.jpg" alt="alt text" width="600">

La **fisica interna** del dispositivo fa si che, per stabilire il numero di scatti e il verso di rotazione dall'osservazione dei due ingressi A e B, bisogna esaminare **almeno due gruppi** di valori consecutivi al fine di isolare il **fronte** di un piedino e il **valore** di un altro.

Alla luce di ciò, la valutazione di  numero di **scatti** e **verso** delle rotazioni si può ottenere essenzialmente con **tre tecniche**:
- Attendere il verificarsi di una **transizione pilota** su di un piedino (ad esempio A) che indica lo **scatto** e valutare, in corrispondenza di questa, quale valore assume l'altro piedino (in questo caso B) al fine di stabilire il **verso**, cioè il valore che identifica una rotazione oraria (CW) o quello che identifica quella antioraria (CCW). Questa modalità, da sola senza null'altro, non è in grado di filtrare le transizioni spurie.
- **Tabella delle transizioni**. Leggere, grazie a 2 due loop consecutivi, una sequenza BABA, e decidere se accettarla o considerarla una **sequenza scorretta** (spuria). Una volta accettata, è possibile usare una sequenza per stabilire se la rotazione ha il verso CW o quello CCW. La geometria interna del **dispositivo** permette solamente 4 **sequenze ammissibili** di tipo BABA e si può, a questo punto, stabilirne la politica di gestione: controllare se ne deve arrivare corretta una in particolare, se ne devono arrivare alcune o se devono arrivare corrette proprio tutte prima di convalidare il risultato. E' possibile anche decidere di accorpare più di 4 transizioni, ad esempio 8 transizioni BABABABA, per ridurre ulteriormente la probabilità di errore nel condurre l'analisi delle sequenze corrette .
   
- **Macchina a stati finiti**. E' una tecnica che per adesso non tratteremo.

<img src="img\wokwi-ky-040-timing-cd6fe446378352c1416ef8817f1e5abf.webp" alt="alt text" width="800">

Per cominciare, ed essere subito operativi, usiamo la tecnica della **transizione pilota**, e tra tutte scegliamo la transizione in FALLING del piedino A per selezionare la **transizione** che indica lo **scatto** e il valore del piedino B per stabilire il **verso** della rotazione:
- La rotazione in **senso orario** fa sì che il pin **CLK** si abbassi **prima**, mentre il pin DT si abbassi dopo.
- La rotazione in **senso antiorario** fa sì che il pin **DT** si abbassi **prima**, mentre il pin a_current si abbassi dopo.

## **TECNICA DELLA TRANSIZIONE PILOTA**

### **Encoder rotativo mediante polling del segnale CK**

In questo esempio, l'encoder rotativo è stato gestito con l'algoritmo con cui tipicamente si possono discriminare i fronti di un interruttore comune a partire dal polling dei livelli dei segnali da questo generato. In questo caso il **fronte** che viene rilevato **direttamente** è quello di **discesa** del piedino **CK**. L'avvenuto passaggio del fronte sul segnale **DT** è valutato **indirettamente** osservando il **livello** letto sul piedino corrispondente tramite l'istruzione ```digitalRead(ENCODER_DT)```.

```C++
// KY-040 Rotary Encoder Example
// Taken from: https://docs.wokwi.com/parts/wokwi-ky-040
// Copyright (C) 2021, Uri Shaked

#define ENCODER_CLK 2
#define ENCODER_DT  3
int a0 = HIGH; // A passato

void setup() {
  Serial.begin(115200);
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
}

void loop() {
  int a = digitalRead(ENCODER_CLK); // polling di CK attuale
  
  if (a0 == HIGH && a == LOW) { // selezione del FALLING di CK
    int b = digitalRead(ENCODER_DT);// polling di DT
    if (b == HIGH) { // se DT non è ancora andato in FALLING
      Serial.println("Rotated clockwise ⏩");
    }
    if (b == LOW) { // se DT è già andato in FALLING
      Serial.println("Rotated counterclockwise ⏪");
    }
  }
  a0 = a; // il polling del CK attuale diventa il polling del CK precedente
}
```
- Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/389913527165282305


### **Encoder rotativo mediante interrupt sul segnale CK**

In questo esempio, l'encoder rotativo è stato gestito con l'algoritmo con cui tipicamente si possono discriminare i fronti di un interruttore mediante interrupt. In questo caso il **fronte** che viene rilevato **direttamente** è quello di **discesa** del piedino **CK**. L'avvenuto passaggio del fronte sul segnale **DT** è valutato **indirettamente** osservando il **livello** letto sul piedino corrispondente  tramite l'istruzione ```digitalRead(ENCODER_DT)```.


```C++
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
  int b = digitalRead(ENCODER_DT);
  if (b == HIGH) {// DT dopo
    counter++; // Clockwise
  }
  if (b == LOW) {// DT prima
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

I debouncer possono essere realizzati utilizzando dei filtri:
- basati sul tempo (timers) analoghi a quanto già visto per i normali pulsanti
- macchine a stati finiti
- basato su tabella di verita di ingressi passati e presenti (ad es. BABA)

La durata tipica, per questi dispositivi, del fenomeno dei rimbalzi è di circa 10 msec.

I rimbalzi rappresentano il limite fisico intrinseco del dispositivo per quanto riguarda la sua velocità di rotazione. Nessun sensore è immune ai rumori, la granularità della misura è sempre finita. In questo caso è data dai 10 msec della fisica dei suoi rimbalzi. Si può migliorarla sfruttando la ridondanza delle transizioni. 

In altre parole, a ridosso dei 10 msec gli errori sono sempre possibili ma la ridondanza di informazione presente nella tabella di verità permette di correggerne alcuni scartando le transizioni non ammissibili. Non è certezza assoluta ma probabilità, che diventa sempre maggiore quanto più indietro si vanno ad esaminare le transizioni a partire dall'istante corrente. Se il matching è sempre su valori ammissibili allora la probabilità che siano tutti frutti di un unico treno di rimbalzi diventa molto ridotta.

Inoltre la tabella della verità delle transizioni passate ed attuali rispetta grossomodo la codifica Gray per cui un singolo errore di un bit cambia sempre cifre poco significative della codifica generando variazioni unitarie sul valore attuale di questa.

Il più grande vantaggio dell'utilizzo di una macchina a stati rispetto ad altri algoritmi è che questo ha un antirimbalzo intrinseco integrato. Altri algoritmi emettono output spuri con rimbalzo dell'interruttore, ma questo passerà semplicemente da uno stato all'altro finché il rimbalzo non si stabilizza, quindi continuerà lungo la macchina a stati .

Un effetto collaterale del rimbalzo è che le rotazioni veloci possono far saltare i passaggi. Non richiedendo ritardi, è possibile misurare con precisione le rotazioni veloci.

Un altro vantaggio è la capacità di gestire correttamente stati errati, come quelli dovuti a EMI, ecc.

https://github.com/buxtronix/arduino/tree/master/libraries/Rotary

### **Encoder rotativo mediante polling con debouncer basato sul tempo**

```C++
#define ENCODER_CLK 2
#define ENCODER_DT  3
int a0 = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10;

void setup() {
  Serial.begin(115200);
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
}

void loop() {
  int a = digitalRead(ENCODER_CLK); // polling di CK attuale

  if (a0 == HIGH && a == LOW) { // selezione del FALLING di CK
    // If enough time has passed since the last state change
    if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis(); // riarmo il timer
      int b = digitalRead(ENCODER_DT);// polling di DT
      if (b == HIGH) { // se DT non è ancora andato in FALLING
        Serial.println("Rotated clockwise ⏩");
      }
      if (b == LOW) { // se DT è già andato in FALLING
        Serial.println("Rotated counterclockwise ⏪");
      }
    }
  }
  a0 = a; // il polling del CK attuale diventa il polling del CK precedente
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

    int b = digitalRead(ENCODER_DT);
    if (b == HIGH) {// DT dopo
      counter++; // Clockwise
    }
    if (b == LOW) {// DT prima
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

## **TECNICA DELLA TABELLA DI TRANSIZIONE**

<img src="img\babatable.png" alt="alt text" width="1400">

Negli esempi che vedremo a seguire hanno tutti in comune il procedimento di ricevere 4 sequenze complete prima di prendere una decisione riguardo al verso dello scatto proprio sull'ultima. Sembrerebbe esserci un'attesa nella decisione, ma questa in realtà è solamente apparente perchè ogni nuova sequenza si ottiene shiftando la precedente di appena 1 bit, e non come sembra, creandone una nuova ogni 4 bit ricevuti. Alla fine, il **ritardo complessivo** è sempre di **4 bit**, corrispondenti ad una **sequenza BABA**.

Complessivamente, devono essere ricevuti almeno 4 bit su due porte al fine di avere tutte le informazioni per ricostruire un ciclo completo dei due segnali AB che abbia la proprietà di comprendere almeno un fronte il segnale A e uno pure per il segnale B. I due contatti A e B strisciano sulla stessa ruota dentata e sono, a **velocità costante**, sfasati di **90°**. Sotto questo limite di numero di bit non è possibile andare altrimenti uno dei due parametri, velocità e verso di rotazione, rimangono incerti. 

Con i **primi due bit** si rileva il fronte di una rotazione, con gli ultimi due il **verso** della stessa. La distanza temporale tra due fronti permette la misura della velocità. Per quanto riguarda la rilevazione del fronte, basterebbe il segnale CK (A) e quindi complessivamente basterebbe una sequenza ABA per determinare completamente rotazione e verso, difatti questa è stata la soluzione adottata nei primi algoritmi proposti. Nelle altre soluzioni che vedremo, il campione B della sequenza BABA fornisce una informazione di ridondanza utile a scartare sequenze affette da rimbalzi o disturbi.

A **velocità costante** maggiore i fronti sono **equidistanziati** ma  più **ravvicinati** che a velocità costante minore.

Una **variazione di velocità** causa una modifica dello sfasamento dei segnali (dovuto al cambiamento della distanza temporale tra due fronti consecutivi):
- se la manopola **accelera** i fronti progressivamente diventano sempre più **vicini**.
- se la manopola **rallenta** i fronti progressivamente diventano sempre più **lontani**.

In **direzione CW** il fronte di **discesa** di CK "vede" un **valore alto** di DT. In **direzione CCW** il fronte di **discesa** di CK "vede" un **valore basso** di DT.

<img src="img\Incremental_directional_encoder.gif" alt="alt text" width="400">

<img src="img\QuadratureOscillatingShaft.png" alt="alt text" width="600">

Sopra sono rappresentati i segnali in quadratura di un **albero oscillante**. Si può identificare la direzione il **senso orario** (CW) da 'Data (DT)' sul fronte di **discesa** di un segnale 'Clock (CK)' che vede un **DT basso**. Quando l'albero **cambia direzione**, allora **rallenta** e inverte, in senso antiorario (CCW) modificando l'ampiezza e la fase dell'impulso in modo che ill fronte di **discesa** del segnale 'Clock (CK)' adesso veda un **DT alto**.


### **Encoder rotativo con tabella e polling metodo tracking**

```C++
/*
---------------------
| Sequenze ammesse  |
---------------------
| 0 1 1 1 CW  last  |
| 0 0 0 1 CW        |
| 1 0 0 0 CW        |
| 1 1 1 0 CW  first |
|-------------------|
| 1 0 1 1 CCW last  |
| 0 0 1 0 CCW       |
| 0 1 0 0 CCW       |
| 1 1 0 1 CCW first |
---------------------
*/

#define ENCODER_CLK 2
#define ENCODER_DT  3
int a0 = LOW;
int b0 = LOW;
int counter = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
}

void loop() {
  int a = digitalRead(ENCODER_CLK); // polling di CK/A attuale
  int b = digitalRead(ENCODER_DT);  // polling di DT/B attuale

  if(a0 == a){
    //Serial.println("Apast = Acurrent");
    if((a == 1) && (b0 < b)){counter++;Serial.print("0 1 1 1 CW ⏩ ");Serial.println(counter);}//   0 1 1 1 CW  fine scatto              
    if((a == 1) && (b0 > b)){Serial.println("1 1 0 1 CCW ");}// 1 1 0 1 CCW
    if((a == 0) && (b0 > b)){Serial.println("1 0 0 0 CW ");}//   1 0 0 0 CW                   
    if((a == 0) && (b0 < b)){Serial.println("0 0 1 0 CCW ");}// 0 0 1 0 CCW
  }
  if((a0 < a) && (b0 == LOW && b == LOW)){Serial.println("0 0 0 1 CW ");}//     0 0 0 1 CW  
  if((a0 < a) && (b0 == HIGH && b == HIGH)){counter--;Serial.print("1 0 1 1 CCW ⏪ ");Serial.println(counter);}// 1 0 1 1 CCW fine scatto
  if((a0 > a) && (b0 == LOW && b == LOW)){Serial.println("0 1 0 0 CCW ");}//   0 1 0 0 CCW
  if((a0 > a) && (b0 == HIGH && b == HIGH)){Serial.println("1 1 1 0 CW ");}//   1 1 1 0 CW   
  
  // increment alarm count
  // test for over/under flows
  a0 = a;
  b0 = b;
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/389979138658609153

### **Encoder rotativo con tabella e polling metodo tracking migliorato**

Nell'esempio precedente, benchè sono state tracciate (tracking) tutte le sequenze BABA sui due ingressi CK e DT solamente a scopo dimostrativo per stamparne i valori, in realtà solo una è stata effettivamente usata per aggiornare il contatore degli scatti. Le altre sequenza potevano benissimo essere scartate, oppure potevano essere adoperate per eseguire una validazione più accurata tenendo conto della storia passata dei campioni BABA ricevuti. 

Ad esempio, sostituendo tutti gli if a cascata con un costrutto else-if è possibile eseguire una validazione ogni 16 bit ricevuti invece che ogni 4, in modo da minimizzare la probabilità di errore (probabilmente a scapito di una minore precisione nella misura delle variazioni di velocità). Oppure, per ottnere lo stesso risultato, si potrebbero contare le transizioni verificate e avalidare lo scatto solo se si raggiunge un conteggio finale di 4 transizioni.


Il risultato è esposto nel codice sottostante

```C++
/*
---------------------
| Sequenze ammesse  |
---------------------
| 0 1 1 1 CW  last  |
| 0 0 0 1 CW        |
| 1 0 0 0 CW        |
| 1 1 1 0 CW  first |
|-------------------|
| 1 0 1 1 CCW last  |
| 0 0 1 0 CCW       |
| 0 1 0 0 CCW       |
| 1 1 0 1 CCW first |
---------------------
*/
#define ENCODER_CLK 2
#define ENCODER_DT  3
int a0 = LOW;
int b0 = LOW;
int counter = 0;
int stepCount = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
}

void loop() {
  int a = digitalRead(ENCODER_CLK); // polling di CK/A attuale
  int b = digitalRead(ENCODER_DT);  // polling di DT/B attuale

  if(a0 == a){
    //Serial.println("Apast = Acurrent");
    if((a == 1) && (b0 < b)){counter++;Serial.println("0 1 1 1 CW");}//   0 1 1 1 CW  fine scatto              
    if((a == 1) && (b0 > b)){Serial.println("1 1 0 1 CCW");}// 1 1 0 1 CCW         inizio scatto 
    if((a == 0) && (b0 > b)){counter++;Serial.println("1 0 0 0 CW ");}//   1 0 0 0 CW                   
    if((a == 0) && (b0 < b)){counter--;Serial.println("0 0 1 0 CCW ");}// 0 0 1 0 CCW
  }
  if((a0 < a) && (b0 == LOW && b == LOW)){counter++;Serial.println("0 0 0 1 CW");}//     0 0 0 1 CW  
  if((a0 < a) && (b0 == HIGH && b == HIGH)){counter--;Serial.println("1 0 1 1 CCW");}// 1 0 1 1 CCW fine scatto
  if((a0 > a) && (b0 == LOW && b == LOW)){counter--;Serial.println("0 1 0 0 CCW");}//   0 1 0 0 CCW
  if((a0 > a) && (b0 == HIGH && b == HIGH)){Serial.println("1 1 1 0 CW");}//   1 1 1 0 CW          inizio scatto
  
  if (counter == 3){
    stepCount++;
    counter=0;
    Serial.print(" ⏩ ");
    Serial.println(stepCount);
  }else  if (counter == -3){
    stepCount--;
    counter=0;
    Serial.print(" ⏪ ");
    Serial.println(stepCount);
  }
   
  // increment alarm count
  // test for over/under flows
  a0 = a;
  b0 = b;
}
```

Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/390374216487657473

### **Encoder rotativo tabella e interrupt metodo tracking**

```C++
/*
---------------------
| Sequenze ammesse  |
---------------------
| 0 1 1 1 CW  last  |
| 0 0 0 1 CW        |
| 1 0 0 0 CW        |
| 1 1 1 0 CW  first |
|-------------------|
| 1 0 1 1 CCW last  |
| 0 0 1 0 CCW       |
| 0 1 0 0 CCW       |
| 1 1 0 1 CCW first |
---------------------
*/
// Define pins for the rotary encoder
#define encoderPinA  2
#define encoderPinB  3

int a0 = HIGH; // A passato
int b0 = HIGH; // B passato
volatile short counter = 0;

void setup() {
  // Set encoder pins as inputs
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);

  // Attach interrupts to encoder pins
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);

  // Initialize Serial communication
  Serial.begin(9600);
}

void loop() {
  // Do whatever you need to do with the encoder value
  // For example, you can print it out:
  //Serial.println(counter);
  delay(100); // Adjust delay as needed for your application
}

void updateEncoder() {
  int a = digitalRead(encoderPinA); // polling di CK/A attuale
  int b = digitalRead(encoderPinB);  // polling di DT/B attuale

  if(a0 == a){
    if((a == 1) && (b0 < b)){counter++;Serial.print("0 1 1 1 CW ⏩ ");Serial.println(counter);}//   0 1 1 1 CW  fine scatto              
    if((a == 1) && (b0 > b)){Serial.println("1 1 0 1 CCW ");}// 1 1 0 1 CCW
    if((a == 0) && (b0 > b)){Serial.println("1 0 0 0 CW ");}//   1 0 0 0 CW                   
    if((a == 0) && (b0 < b)){Serial.println("0 0 1 0 CCW ");}// 0 0 1 0 CCW
  }
  if((a0 < a) && (b0 == LOW && b == LOW)){Serial.println("0 0 0 1 CW ");}//     0 0 0 1 CW  
  if((a0 < a) && (b0 == HIGH && b == HIGH)){counter--;Serial.print("1 0 1 1 CCW ⏪ ");Serial.println(counter);}// 1 0 1 1 CCW fine scatto
  if((a0 > a) && (b0 == LOW && b == LOW)){Serial.println("0 1 0 0 CCW ");}//   0 1 0 0 CCW
  if((a0 > a) && (b0 == HIGH && b == HIGH)){Serial.println("1 1 1 0 CW ");}//   1 1 1 0 CW   
  
  // increment alarm count
  // test for over/under flows
  a0 = a;
  b0 = b;
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/390001300410019841

### **Encoder rotativo mediante interrupt con debouncer non basato sul tempo**


Tratto da http://www.technoblogy.com/show?1YHJ

<img src="img\staticdebounce.png" alt="alt text" width="500">

La figura si riferisce ad una **rotazione oraria** in cui ad **ogni transizione di A** viene copiato in C il valore corrente di B. Eventuali rimbalzi non modificano il valore di C perchè questo rimane influenzato solo dal valore raggiunto dal primo fronte del treno di rimbalzi di A perchè per i successivi B ha lo stesso valore di C e la condizione falsa sull'if è efficace a filtrarli. In questo caso si può notare che A e B assumono sempre lo **valori diversi** e il pattern corrispondente, **a cavallo** di una transizione di A, è prima 1 1 1 0 CW e 0 0 0 1 CCW.

Si potrebbe ricostruire pure la figura relativa ad una **rotazione antioraria** in cui **ogni transizione di A** viene copiato in C il valore corrente di B. Eventuali rimbalzi non modificano il valore di C perchè questo rimane influenzato solo dal valore raggiunto dal primo fronte del treno di rimbalzi di A perchè per i successivi B ha lo stesso valore di C e la condizione falsa sull'if è efficace a filtrarli. In questo caso si può notare che A e B assumono sempre lo **valori uguali** e il pattern corrispondente, **a cavallo** di una transizione di A, è prima 1 0 1 1 CCW e poi 0 1 0 0 CCW.

Le coppie di sequenze vengono catturate entrambe e quindi vanno poi contate una volta sola per ciascuna coppia.

```C++
/*
---------------------
| Sequenze ammesse  |
---------------------
| 0 1 1 1 CW  first |
| 0 0 0 1 CW        |
| 1 0 0 0 CW        |
| 1 1 1 0 CW  last  |
|-------------------|
| 1 0 1 1 CCW first |
| 0 0 1 0 CCW       |
| 0 1 0 0 CCW       |
| 1 1 0 1 CCW last  |
---------------------
*/
#define ENCODER_CLK 2
#define ENCODER_DT  3
#define ENCODER_SW  4
#define DEBOUNCE_DELAY 50

int counter = 0;
volatile uint8_t a0 = HIGH;
volatile uint8_t c0 = HIGH;

void setup() {
  Serial.begin(115200);
  // Initialize encoder pins
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
}

void readEncoder() {// ogni change di A
  int a = digitalRead(ENCODER_CLK);
  int b = digitalRead(ENCODER_DT);
  
  if (a != a0) {// rileva transizione di A (contascatti)
    a0 = a;// aggiorna valore passato di A
    if (b != c0) {// rileva transizione di C (b è il nuovo c)
      c0 = b;// aggiorna il vecchio c col nuovo c
      if(b == HIGH){//seleziona le transizioni 1 1 1 0 CW e 1 0 1 1 CCW
        if(a != b){// 1 1 1 0 CW
          counter++;
          Serial.print("CW ⏩ "); Serial.println(counter);
        }else{// 1 0 1 1 CCW
          counter--;
          Serial.print("CCW ⏪ "); Serial.println(counter);
        }
      }
    }
  }
  a0 = a;
}

void resetCounter() {
  counter = 0;
}

void loop() {
  readEncoder();

  if (digitalRead(ENCODER_SW) == LOW) {
    resetCounter();
  }
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/390543783554708481

Di seguito è riportata la variante dello stesso metodo però alimentata da un interrut.

Si noti che adesso l'algoritmo è tutto contenutoin una ISR che è confinante con un fronte che la comincia, per cui non è possibile vedere i valori BABA che stanno a cavallo tra il fronte chel'ha generata. Bisogna individuare altre due sequenze per segnalare la rotazione oraria e antioraria.

La figura si riferisce ad una **rotazione oraria** in cui ad **ogni transizione di A** viene copiato in C il valore corrente di B. Eventuali rimbalzi non modificano il valore di C perchè questo rimane influenzato solo dal valore raggiunto dal primo fronte del treno di rimbalzi di A perchè per i successivi B ha lo stesso valore di C e la condizione falsa sull'if è efficace a filtrarli. In questo caso si può notare che A e B assumono sempre lo **stesso valore** e il pattern corrispondente, **subito prima** di una transizione di A, è 1 0 0 0 CW.

Si potrebbe ricostruire pure la figura relativa ad una **rotazione antioraria** in cui **ogni transizione di A** viene copiato in C il valore corrente di B. Eventuali rimbalzi non modificano il valore di C perchè questo rimane influenzato solo dal valore raggiunto dal primo fronte del treno di rimbalzi di A perchè per i successivi B ha lo stesso valore di C e la condizione falsa sull'if è efficace a filtrarli. In questo caso si può notare che A e B assumono sempre **valori diversi** e il pattern corrispondente, **subito prima** di una transizione di A, è e 1 1 0 1 CCW.

```C++
/*
  ---------------------
  | Sequenze ammesse  |
  ---------------------
  | 0 1 1 1 CW  first |
  | 0 0 0 1 CW        |
  | 1 0 0 0 CW        |
  | 1 1 1 0 CW  last  |
  |-------------------|
  | 1 0 1 1 CCW first |
  | 0 0 1 0 CCW       |
  | 0 1 0 0 CCW       |
  | 1 1 0 1 CCW last  |
  ---------------------
*/
#define ENCODER_CLK 2
#define ENCODER_DT  3
#define ENCODER_SW  4
#define DEBOUNCE_DELAY 50

int counter = 0;
volatile uint8_t a0 = HIGH;
volatile uint8_t c0 = HIGH;

void setup() {
  Serial.begin(115200);
  // Initialize encoder pins
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoder, CHANGE);
}

void readEncoder() {// ogni change di A
  int a = digitalRead(ENCODER_CLK);
  int b = digitalRead(ENCODER_DT);

  if (b != c0) {// rileva transizione di C (b è il nuovo c)
    c0 = b;// aggiorna il vecchio c col nuovo c
    if (b == HIGH) { //seleziona le transizioni 1 1 1 0 CW e 1 0 1 1 CCW
      if (a == b) { // 1 1 1 0 CW
        counter++;
        Serial.print("CW ⏩ "); Serial.println(counter);
      } else { // 1 0 1 1 CCW
        counter--;
        Serial.print("CCW ⏪ "); Serial.println(counter);
      }
    }
  }
}

void resetCounter() {
  counter = 0;
}

void loop() {
  readEncoder();

  if (digitalRead(ENCODER_SW) == LOW) {
    resetCounter();
  }
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/390546530881665025

### **Encoder rotativo tabella e polling metodo array**

Con "transizioni non ammesse" ci riferiamo a quei casi in cui lo stato corrente degli ingressi dell'encoder non corrisponde a nessuna transizione valida nell'array encoderTable. Questo potrebbe accadere a causa di un rumore o di un rimbalzo dell'encoder.

Nel codice fornito, le transizioni non ammesse vengono gestite in modo implicito. Quando si verifica una transizione non ammessa, l'array encoderTable restituisce un valore 0, il che significa che l'encoder non ha cambiato posizione. Di conseguenza, l'encoderValue rimane invariato e non viene stampata alcuna direzione di rotazione.

Ecco un esempio di transizione non ammessa e come viene gestita nel codice:

- Supponiamo che l'ultimo stato dell'encoder sia 0b01 e il nuovo stato sia 0b11. In questo caso, non c'è una transizione valida nell'array ```debounceTable``` che corrisponda a questa sequenza. Quindi, la funzione  ```read_rotary()``` restituirà un valore di incremento pari a 0. Di conseguenza, l'encoderValue non verrà modificato e non verrà stampata alcuna direzione di rotazione.

Questo approccio implicito assume che le transizioni non ammesse siano rare e che l'encoder produca principalmente transizioni valide. Se le transizioni non ammesse diventano un problema significativo, potrebbe essere necessario implementare un meccanismo più sofisticato per gestirle, ad esempio introducendo un conteggio dei tentativi o una logica di correzione degli errori nell'aggiornamento della sequenza ```baba```.

```C++
/*
---------------------
| Sequenze ammesse  |
---------------------
| 0 1 1 1 CW  last  |
| 0 0 0 1 CW        |
| 1 0 0 0 CW        |
| 1 1 1 0 CW  first |
|-------------------|
| 1 0 1 1 CCW last  |
| 0 0 1 0 CCW       |
| 0 1 0 0 CCW       |
| 1 1 0 1 CCW first |
---------------------
*/
#define CLK 2
#define DATA 3
#define BUTTON 4
#define YLED A2
uint8_t baba = 0;
int8_t c,val;

int8_t debounceTable[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

void printBin(byte aByte) {
  for (int8_t aBit = 3; aBit >= 0; aBit--)
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
}

void setup() {
  pinMode(CLK, INPUT);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, INPUT);
  pinMode(DATA, INPUT_PULLUP);
  pinMode(BUTTON, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(YLED,OUTPUT);

  Serial.begin (115200);
  Serial.println("KY-040 Quality test:");
}

void loop() {
   if( val=read_rotary() ) {
      Serial.print("BABA: ");printBin(baba);Serial.println();
      if (baba==0x0b) {// seleziona 1011 (fine scatto)
        c -= val;
        Serial.print(c);Serial.print(" ");
        Serial.println("Vedo undici ⏪");
      }

      if (baba==0x07) {// seleziona 0111  (fine scatto)
        c += val;
        Serial.print(c);Serial.print(" ");
        Serial.println("Vedo sette ⏩");
      }
   }

   if (digitalRead(BUTTON)==0) {

      delay(10);
      if (digitalRead(BUTTON)==0) {
          Serial.println("Next Detent");
          while(digitalRead(BUTTON)==0);
      }
   }
}

// A vald CW or CCW move returns 1, invalid returns 0.
int8_t read_rotary() {
  // Read the current state of the encoder pins
  uint8_t msb = digitalRead(DATA); // B
  uint8_t lsb = digitalRead(CLK); //  A

  // Bitwise operation to construct 2-bit code
  uint8_t ba = (msb << 1) | lsb;

  baba = baba << 2 | ba;
  baba &= 0x0f;
  return (debounceTable[( baba )]);
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/390057874113238017

### **Encoder rotativo tabella e interrupt metodo array**

```C++
/*
---------------------
| Sequenze ammesse  |
---------------------
| 0 1 1 1 CW  last  |
| 0 0 0 1 CW        |
| 1 0 0 0 CW        |
| 1 1 1 0 CW  first |
|-------------------|
| 1 0 1 1 CCW last  |
| 0 0 1 0 CCW       |
| 0 1 0 0 CCW       |
| 1 1 0 1 CCW first |
---------------------
*/
// Define pins for the rotary encoder
#define encoderPinA  2
#define encoderPinB  3

// Variables to store previous state of encoder pins
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
volatile uint8_t baba = 0;

// Array to define the transition states for debouncing
const int8_t debounceTable[16] = {
  0,  1, -1,  0,
 -1,  0,  0,  1,
  1,  0,  0, -1,
  0, -1,  1,  0
};

void printBin(byte aByte) {
  for (int8_t aBit = 3; aBit >= 0; aBit--)
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
}

void setup() {
  // Set encoder pins as inputs
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // Attach interrupts to encoder pins
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);

  // Initialize Serial communication
  Serial.begin(9600);
}

void loop() {
  // Do whatever you need to do with the encoder value
  // For example, you can print it out:
  //Serial.println(encoderValue);
  delay(100); // Adjust delay as needed for your application
}

void updateEncoder() {
  // Read the current state of the encoder pins
  int MSB = digitalRead(encoderPinB);
  int LSB = digitalRead(encoderPinA);

  // Bitwise operation to construct 2-bit code
  int encoded = (MSB << 1) | LSB;

  // Compare the current state with the previous state
  baba = lastEncoded << 2 | encoded;
  int delta = debounceTable[baba];
  if (delta) {
    // Print out direction based on delta value
    if (delta > 0) {
      printBin(baba);Serial.println(" CW ");
      if (baba==0x07) {// seleziona 0111 (fine scatto)
        encoderValue += delta;
        Serial.print("Ho visto sette ⏩ "); Serial.println (encoderValue);
      }
    } else {
      printBin(baba);Serial.println(" CCW ");
      if (baba==0x0b) {// seleziona 1011 (fine scatto)
        encoderValue += delta;
        Serial.print("Ho visto undici ⏪ "); Serial.println (encoderValue);
      }
    }
  }

  // Store current encoded value for the next iteration
  lastEncoded = encoded;
}
```

Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/389999046461201409

### **Encoder rotativo tabella e polling metodo array migliorato**

La **miglioria** consiste nel potenziare la **robustezza agli errori** dell'algoritmo mettendolo in grado di discriminare più efficacemente le **sequenze non ammesse** dall'encoder. In corrispondenza di queste l'encoder **non fa nulla** e non sposta nessun conteggio in quanto ciascuna è considerata dall'encoder come un **valore spurio**, cioè non voluto perchè causato ad una qualche **sorgente di rumore**. Tipiche sorgenti di rumore sono i **rimbalzi** dell'interruttore o eventuali **disturbi elettromagnetici** (RFI).

Le **sequenze BABA** di 4 bit sono normalmente la base su cui si testa la **correttezza** delle sequenze **misurate**. Man mano che si ricevono i bit nel loop, se questi non sono affetti da errori, viene ricevuta una sequenza corretta ogni 4 bit. La sequenza **ricevuta prima** è quella della transizione a ridosso dello scatto. Quindi se si vuole un riconoscimento più tempestivo possibile della **accelerazione** di uno scatto bisogna accontentarsi soltanto di questa. Se si aspettano altri 4 bit se ne riceve completamente un'altra che, unita a quella precedente, fornisce una base di test statisticamente **più affidabile** (benchè meno affidabile nei cambi di velocità).

Si può riceverne una terza migliorando ancora l'affidabilità (a scapito della misura dell'accelerazione), fino a ricevere tutti i 16 bit che ospitano tutte le 4 sequenze che sono ammissibili a seguito di uno scatto. In quest'ultino caso l'**affidabilità** sull'interpretazione di uno scatto, statisticamente è massima ma è ottenuta al prezzo della **rilevazione ritardata** degli scatti successivi a quello sotto test. 

In definitiva, con una parola di 16 bit che contiene tutte le sequenze ammissibili di uno scatto, si rileva uno scatto ogni 4. Con una parola di 8 bit uno scatto ogni 2. Con una parola di 4 bit, corrispondente alla sequenza BABA contente i 2 bit attuali delle porte più i 2 della misura passata, si ottiene la misura tempestiva, in velocità e accelerazione, di tutti gli scatti.

Una sequenza di 16 bit **non ritarda granchè** nella misura del numero di scatti rispetto ad una ad 8 o 4 a velocità costante (in pratica nessun ritardo). Invece, forse, potrebbe ritardare la lettura delle **accelerazioni** (decelerazioni) degli scatti che, con sequenze lunghe, magari sono campionate meno velocemente. 

In generale, è sempre consigliabile condurre test empirici per valutare l'effetto di qualsiasi modifica, come l'introduzione di sequenze più lunghe, sull'accuratezza complessiva del sistema di misurazione dell'accelerazione.

Si noti la necessità di **due misure** per la rilevazione di uno scatto, una corrispondente alla **BA attuale** e una a quella passata. Sono il minimo necessario per la rilevazione di una transizione che, fisicamente, corrisponde alla discontinuità alto-basso del dente dell'ingranaggio che è, in quell'istante, sotto misura. 

Con i **primi due bit** si rileva il fronte di una rotazione, con gli ultimi due il **verso** della stessa. La distanza temporale tra due fronti permette la misura della velocità. Per quanto riguarda la rilevazione del fronte, basterebbe il segnale CK (A) e quindi complessivamente basterebbe una sequenza ABA per determinare completamente rotazione e verso, difatti questa è stata la soluzione adottata nei primi algoritmi proposti. Nelle altre soluzioni, il campione B della sequenza BABA fornisce una informazione di ridondanza utile a scartare sequenze affette da rimbalzi o disturbi.

In ogni caso, esiste un limite inferiore alla risoluzione temporale del dispositivo data dal contributo di molti parametri fisici diversi. Uno dei più limitanti è sicuramente il fenomeno dei **rimbalzi** che normalmente hanno una durata che, pur essendo tipica per una certo modello di encoder, può in ogni caso variare da un pezzo all'altro principalmente a causa del suo stato di **usura**. L'usura, in particolare fa si che il tempo medio dei rimbalzi si allunghi fino al punto da rendere il dispositivo **inutilizzabile**.

I debouncer basati sui timer hanno la necessità di essere impostati in fabbrica, scegliendo possibilmente un valore di sicurezza adatto anche per un dispositivo discretamente usurato. A prescindere da altre differenze individuali, il limite inferiore delle prestazioni di velocità dell'encoder è adesso fissato una volta per tutte dal timer.

Nel caso dei debouncer basati sul filtro delle transizioni, il limite è invece automaticamente tarato sulla reazione dell'algoritmo alle sequenze sbagliate che verranno scartate, lasciando immutata l'ultimo valore valido del conteggio, non in base ad un ritardo teorico ma esattamente in base al limite fisico di quel particolare dispositivo.
Il vantaggio, in questo caso, è la possibilità di tarare la granularità della risposta sul **livello corrente** delle prestazioni di un encoder, il cui limite superiore è determinabile automaticamente alla ricezione di sequenze non ammissibili (che bloccano il conteggio). Se è nuovo e di buona qualità saranno ottime, se è scadente e molto usurato saranno notevolmente peggiori.


```C++

/*
---------------------
| Sequenze ammesse  |
---------------------
| 0 1 1 1 CW  last  |
| 0 0 0 1 CW        |
| 1 0 0 0 CW        |
| 1 1 1 0 CW  first |
|-------------------|
| 1 0 1 1 CCW last  |
| 0 0 1 0 CCW       |
| 0 1 0 0 CCW       |
| 1 1 0 1 CCW first |
---------------------
*/
#define CLK 2
#define DATA 3
uint8_t baba = 0;
uint16_t store=0;
int8_t c,val;
int8_t debounceTable[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

void printBin(uint16_t aByte) {
  for (int8_t aBit = 15; aBit >= 0; aBit--)
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
}

void setup() {
  pinMode(CLK, INPUT);
  pinMode(CLK, INPUT);
  pinMode(DATA, INPUT);
  pinMode(DATA, INPUT);
  Serial.begin (115200);
  Serial.println("KY-040 Start:");
}

void loop() {
//static int8_t c,val;

    if( val=read_rotary() ) {
       c += val;
      if (val == -1) {// seleziona 1011 (fine scatto)
        Serial.print(c);Serial.print(" ");
        Serial.println("Vedo 54315 ⏪");
      }
      if (val == 1) {// seleziona 0111  (fine scatto)
        Serial.print(c);Serial.print(" ");
        Serial.println("Vedo 59415 ⏩");
      }
   }
 }

// A vald CW or  CCW move returns 1, invalid returns 0.
int8_t read_rotary() {
  // Read the current state of the encoder pins
  uint8_t msb = digitalRead(DATA); // B
  uint8_t lsb = digitalRead(CLK); //  A

  // Bitwise operation to construct 2-bit code
  uint8_t ba = (msb << 1) | lsb;

  baba = baba << 2 | ba;
  baba &= 0x0f;

  // If valid then store as 16 bit data.
  if (debounceTable[baba]) {
    store <<= 4;        // shift dell'ultimo valore in coda
    store |= baba;      // inserimento in coda di BABA
    
    Serial.print("BAx8: ");printBin(store); Serial.print(" DEC: ");Serial.println(store);
    if (store==0xd42b) return -1;// 1101 0100 0010 1011 (54315) controllo sequenza su 16 bit
    if (store==0xe817) return 1;// 1110 1000 0001 0111 (59415) controllo sequenza su 16 bit
    //if ((store&0xff)==0x2b) return -1;// 00101011 controllo sequenza su 8 bit
    //if ((store&0xff)==0x17) return 1;//  00010111 controllo sequenza su 8 bit
  }
  return 0;
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/390014530229883905

Sitografia:
- https://docs.wokwi.com/parts/wokwi-ky-040
- https://www.lombardoandrea.com/utilizzare-un-encoder-rotativo-con-arduino/
- https://www.pinteric.com/rotary.html
- http://www.technoblogy.com/show?1YHJ
- https://github.com/fryktoria/FR_RotaryEncoder/blob/master/src/FR_RotaryEncoder.cpp
- https://it.wikipedia.org/wiki/Codice_Gray
- https://web.engr.oregonstate.edu/~traylor/ece473/student_projects/ReadingEncoderSwitches.pdf
- https://www.best-microcontroller-projects.com/support-files/pec11l.pdf
- https://www.best-microcontroller-projects.com/rotary-encoder.html
- https://wokwi.com/projects/390078908878152705
- https://wokwi.com/projects/390079895422569473
- https://en.wikipedia.org/wiki/Incremental_encoder
- https://wokwi.com/projects/390367241693177857

>[Torna all'indice](indexpulsanti.md)

