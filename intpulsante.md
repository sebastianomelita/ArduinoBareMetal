>[Torna all'indice](indexinterrupts.md)
## **PULSANTE CON INTERRUPT**

Gli ingressi rilevati **tramite un interrupt** vengono sicuramente serviti in maniera **più rapida** rispetto ad altre soluzioni (delay, polling della millis(), thread), questo perchè l'arrivo di un segnale di interrupt blocca immediatamente l'esecuzione di un task direttamente sull'**istruzione che è attualmente in esecuzione** al momento dell'arrivo dell'interrupt. Mantiene la complessità (tutto sommato non elevata) di una normale chiamata di funzione ed è attivato a ridosso di un qualunque evento asincrono, del quale, per definizione, non è mai possibile prevedere in anticipo l'accadere. 

**In definitiva**, tra le tecniche più responsive, quella dell'interrupt è certamente la **più veloce** e quindi non solo garantisce una adeguata responsività ai comandi ma anche la massima velocità possibile che è un oprerequisito necessario per il comando efficace dei **dispositivi di sicurezza**, cioè quei dispositivi critici che devono proteggere beni e persone da danni irreparabili.

### **PULSANTE TOGGLE**

Di seguito è riportato un esempio di pulsante con antirimbalzo realizzato con interrupt, polling di un flag e la generazione di un evento pressione/rilascio debounced (senza rimbalzi) grazie ad un timer polled. 

L'operazione avviene in **due fasi**, una nella **ISR** e l'altra in un **polling nel loop()**. Nella **ISR** viene rilevato prontamente un **fronte** (di salita o di discesa), viene campionato il livello successivo e viene settato il **flag di segnalazione** ```numberOfButtonInterrupts```. Nel **```loop()```** viene eseguito il polling del flag ed eseguito il codice dell'**antirimbalzo** se esso risulta asserito.

Può leggere sia fronti di salita che di discesa in maniera immune ai rimbalzi. Il tipo di fronte si seleziona valutando la variabile lastState.

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
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), switchPressed, CHANGE );  
  
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


### **PULSANTE DI SICUREZZA**

Il codice precedente, per quanto **molto reponsivo**, non è adatto a realizzare un **blocco di sicurezza** per via del **ritardo** nell'intervento di attivazione e disattivazione dell'uscita cusato dalll'algoritmo di **debouncing** (antirimbalzo). Per adattarlo a quest'ultimo scopo, il codice va modificato in modo da avere un intervento **immediato** su uno dei fronti (quello che comanda lo sblocco dell'alimentazione) ed uno ritardato sull'altro (quello che comanda il riarmo).

```C++
const unsigned long DEBOUNCETIME = 50;
const byte ENGINE = 7;
const byte BUTTONPIN = 3;
volatile unsigned long previousMillis = 0;
volatile unsigned short numberOfButtonInterrupts = 0;
volatile bool lastState;
bool prevState;

// Interrupt Service Routine (ISR)
void stopEngine ()
{
  numberOfButtonInterrupts++; // contatore rimbalzi
  lastState = digitalRead(BUTTONPIN); // lettura stato pulsante
  previousMillis = millis(); // tempo evento
  if(lastState==HIGH){ // fronte di salita
  	digitalWrite(ENGINE, LOW); // blocco subito il motore
  }
}  

void setup ()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);  	  // so we can update the LED
  digitalWrite(BUTTONPIN, HIGH);  // internal pull-up resistor
  // attach interrupt handler
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), stopEngine, CHANGE );  
  
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
	  }else{
		Serial.println(" in DISCESA");
		digitalWrite(ENGINE, HIGH); // riattivo il motore
	  }
     }
}
```

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/348783670266430034

>[Torna all'indice](indexinterrupts.md)
