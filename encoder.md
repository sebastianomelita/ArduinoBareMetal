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

L'encoder rotativo offre due **modalità di interazione**:
- **Rotazione**: è possibile ruotare la manopola facendo clic sulle frecce. La freccia superiore lo ruota di un passo in senso orario, mentre la freccia inferiore lo ruota di un passo in senso antiorario. La rotazione della manopola produrrà segnali digitali sui pin DT e CLK, come spiegato di seguito.
- **Pulsante**: fare clic sulla manopola per premere il pulsante. Mentre è premuto, il pulsante collega il pin SW con il pin GND.
Ogni volta che l'utente ruota la manopola, viene prodotto un serie di segnali sui pin DT e CLK.

<img src="img\tansitions.jpg" alt="alt text" width="1000">

La **fisica interna** del dispositivo fa si che, per stabilire il numero di scatti e il verso di rotazione dall'osservazione dei due ingressi A e B, bisogna esaminare **almeno due gruppi** di valori consecutivi al fine di isolare il **fronte** di un piedino e il **valore** di un altro.

Alla luce di ciò, la valutazione di  numero di **scatti** e **verso** delle rotazioni si può ottenere essenzialmente con **tre tecniche**:
- Attendere il verificarsi di una **transizione pilota** su di un piedino (ad esempio A) che indica lo **scatto** e valutare, in corrispondenza di questa, quale valore assume l'altro piedino (in questo caso B) al fine di stabilire il **verso**, cioè il valore che identifica una rotazione oraria (CW) o quello che identifica quella antioraria (CCW). Questa modalità, da sola senza null'altro, non è in grado di filtrare le transizioni spurie.
- **Tabella delle transizioni**. Leggere, grazie a 2 due loop consecutivi, una sequenza BABA, e decidere se accettarla o considerarla una **sequenza scorretta** (spuria). Una volta accettata, è possibile usare una sequenza per stabilire se la rotazione ha il verso CW o quello CCW. La geometria interna del **dispositivo** permette solamente 4 **sequenze ammissibili** di tipo BABA e si può, a questo punto, stabilirne la politica di gestione: controllare se ne deve arrivare corretta una in particolare, se ne devono arrivare alcune o se devono arrivare corrette proprio tutte prima di convalidare il risultato. E' possibile anche decidere di accorpare più di 4 transizioni, ad esempio 8 transizioni BABABABA, per ridurre ulteriormente la probabilità di errore nel condurre l'analisi delle sequenze corrette .
   
- **Macchina a stati finiti**. E' una tecnica che per adesso non tratteremo.

<img src="img\wokwi-ky-040-timing-cd6fe446378352c1416ef8817f1e5abf.webp" alt="alt text" width="800">

Per cominciare, ed essere subito operativi, usiamo la tecnica della **transizione pilota**, e tra tutte scegliamo la transizione in FALLING del piedino A per selezionare la **transizione** che indica lo **scatto** e il valore del piedino B per stabilire il **verso** della rotazione:
- La rotazione in **senso orario** fa sì che il pin **CLK** si abbassi **prima**, quindi anche il pin DT si abbassi.
- La rotazione in **senso antiorario** fa sì che il pin **DT** si abbassi **prima**, quindi il pin CLK si abbassi.

## **TECNICA DELLA TRANSIZIONE PILOTA**

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

### **Encoder rotativo mediante interrupt con debouncer non basato sul tempo**


Tratto da http://www.technoblogy.com/show?1YHJ

<img src="img\staticdebounce.png" alt="alt text" width="500">

