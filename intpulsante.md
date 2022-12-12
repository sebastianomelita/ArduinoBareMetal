>[Torna all'indice](indexinterrupts.md)
## **PULSANTE CON INTERRUPT**

Gli ingressi rilevati **tramite un interrupt** vengono sicuramente serviti in maniera **più rapida** rispetto ad altre soluzioni (delay, polling della millis(), thread), questo perchè l'arrivo di un segnale di interrupt blocca immediatamente l'esecuzione di un task direttamente sull'**istruzione che è attualmente in esecuzione** al momento dell'arrivo dell'interrupt. Mantiene la complessità (tutto sommato non elevata) di una normale chiamata di funzione ed è attivato a ridosso di un qualunque **evento asincrono**, del quale, per definizione, non è mai possibile prevedere in anticipo l'accadere. 

**In definitiva**, tra le tecniche più responsive, quella dell'interrupt è certamente la **più veloce** e quindi non solo garantisce una adeguata responsività ai comandi ma anche la massima velocità di risposta possibile che è un prerequisito necessario per il comando efficace dei **dispositivi di sicurezza**, cioè di quei dispositivi critici deputati alla protezione di beni e persone da danni irreparabili.

### **PULSANTE TOGGLE CON DEBOUNCER BASATO SU POLLING**

Di seguito è riportato un esempio di pulsante con antirimbalzo realizzato con interrupt, polling di un flag e la generazione di un evento pressione/rilascio debounced (senza rimbalzi) grazie ad un timer polled. 

L'operazione avviene in **due fasi**, una nella **ISR** e l'altra in un **polling nel loop()**. Nella **ISR** viene rilevato prontamente un **fronte** (di salita o di discesa), viene campionato il livello successivo e viene settato il **flag di segnalazione** ```numberOfButtonInterrupts```. Nel **```loop()```** viene eseguito il polling del flag ed eseguito il codice dell'**antirimbalzo** se esso risulta asserito.

Può leggere sia fronti di salita che di discesa in maniera immune ai rimbalzi. Il tipo di fronte si seleziona valutando la variabile ```lastState```.

Da seriale si può vedere il numero di eventi di interrupt spuri che sono stati generati a causa dei rimbalzi e che sono stati filtrati dal timer (solo nella simulazione su Esp32 mettendo la spunta al flag debounce del tasto).

Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/jWma7uWoY7T?editbtn=1

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/349402186520724050

```C++
const unsigned long DEBOUNCETIME = 50;
const byte LED = 7;
const byte BUTTONPIN = 3;
volatile unsigned long previousMillis = 0;
volatile unsigned short numberOfButtonInterrupts = 0;
volatile bool lastState;
bool prevState;

// Interrupt Service Routine (ISR)
void switchPressed ()
{
  numberOfButtonInterrupts++; // contatore rimbalzi
  lastState = digitalRead(BUTTONPIN); // lettura stato pulsante
  previousMillis = millis(); // tempo evento
}  // end of switchPressed

void setup ()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);  	  // so we can update the LED
  digitalWrite(BUTTONPIN, HIGH);  // internal pull-up resistor
  // attach interrupt handler
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), switchPressed, CHANGE);  
  numberOfButtonInterrupts = 0;
}  // end of setup

void loop ()
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
	&& prevState != lastState // elimina transizioni anomale LL o HH 
	&& digitalRead(BUTTONPIN) == lastState)//se coincide con il valore di un polling
    { 
	  Serial.print("HIT: "); Serial.print(numberOfButtonInterrupts);
	  numberOfButtonInterrupts = 0; // reset del flag
	  
	  prevState = lastState;
	  if(lastState){ // fronte di salita
	    Serial.println(" in SALITA");
		digitalWrite(LED, !digitalRead(LED));
	  }else{
		Serial.println(" in DISCESA");
	  }
     }
}
```
Vale il **“corto circuito”** degli operatori, cioè nel valutare l’espressione
```C++
(numberOfButtonInterrupts != 0) && (millis() - lastintTime > DEBOUNCETIME ) && digitalRead(BUTTONPIN) == lastState)
```
Valgono le proprietà che:
-	è vera se le tre condizioni sono contemporaneamente vere
-	le condizioni a destra sono valutate solo se quelle a sinistra sono vere (la lenta digitalRead() non si fa se non necessario)

Le variabili **condivise** tra una ISR e il loop() andrebbero protette da accessi **paralleli** da parte di quelle due funzioni tramite delle **corse critiche** che rendano l'accesso **strettamente sequenziale**. Inoltre le variabili condivise devono sempre essere dichiarate con il qualificatore ```volatile``` per forzarne la modifica istantanea anche sui registri della CPU. 

Gli **accessi paralleli** non sono un problema quando le **istruzioni** sono **atomiche**, cioè non interrompibili. Le istruzioni atomiche o sono eseguite per intero o non sono eseguite affatto. In questo caso gli **accessi**, sia in lettura che in scrittura, sono in realtà, a basso livello, **intrinsecamente sequenziali**.

Nei microcontrollori attuali, in genere **nessuna istruzione** gode della proprietà di essere **atomica** con una sola eccezione per la lettura e scrittura delle **variabili ad 8 bit**. Per le variabili codificate con **8 bit** l'accesso a basso livello (linguaggio macchina) è intrinsecamente garantito essere **atomico**. Per queste variabili rimane comunque la necessita dell'uso del qualificatore ```volatile```.

Le **modifiche** a valori con codifiche **maggiori di 8 bit** sono in genere **non atomiche**, pertanto le variabili a 16 o 32 bit andrebbero gestite con gli interrupt disabilitati (sezione critica). Tuttavia, gli interrupt vengono disabilitati di default durante una routine di servizio di interrupt, quindi, non potendo verificarsi il danneggiamento di una variabile multibyte in una ISR, le **sezioni critiche** vanno inserite soltanto nel ```loop()```.

Le variabili **condivise** tra ISR e loop() e **8 bit** sono ```numberOfButtonInterrupts```, ```prevState``` e ```lastState``` che sono stata semplicemente dichiarate come ```volatile``` senza sezioni critiche su di essa.

L'unica variabile **condivisa** tra ISR e loop() e **16 o 32 bit** sono ```previousMillis``` che è stata dichiarata come ```volatile``` e ha nel loop() una **sezione critica** intorno all'accesso in lettura su di essa.

### **PULSANTE DI SICUREZZA BASATO SU TIMER SW (POLLING)**

Il codice precedente, per quanto **molto reponsivo**, non è adatto a realizzare un **blocco di sicurezza** per via del **ritardo** nell'intervento di attivazione e disattivazione dell'uscita causato dalll'algoritmo di **debouncing** (antirimbalzo). Per adattarlo a quest'ultimo scopo, il codice va modificato in modo da avere un intervento **immediato** su uno dei fronti (quello che comanda lo sblocco dell'alimentazione) ed uno ritardato (per realizzare il debouncing) sull'altro (quello che comanda il riarmo).  

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
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/jWfXle1Us2E?editbtn=1

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/348783670266430034

### **PULSANTE TOGGLE CON DEBOUNCER BASATO SU TIMER HW**

All'**ingresso** di una **porta digitale**, per ottenere la rilevazione **sicura** (senza rimbalzi) ma anche **tempestiva** (più rapida possibile) del solo **fronte di salita** è stata usata la **combinazione** di due tecniche di schedulazione:
- una **asincrona** (una ISR), non governata dal sistema, ma da un segnale di **interrupt** in ingresso proveniente dall'**esterno**, per la determinazione istantanea (o quasi) del suo fronte di salita per poter elaborare la risposta il più vicino possibile all'evento che la ha causata.
- una **sincrona** (una callback), gestita dal sistema tramite un **timer HW**, per la realizzazione della funzione di debouncing (antirimbalzo) del segnale in ingresso.

Il **rilevatore dei fronti** è realizzato **campionando** il valore del livello al momento dell'arrivo del segnale di interrupt e **confrontandolo** con il valore del livello campionato in istanti **periodici** successivi a quello, pianificati (schedulati) tramite un timer HW, allo scadere del quale viene chiamata l'istruzione ```waitUntilInputLow()```. La funzione, di fatto, esegue una **disabilitazione** della rilevazione dei fronti (per evitare letture duplicate dello stesso) in **"attesa"** che si verifichi una certa **condizione**, L'**attesa** è spesa campionando continuamente l'**ingresso** fino a che questo non **diventa LOW**. Quando ciò accade allora vuol dire che si è rilevato un **fronte di discesa** per cui, qualora **in futuro**, all'arrivo di un **nuovo interrupt**, si determinasse sullo stesso ingresso un valore HIGH, allora si può essere certi di essere in presenza di un nuovo **fronte di salita**. Alla rilevazione del fronte **di discesa** il pulsante viene riabilitato per permettere la rilevazione del prossimo fronte di salita. La funzione di **debouncing** è garantita introducendo un tempo minimo di attesa tra un campionamento e l'altro.

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

Le variabili **condivise** tra una ISR e il loop() andrebbero protette da accessi **paralleli** da parte di quellew due funzioni tramite delle **corse critiche** che rendano l'accesso **strettamente sequenziale**. Inoltre le variabili condivise devono sempre essere dichiarate con il qualificatore ```volatile``` per forzarne la modifica istantanea anche sui registri della CPU. 

Gli **accessi paralleli** non sono un problema quando le **istruzioni** sono **atomiche**, cioè non interrompibili. Le istruzioni atomiche o sono eseguite per intero o non sono eseguite affatto. Nei microcontrollori attuali, in genere **nessuna istruzione** gode della proprietà di essere **atomica** con una sola eccezione per la lettura e scrittura delle **variabili ad 8 bit**.

Per le variabili codificate con **8 bit** l'accesso a basso livello (linguaggio macchina) è intrinsecamente garantito essere **atomico**. Per queste variabili rimane comunque la necessita dell'uso del qualificatore ```volatile```.

L'unica variabile **condivisa** tra ISR e loop() nel progetto è ```stato``` che è ad 8 bit ed è stata dichiarata come ```volatile```.
In questo caso gli **accessi**, sia in lettura che in scrittura, sono quindi, a basso livello, **intrinsecamente sequenziali**.

Simulazione su Arduino con Tinkercad: 

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/350052113369268819

>[Torna all'indice](indexinterrupts.md)
