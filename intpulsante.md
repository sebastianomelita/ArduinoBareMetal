## **PULSANTE CON INTERRUPT**

Di seguito è riportato un esempio di pulsante con antirimbalzo realizzato con interrupt, polling di un flag e la generazione di un evento pressione/rilascio debounced (senza rimbalzi) grazie ad un timer polled. 
Può leggere sia fronti di salita che di discesa in maniera immune ai rimbalzi. Il tipo di fronte si seleziona valutando la variabile lastState.
Da seriale si può vedere il numero di eventi di interrupt spuri che sono stati generati a causa dei rimbalzi e che sono stati filtrati dal timer.

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
valgono le proprietà che:
-	è vera se le tre condizioni sono contemporaneamente vere
-	le condizioni a destra sono valutate solo se quelle a sinistra sono vere (la lenta digitalRead() non si fa se non necessario)