```C++
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define ENCODER_CLK 2
#define ENCODER_DT  3
#define ENCODER_SW  4
#define DEBOUNCE_DELAY 50

int counter = 0;
int count = 0;
volatile uint8_t a0 = HIGH;
volatile uint8_t c0 = HIGH;

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize encoder pins
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoder, CHANGE);
}

// Called when encoder value changes
int changeValue (bool up) {
  int val = 0;
  //counter = max(min((counter + (up ? 1 : -1)), 1000), 0);
  //counter = min((counter + (up ? 1 : -1)), 1000);
  if(count >= 1){
    count = 0;
    val = (up ? 1 : -1);
  }else{
    count++;
  }
  return val;
}

void readEncoder() {// ogni change di A
  int a = digitalRead(ENCODER_CLK);
  int b = digitalRead(ENCODER_DT);
  
  //if (a != a0) {              // A changed
    //a0 = a;
    if (b != c0) {// genera falling di c0
      c0 = b;
      counter += changeValue(a != b);
    }
  //}
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
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/390109278439107585

## **TECNICA DELLA TABELLA DI TRANSIZIONE**

<img src="img\babatable.png" alt="alt text" width="1400">

<img src="img\tansitions.jpg" alt="alt text" width="1000">

### **Encoder rotativo con tabella e polling metodo tracking**

```C++
/*
B A B A Direction
0 0 0 0 NA
0 0 0 1 CW
0 0 1 0 CCW
0 0 1 1 NA
0 1 0 0 CCW
0 1 0 1 NA
0 1 1 0 NA
0 1 1 1 CW
1 0 0 0 CW
1 0 0 1 NA
1 0 1 0 NA
1 0 1 1 CCW
1 1 0 0 NA
1 1 0 1 CCW
1 1 1 0 CW
1 1 1 1 NA
*/
#define CW 1
#define CCW 2
#define ENCODER_CLK 2
#define ENCODER_DT  3
int a_past = LOW;
int b_past = LOW;
int a0_past = HIGH;
int direction = 0;
int counter = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
}

