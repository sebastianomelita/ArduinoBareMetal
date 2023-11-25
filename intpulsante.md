>[Torna all'indice](indexinterrupts.md)
## **PULSANTE CON INTERRUPT**

Gli ingressi rilevati **tramite un interrupt** vengono sicuramente serviti in maniera **più rapida** rispetto ad altre soluzioni (delay, polling della millis(), thread), questo perchè l'arrivo di un segnale di interrupt blocca immediatamente l'esecuzione di un task direttamente sull'**istruzione che è attualmente in esecuzione** al momento dell'arrivo dell'interrupt. Mantiene la complessità (tutto sommato non elevata) di una normale chiamata di funzione ed è attivato a ridosso di un qualunque **evento asincrono**, del quale, per definizione, non è mai possibile prevedere in anticipo l'accadere. 

**In definitiva**, tra le tecniche più responsive, quella dell'interrupt è certamente la **più veloce** e quindi non solo garantisce una adeguata responsività ai comandi ma anche la massima velocità di risposta possibile che è un prerequisito necessario per il comando efficace dei **dispositivi di sicurezza**, cioè di quei dispositivi critici deputati alla protezione di beni e persone da danni irreparabili.

La **gestione** di un **pulsante** mediante gli **interrupts** passa sostanzialmente per **due fasi**:
- l'**attivazione** asincrona e immediata su un fronte del valore della prota digitale di ingresso (di salita o di discesa)
- **l'impostazione** del pulsante per una nuova attivazione (riarmo) che non è immediata, perchè, prima di consentire nuove pressioni, va speso del **tempo** per lasciare **estinguere le oscillazioni elettriche** elettrici causati dai **rimbalzi meccanici** del pulsante.

Le **tecniche individuate** nella presente dispensa sono sostanzialmente **le stesse** per quanto riguarda la prima fase di attivazione mentre **differiscono** nel modo con cui viene realizzato il **ritardo** necessario per il **debouncing**.

Per una discussione più completa sugli interrupt vedi [interrupt](interruptsbase.md).

### **PULSANTE DI SICUREZZA CON DEBOUNCER BASATO SU TIMER SW (POLLING)**

Il codice precedente, per quanto **molto reponsivo**, non è adatto a realizzare un **blocco di sicurezza** per via del **ritardo** nell'intervento di attivazione e disattivazione dell'uscita causato dalll'algoritmo di **debouncing** (antirimbalzo). Per adattarlo a quest'ultimo scopo, il codice va modificato in modo da avere un intervento **immediato** su uno dei fronti (quello che comanda lo sblocco dell'alimentazione) ed uno ritardato (per realizzare il debouncing) sull'altro (quello che comanda il riarmo). 

Il **ritardo** per il debouncing è realizzato senza delay() utilizzando un timer SW basato sul **polling** della funzione millis() nel loop principale. Il polling è un'operazione non bloccante e quindi non interferisce con nessun task, all'interno del loop dove essa viene eseguita.

```C++
const unsigned long DEBOUNCETIME = 50;
const byte ENGINE = 13;
const byte BUTTONPIN = 12;
volatile unsigned long previousMillis = 0;
volatile unsigned short numberOfButtonInterrupts = 0;
bool pressed;

// Interrupt Service Routine (ISR)
void stopEngine ()
{
  numberOfButtonInterrupts++; // contatore rimbalzi
  byte val = digitalRead(BUTTONPIN); // lettura stato pulsante
  previousMillis = millis(); // tempo evento
  if(val==HIGH){ // fronte di salita
    	pressed = true; // disarmo il pulsante
  	digitalWrite(ENGINE, LOW); // blocco subito il motore
  }
}  

void waitUntilInputLOW()
{
   // sezione critica
   // protegge previousMillis che, essendo a 16it, potrebbe essere danneggiata se interrotta da un interrupt
   // numberOfButtonInterrupts è 8bit e non è danneggiabile ne in lettura ne in scrittura
   noInterrupts();
   // il valore lastintTime potrà essere in seguito letto interrotto ma non danneggiato
   unsigned long lastintTime = previousMillis;
   interrupts();
   
   if ((numberOfButtonInterrupts != 0) //flag interrupt! Rimbalzo o valore sicuro? 
      && (millis() - lastintTime > DEBOUNCETIME )//se è passato il transitorio 
      && digitalRead(BUTTONPIN) == LOW)//se il pulsante è ancora premuto
    { 
      Serial.print("HIT: "); Serial.print(numberOfButtonInterrupts);
      numberOfButtonInterrupts = 0; // reset del flag
      Serial.println(" in DISCESA riarmo pulsante");
      pressed = false; // riarmo il pulsante
      digitalWrite(ENGINE, HIGH); // riattivo il motore
    }
}

void setup ()
{
  Serial.begin(115200);
  pinMode(ENGINE, OUTPUT);  	  // so we can update the LED
  digitalWrite(ENGINE, HIGH);
  digitalWrite(BUTTONPIN, LOW); 
  // attach interrupt handler
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), stopEngine, CHANGE);  
  numberOfButtonInterrupts = 0;
  pressed = false;
}  // end of setup

void loop ()
{
   waitUntilInputLOW();
   delay(10);
}
```

Un debouncer come questo, basato su timer SW, potrebbe essere **preferibile** alla controparte HW perchè non impegna affatto una risorsa così preziosa come un timer HW, soprattutto quando non ne esiste sul sistema una sua **versione logica** anch'essa **basata su interrupt** (ad esempio quelli forniti dalla **libreria Ticker**).

Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/jWfXle1Us2E?editbtn=1

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/348783670266430034

### **PULSANTE DI SICUREZZA CON DEBOUNCER BASATO SUI DELAY**

Il codice precedente, per quanto **molto reponsivo**, non è adatto a realizzare un **blocco di sicurezza** per via del **ritardo** nell'intervento di attivazione e disattivazione dell'uscita causato dalll'algoritmo di **debouncing** (antirimbalzo). Per adattarlo a quest'ultimo scopo, il codice va modificato in modo da avere un intervento **immediato** su uno dei fronti (quello che comanda lo sblocco dell'alimentazione) ed uno ritardato (per realizzare il debouncing) sull'altro (quello che comanda il riarmo). 

Il **ritardo** per il debouncing è realizzato con la ```waitUntilInputLow()``` nel loop principale, che utilizza internamente i ```delay()```. Il ritardo, utilizzando i ```delay()```, è un'operazione bloccante e quindi potenzialmente potrebbe interferire negativamente con tutti i task, all'interno del loop che hanno bisogno di essere eseguiti in parallelo al task del debouncer.

```C++
#include "urutils.h"
const unsigned long DEBOUNCETIME = 50;
const byte ENGINE = 13;
const byte safetystop = 12;
volatile unsigned short numberOfButtonInterrupts = 0;
bool pressed;

// Interrupt Service Routine (ISR)
void switchPressed ()
{
  numberOfButtonInterrupts++; // contatore rimbalzi
  byte val = digitalRead(safetystop); // lettura stato pulsante
  if(val==HIGH){ // fronte di salita
    pressed = true; // disarmo il pulsante
  	digitalWrite(ENGINE, LOW); // blocco subito il motore
  }
}  

bool waitUntilInterruptLOW(int pin)
{   
  if (pressed)//se il pulsante è ancora premuto
  { 
    waitUntilInputLow(pin, 50);
    pressed = false; // riarmo il pulsante
    Serial.print("HIT: "); Serial.print(numberOfButtonInterrupts);
    numberOfButtonInterrupts = 0; // reset del flag
    Serial.println(" in DISCESA riarmo pulsante");
    return true;
  }
  return false;
}

void setup ()
{
  Serial.begin(115200);
  pinMode(ENGINE, OUTPUT);  	  // so we can update the LED
  digitalWrite(ENGINE, HIGH);
  digitalWrite(safetystop, LOW); 
  // attach interrupt handler
  attachInterrupt(digitalPinToInterrupt(safetystop), switchPressed, CHANGE);  
  numberOfButtonInterrupts = 0;
  pressed = false;
}  // end of setup

void loop ()
{
  if(waitUntilInterruptLOW(safetystop)){
	digitalWrite(ENGINE, HIGH); // riattivo il motore
  }
  delay(10);
}
```
Simulazione su Esp32 con Wowki: https://wokwi.com/projects/382390727185717249

### **PULSANTE DI SICUREZZA CON DEBOUNCER BASATO SUI DELAY IN UN THREAD A PARTE**

Il codice precedente, per quanto **molto reponsivo**, non è adatto a realizzare un **blocco di sicurezza** per via del **ritardo** nell'intervento di attivazione e disattivazione dell'uscita causato dalll'algoritmo di **debouncing** (antirimbalzo). Per adattarlo a quest'ultimo scopo, il codice va modificato in modo da avere un intervento **immediato** su uno dei fronti (quello che comanda lo sblocco dell'alimentazione) ed uno ritardato (per realizzare il debouncing) sull'altro (quello che comanda il riarmo). 

Il **ritardo** per il debouncing è realizzato con la ```waitUntilInputLow()``` ma stavolta non nel loop principale. Il ritardo, utilizza i ```delay()```,in un thread separato che non può interferire con altri task.

```C++
#include "urutils.h"
#include <pthread.h> //libreria di tipo preemptive
const unsigned long DEBOUNCETIME = 50;
const byte ENGINE = 13;
const byte safetystop = 12;
volatile unsigned short numberOfButtonInterrupts = 0;
bool volatile pressed;
pthread_t t_debounce;

// Interrupt Service Routine (ISR)
void switchPressed ()
{
  numberOfButtonInterrupts++; // contatore rimbalzi
  byte val = digitalRead(safetystop); // lettura stato pulsante
  if(val==HIGH){ // fronte di salita
    pressed = true; // disarmo il pulsante
    digitalWrite(ENGINE, LOW); // blocco subito il motore
  }
}  
void * taskDebounce(void *)
{
  while(true){    
    if (pressed)//se il pulsante è ancora premuto
    { 
      waitUntilInputLow(safetystop, 50);
      pressed = false; // riarmo il pulsante
      Serial.print("HIT: "); Serial.print(numberOfButtonInterrupts);
      numberOfButtonInterrupts = 0; // reset del flag
      Serial.println(" in DISCESA riarmo pulsante");
    }
  }
}

void setup ()
{
  Serial.begin(115200);
  pinMode(ENGINE, OUTPUT);  	  // so we can update the LED
  digitalWrite(ENGINE, HIGH);
  digitalWrite(safetystop, LOW); 
  // attach interrupt handler
  attachInterrupt(digitalPinToInterrupt(safetystop), switchPressed, CHANGE);  
  numberOfButtonInterrupts = 0;
  pressed = false;
  pthread_create(&t_debounce, NULL, taskDebounce, NULL);
}  // end of setup

void loop ()
{
  if(!pressed){// riavvio, senza fretta, il motore nel loop()
	digitalWrite(ENGINE, HIGH); // riattivo il motore
  }
  delay(10);
}
```
Simulazione su Esp32 con Wowki: https://wokwi.com/projects/382393152775960577

### **PULSANTE TOGGLE CON DEBOUNCER BASATO SU TIMER HW**

All'**ingresso** di una **porta digitale**, per ottenere la rilevazione **sicura** (senza rimbalzi) ma anche **tempestiva** (più rapida possibile) del solo **fronte di salita** è stata usata la **combinazione** di due tecniche di schedulazione:
- una **asincrona** (una ISR), non governata dal sistema, ma da un segnale di **interrupt** in ingresso proveniente dall'**esterno**, per la determinazione istantanea (o quasi) del suo fronte di salita per poter elaborare la risposta il più vicino possibile all'evento che la ha causata.
- una **sincrona** (una callback), gestita dal sistema tramite un **timer HW**, per la realizzazione della funzione di debouncing (antirimbalzo) del segnale in ingresso.

Il **rilevatore dei fronti** è realizzato **campionando** il valore del livello al momento dell'arrivo del segnale di interrupt e **confrontandolo** con il valore del livello campionato in istanti **periodici** successivi a quello, pianificati (schedulati) tramite un timer HW, allo scadere del quale viene chiamata l'istruzione ```waitUntilInputLow()```. La funzione, di fatto, esegue una **disabilitazione** della rilevazione dei fronti (per evitare letture duplicate dello stesso) in **"attesa"** che si verifichi una certa **condizione**. L'**attesa** è spesa campionando continuamente l'**ingresso** fino a che questo non **diventa LOW**. Quando ciò accade allora vuol dire che si è rilevato un **fronte di discesa** per cui, qualora **in futuro**, all'arrivo di un **nuovo interrupt**, si determinasse sullo stesso ingresso un valore HIGH, allora si può essere certi di essere in presenza di un nuovo **fronte di salita**. Alla rilevazione del fronte **di discesa** il pulsante viene riabilitato per permettere la rilevazione del prossimo fronte di salita. La funzione di **debouncing** è garantita introducendo un tempo minimo di attesa tra un campionamento e l'altro.

Le attese sono tutte **non bloccanti** e realizzate tramite un timer HW che adopera esso stesso gli **interrupt** per richiamare la funzione di servizio (callback) da eseguire allo scadere del **timeout**. Il timer, utilizzando gli interrupt, è in grado di intervenire in tempo in tutte le situazioni, eventualmente interrompendo l'esecuzione di quel codice che impegna intensamente il loop(). Si tratta sicuramente di una realizzazione che, avendo la massima efficacia possibile in tutte le situazioni, si presta alla realizzazione di **dispositivi di sicurezza**.


```C++
#include <Ticker.h>
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
int led = 13;
byte pulsante =12;
volatile unsigned short numberOfButtonInterrupts = 0;
volatile bool pressed;
volatile bool stato;
#define DEBOUNCETIME 50
Ticker debounceTicker;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, CHANGE );
  numberOfButtonInterrupts = 0;
  pressed = false;
  stato = false;
}

void switchPressed ()
{
  numberOfButtonInterrupts++; // contatore rimbalzi
  bool val = digitalRead(pulsante); // lettura stato pulsante
  if(val && !pressed){ // fronte di salita
    pressed = true; // disarmo il pulsante
    debounceTicker.once_ms(50, waitUntilInputLow);  
    Serial.println("SALITA disarmo pulsante");
    stato = !stato; 	  // logica toggle  
  }
}  // end of switchPressed

void waitUntilInputLow()
{
    // sezione critica
    if (digitalRead(pulsante) == HIGH)// se il pulsante è ancora premuto
    { 
        Serial.print("Aspetto");
        Serial.print("HIT: "); Serial.println(numberOfButtonInterrupts);
        debounceTicker.once_ms(50, waitUntilInputLow);  
    }else{
        Serial.print("DISCESA riarmo pulsante\n");
        Serial.print("HIT: "); Serial.println(numberOfButtonInterrupts);
        numberOfButtonInterrupts = 0; // reset del flag
        pressed = false; // riarmo il pulsante
    }
}

// loop principale
void loop() {
	if (stato) {
		digitalWrite(led, !digitalRead(led));   	// inverti lo stato precedente del led
		delay(500);
	} else {
		digitalWrite(led, LOW);    	// turn the LED off by making the voltage LOW
    		delay(10);
	}
}
```

Le variabili **condivise** tra una ISR e il loop() andrebbero protette da accessi **paralleli** da parte di quelle due funzioni tramite delle **corse critiche** che rendano l'accesso **strettamente sequenziale**. Inoltre le variabili condivise devono sempre essere dichiarate con il qualificatore ```volatile``` per forzarne la modifica istantanea anche sui registri della CPU. 

Gli **accessi paralleli** non sono un problema quando le **istruzioni** sono **atomiche**, cioè non interrompibili. Le istruzioni atomiche o sono eseguite per intero o non sono eseguite affatto. Nei microcontrollori attuali, in genere **nessuna istruzione** gode della proprietà di essere **atomica** con una sola eccezione per la lettura e scrittura delle **variabili ad 8 bit**.

Per le variabili codificate con **8 bit** l'accesso a basso livello (linguaggio macchina) è intrinsecamente garantito essere **atomico**. Per queste variabili rimane comunque la necessita dell'uso del qualificatore ```volatile```.

L'unica variabile **condivisa** tra ISR e loop() nel progetto è ```stato``` che è ad 8 bit ed è stata dichiarata come ```volatile```.
In questo caso gli **accessi**, sia in lettura che in scrittura, sono quindi, a basso livello, **intrinsecamente sequenziali**.

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/350052113369268819

### **PULSANTE TOGGLE CON DEBOUNCER BASATO SU TIMER HW PER ARDUINO**

```C++
#include <TimerOne.h>
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
int led = 13;
byte pulsante =2;
volatile unsigned short numberOfButtonInterrupts = 0;
volatile bool pressed;
volatile bool stato;
#define DEBOUNCETIME 50
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, CHANGE );
  Timer1.initialize(50000);
  numberOfButtonInterrupts = 0;
  pressed = false;
  stato = false;
}

void switchPressed ()
{
  numberOfButtonInterrupts++; // contatore rimbalzi
  bool val = digitalRead(pulsante); // lettura stato pulsante
  if(val && !pressed){ // fronte di salita
    pressed = true; // disarmo il pulsante
    Timer1.start(); 
    Timer1.attachInterrupt(waitUntilInputLow);
    Serial.println("SALITA disarmo pulsante");
    stato = !stato; 	  // logica toggle  
  }
}  // end of switchPressed

void waitUntilInputLow()
{
    // sezione critica
    if (digitalRead(pulsante) == HIGH)// se il pulsante è ancora premuto
    { 
        Serial.print("Aspetto");
        Serial.print("HIT: "); Serial.println(numberOfButtonInterrupts);
    }else{
        Timer1.stop(); 
        Timer1.detachInterrupt();
        Serial.print("DISCESA riarmo pulsante\n");
        Serial.print("HIT: "); Serial.println(numberOfButtonInterrupts);
        numberOfButtonInterrupts = 0; // reset del flag
        pressed = false; // riarmo il pulsante
    }
}

// loop principale
void loop() {
	if (stato) {
		digitalWrite(led, !digitalRead(led));   	// inverti lo stato precedente del led
		delay(500);
	} else {
		digitalWrite(led, LOW);    	// turn the LED off by making the voltage LOW
		delay(10);
	}
}
```
Simulazione su Arduino con Wowki:  https://wokwi.com/projects/351244710765920855


### **PULSANTE TOGGLE CON DEBOUNCER BASATO SU TIMER HW PER ARDUINO 2**

Versione eseguibile senza l'ausilio di librerie adatta per il simulatore Tinkercad:

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
int led = 13;
byte pulsante =2;
volatile unsigned short numberOfButtonInterrupts = 0;
volatile bool pressed;
volatile bool stato;
#define DEBOUNCETIME 500
volatile bool knockKnock = false;
volatile int timer2Counter = 0;
#define WATCH 0
#define TIMER 1
#define TIMER2_POSTSCALER 1000

bool knockMeAfter(int t);  // t is the time in ms
void switchPressed ();
void waitUntilInputLow();

// impostazione prescaler timer HW monostabile
bool knockMeAfter(int t)       // t is the time in ms
{
  static int t_prev = 0;
  byte prescaler = 0;
  if (TIMSK2 == 0b00000010)
    return false;              // Timer2 is busy now
  TCNT2 = 0;                 // Reset counter
  if (t_prev != t) {           // Find the appropriate prescaler taking t in us
    if (t > 16320)
      t = 16320;
    if (t > 4080)
      prescaler = 0b00000111;  // f_cpu / 1024
    else if (t > 2040)
      prescaler = 0b00000110;  // f_cpu / 256
    else if (t > 1020)
      prescaler = 0b00000101;  // f_cpu / 128
    else if (t > 510)
      prescaler = 0b00000100;  // f_cpu / 64
    else if (t > 127)
      prescaler = 0b00000011;  // f_cpu / 32
    else if (t > 15)
      prescaler = 0b00000010;  // f_cpu / 8
    else
      prescaler = 0b00000001;  // No prescaling

    TCCR2A = 0b00000010;       // Mode 2: CTC Mode
    TCCR2B = prescaler;
    OCR2A = (float)t / ((float)(prescaler == 0b00000111 ? 1024 : prescaler == 0b00000110 ? 256 : prescaler == 0b00000101 ? 128 : prescaler == 0b00000100 ? 64 : prescaler == 0b00000011 ? 32 : prescaler == 0b00000010 ? 8 : 1) * 0.0625);
    t_prev = t;
  }
  TIMSK2 = 0b00000010;         // Interrupt on overflow
  sei();                       // Enable global interrupts
  return true;
}

// ISR interrupt tasto
void switchPressed ()
{
  numberOfButtonInterrupts++; // contatore rimbalzi
  bool val = digitalRead(pulsante); // lettura stato pulsante
  if(val && !pressed){ // fronte di salita
    pressed = true; // disarmo il pulsante
    knockMeAfter(DEBOUNCETIME);
    Serial.println("SALITA disarmo pulsante");
    stato = !stato; 	  // logica toggle  
  }
}  // end of switchPressed

// ISR interrupt timer HW
ISR(TIMER2_COMPA_vect)
{
  timer2Counter++;
  if (timer2Counter >= TIMER2_POSTSCALER) {
    timer2Counter = 0;
    TIMSK2 = 0b00000000;   // resetto il timer inibendo ulteriori interrupt
    waitUntilInputLow();
  }
}

void waitUntilInputLow()
{
    if (digitalRead(pulsante) == HIGH)// se il pulsante è ancora premuto
    { 
        Serial.print("Aspetto");
        Serial.print("HIT: "); Serial.println(numberOfButtonInterrupts);
        knockMeAfter(DEBOUNCETIME);
    }else{
        Serial.print("DISCESA riarmo pulsante\n");
        Serial.print("HIT: "); Serial.println(numberOfButtonInterrupts);
        numberOfButtonInterrupts = 0; // reset del flag
        pressed = false; // riarmo il pulsante
    }
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, CHANGE );
  numberOfButtonInterrupts = 0;
  pressed = false;
  stato = false;
}

// loop principale
void loop() {
	if (stato) {
		digitalWrite(led, !digitalRead(led));   	// inverti lo stato precedente del led
		delay(500);
	} else {
		digitalWrite(led, LOW);    	// turn the LED off by making the voltage LOW
    		delay(10);
	}
}
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/iui5Fd9MYtU?editbtn=1

Simulazione su Arduino con Wowki:  https://wokwi.com/projects/351242718844813912

>[Torna all'indice](indexinterrupts.md)