void loop() {
  int a_current = digitalRead(ENCODER_CLK); // polling di CK/A attuale
  int b_current = digitalRead(ENCODER_DT);  // polling di DT/B attuale
  direction = 0;
  /*
  B A B A
  0 0 0 0 NA
  0 0 1 1 NA
  0 1 0 1 NA
  0 1 1 0 NA
  1 0 0 1 NA
  1 0 1 0 NA
  1 1 0 0 NA
  1 1 1 1 NA
  */
  if(a_past == a_current){
    //Serial.println("Apast = Acurrent");
    if((a_current == 1) && (b_past < b_current)){direction = CW;counter++;Serial.print("0 1 1 1 CW ⏩ ");Serial.println(counter);}//   0 1 1 1 CW  fine scatto              
    if((a_current == 1) && (b_past > b_current)){direction = CCW;Serial.println("1 1 0 1 CCW ");}// 1 1 0 1 CCW
    if((a_current == 0) && (b_past > b_current)){direction = CW;Serial.println("1 0 0 0 CW ");}//   1 0 0 0 CW                   
    if((a_current == 0) && (b_past < b_current)){direction = CCW;Serial.println("0 0 1 0 CCW ");}// 0 0 1 0 CCW
  }
  if((a_past < a_current) && (b_past == LOW && b_current == LOW)){direction = CW;Serial.println("0 0 0 1 CW ");}//     0 0 0 1 CW  
  if((a_past < a_current) && (b_past == HIGH && b_current == HIGH)){direction = CCW;counter--;Serial.print("1 0 1 1 CCW ⏪ ");Serial.println(counter);}// 1 0 1 1 CCW fine scatto
  if((a_past > a_current) && (b_past == LOW && b_current == LOW)){direction = CCW;Serial.println("0 1 0 0 CCW ");}//   0 1 0 0 CCW
  if((a_past > a_current) && (b_past == HIGH && b_current == HIGH)){direction = CW;Serial.println("1 1 1 0 CW ");}//   1 1 1 0 CW   
  

  // increment alarm count
  // test for over/under flows
  a_past = a_current;
  b_past = b_current;
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/389979138658609153

### **Encoder rotativo tabella e interrupt metodo tracking**

```C++
// Define pins for the rotary encoder
#define encoderPinA  2
#define encoderPinB  3
#define CW 1
#define CCW 2

int a_past = HIGH;
int b_past = HIGH;
int direction = 0;
volatile short counter = 0;

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
  //Serial.println(counter);
  delay(100); // Adjust delay as needed for your application
}

void updateEncoder() {
  int a_current = digitalRead(encoderPinA); // polling di CK/A attuale
  int b_current = digitalRead(encoderPinB);  // polling di DT/B attuale
  //if (a_past == a_current) { // selezione di un fronte di discesa di CK/A 
  //int b_current = digitalRead(ENCODER_DT);  // polling di DT/B attuale
  direction = 0;
  /*
  B A B A
  0 0 0 0 NA
  0 0 1 1 NA
  0 1 0 1 NA
  0 1 1 0 NA
  1 0 0 1 NA
  1 0 1 0 NA
  1 1 0 0 NA
  1 1 1 1 NA
  */
  if(a_past == a_current){
    //Serial.println("Apast = Acurrent");
    if((a_current == 1) && (b_past < b_current)){direction = CW;counter++;Serial.print("0 1 1 1 CW ⏩ ");Serial.println(counter);}//   0 1 1 1 CW  fine scatto              
    if((a_current == 1) && (b_past > b_current)){direction = CCW;Serial.println("1 1 0 1 CCW ");}// 1 1 0 1 CCW
    if((a_current == 0) && (b_past > b_current)){direction = CW;Serial.println("1 0 0 0 CW ");}//   1 0 0 0 CW                   
    if((a_current == 0) && (b_past < b_current)){direction = CCW;Serial.println("0 0 1 0 CCW ");}// 0 0 1 0 CCW
  }
  if((a_past < a_current) && (b_past == LOW && b_current == LOW)){direction = CW;Serial.println("0 0 0 1 CW ");}//     0 0 0 1 CW  
  if((a_past < a_current) && (b_past == HIGH && b_current == HIGH)){direction = CCW;counter--;Serial.print("1 0 1 1 CCW ⏪ ");Serial.println(counter);}// 1 0 1 1 CCW fine scatto
  if((a_past > a_current) && (b_past == LOW && b_current == LOW)){direction = CCW;Serial.println("0 1 0 0 CCW ");}//   0 1 0 0 CCW
  if((a_past > a_current) && (b_past == HIGH && b_current == HIGH)){direction = CW;Serial.println("1 1 1 0 CW ");}//   1 1 1 0 CW   
  
  // increment alarm count
  // test for over/under flows
  a_past = a_current;
  b_past = b_current;
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/390001300410019841

### **Encoder rotativo tabella e polling metodo array**

Con "transizioni non ammesse" ci riferiamo a quei casi in cui lo stato corrente degli ingressi dell'encoder non corrisponde a nessuna transizione valida nell'array encoderTable. Questo potrebbe accadere a causa di un rumore o di un rimbalzo dell'encoder.

Nel codice fornito, le transizioni non ammesse vengono gestite in modo implicito. Quando si verifica una transizione non ammessa, l'array encoderTable restituisce un valore 0, il che significa che l'encoder non ha cambiato posizione. Di conseguenza, l'encoderValue rimane invariato e non viene stampata alcuna direzione di rotazione.

Ecco un esempio di transizione non ammessa e come viene gestita nel codice:

- Supponiamo che l'ultimo stato dell'encoder sia 0b01 e il nuovo stato sia 0b11. In questo caso, non c'è una transizione valida nell'array ```rot_enc_table``` che corrisponda a questa sequenza. Quindi, la funzione  ```read_rotary()``` restituirà un valore di incremento pari a 0. Di conseguenza, l'encoderValue non verrà modificato e non verrà stampata alcuna direzione di rotazione.

Questo approccio implicito assume che le transizioni non ammesse siano rare e che l'encoder produca principalmente transizioni valide. Se le transizioni non ammesse diventano un problema significativo, potrebbe essere necessario implementare un meccanismo più sofisticato per gestirle, ad esempio introducendo un conteggio dei tentativi o una logica di correzione degli errori nell'aggiornamento dell'encoderValue.

```C++
#define CLK 2
#define DATA 3
#define BUTTON 4
#define YLED A2
volatile long encoderValue = 0;

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

uint8_t baba = 0;
int8_t c,val;

void loop() {
uint32_t pwas=0;

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
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};
  baba <<= 2;
  if (digitalRead(DATA)) baba |= 0x02;
  if (digitalRead(CLK)) baba |= 0x01;
  baba &= 0x0f;
  return ( rot_enc_table[( baba & 0x0f )]);
}
```
Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/390057874113238017

### **Encoder rotativo tabella e interrupt metodo array**

```C++
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
      if (baba==0x07) {// seleziona 1011 (fine scatto)
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

La miglioria consiste nel potenziare la capacità di debouncing dell'algoritmo mettendolo in grado di discriminare più efficacemente le sequenze non ammesse dell'encoder in quanto ciascuna è considerata una traccia con cui è possibile riconoscere gli effetti sia dei rimbalzi dell'interruttore che di eventuali disturbi elettromagnetici (RFI).

Le sequenze BABA di 4 bit sono normalmente la base su cui si testano le sequenze misurate. Man mano che si ricevono i bit nel loop, se questi non sono affetti da errori, viene ricevuta una sequenza corretta ogni 4 bit. La sequenza ricevuta prima è quella della transizione a ridosso dello scatto. Quindi se si vuole un riconoscimento più tempestivo possibile dello scatto bisogna accontentarsi soltanto di questa. Se si aspettano altri 4 bit se ne riceve completamente un'altra che, unita a quella precedente, fornisce una base di test più affidabile (benchè più lenta).

Si può riceverne una terza migliorando ancora l'affidabilità (a scapito della velocità), fino a ricevere tutti i 16 bit che ospitano le 4 sequenze ammissibili a seguito di uno scatto. In quest'ultino caso l'affidabilità sull'interpretazione di uno scatto, statisticamente è massima ma è ottenuta al prezzo della rilevazione ritardata dei 3 scatti successivi a quello sotto test. 

In definitiva, con una parola di 16 bit che contiene tutte le sequenze ammissibili di uno scatto, si rileva uno scatto ogni 4. Con una parola di 8 bit uno scatto ogni 2. Con una parola di 4 bit, corrispondente alla sequenza BABA contente i 2 bit attuali delle porte più i 2 della misura passata, si ottiene la misura tempestiva di tutti gli scatti.

Si noti la necessità di **due misure** per la rilevazione di uno scatto, una corrispondente alla BA attuale e una a quella passata. Sono il minimo necessario per la rilevazione di una transizione che, fisicamente, corrisponde alla discontinuità alto-basso del dente dell'ingranaggio che è, in quell'istante, sotto misura.

In ogni caso, esiste un limite inferiore alla risoluzione temporale del dispositivo data dal contributo di molti parametri fisici diversi. Uno dei più limitanti è sicuramente il fenomeno dei **rimbalzi** che normalmente hanno una durata che è tipica per una certo modello di encoder ma può nel contempo variare a seconda della bontà della manifattura del singolo pezzo e, soprattutto, a seconda del suo stato di usura. L'usura, in particolare fa si che il tempo medio del fenomeno si allunghi fino al punto da rendere il dispositivo inutilizzabile.

I debouncer basati sui timer hanno la necessità di essere impostati una volta per sempre sul valore medio di un dispositivo discretamente usurato. A prescindere da altre differenze individuali, il limite inferiore delle prestazioni di velocità dell'encoder è adesso fissato una volta per tutte dal timer.

Nel caso dei debouncer basati sul filtro delle transizioni il limite è adesso tarato dalla reazione dell'algoritmo alle sequenze errate che verranno scartate lasciando immutata l'ultima impostazione valida misurata (il dispositivo reagisce con incremento nullo).

Il vantaggio, in questo caso, è la possibilità di tarare la granularità della risposta sullo stato corrente delle prestazioni di un encoder. Se è nuovo e di buona qualità saranno ottime, se è scadente e molto usurato saranno notevolmente peggiori.


```C++
// Robust Rotary encoder reading
//
// Copyright John Main - best-microcontroller-projects.com
//
#define CLK 3
#define DATA 2

uint8_t baba = 0;
uint16_t store=0;
int8_t c,val;


void printBin(uint16_t aByte) {
  for (int8_t aBit = 15; aBit >= 0; aBit--)
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
}

void setup() {
  pinMode(CLK, INPUT);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, INPUT);
  pinMode(DATA, INPUT_PULLUP);
  Serial.begin (115200);
  Serial.println("KY-040 Start:");
}

void loop() {
//static int8_t c,val;

    if( val=read_rotary() ) {
       c += val;
      if (val == -1) {// seleziona 1011 (fine scatto)
        Serial.print(c);Serial.print(" ");
        Serial.println("Vedo 59415 ⏪");
      }
      if (val == 1) {// seleziona 0111  (fine scatto)
        Serial.print(c);Serial.print(" ");
        Serial.println("Vedo 54315 ⏩");
      }
   }
 }

// A vald CW or  CCW move returns 1, invalid returns 0.
int8_t read_rotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  baba <<= 2;
  if (digitalRead(DATA)) baba |= 0x02;
  if (digitalRead(CLK)) baba |= 0x01;
  baba &= 0x0f;

   // If valid then store as 16 bit data.
   if  (rot_enc_table[baba] ) {
      store <<= 4;        // shift dell'ultimo valore in coda
      store |= baba;      // inserimento in coda di BABA
      
      Serial.print("BAx8: ");printBin(store); Serial.print(" DEC: ");Serial.println(store);
      if (store==0xd42b) return 1;// 1101 0100 0010 1011 (54315) controllo sequenza su 16 bit
      if (store==0xe817) return -1;// 1110 1000 0001 0111 (59415) controllo sequenza su 16 bit
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

>[Torna all'indice](indexpulsanti.md)

