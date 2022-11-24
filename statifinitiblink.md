>[Torna all'indice](indexstatifiniti.md)
## **ESEMPIO DI SFM**
Ci concentreremo sugli aspetti pratici della programmazione di un compito utilizzando un FSM, utilizzando il LED lampeggiante come esempio iniziale. Per essere chiari, questa non è un'attività che necessita davvero di un approccio FSM, ma è utile illustrare come passare dal codice lineare a una struttura FSM con un esempio semplice e familiare.

Il codice **“lineare”** avrebbe grossomodo questa forma:

```C++
#define LED_DELAY 1000       // periodo base in millisecondi
byte LED =7; 
unsigned long timeLastTransition =0;  

void setup()
{
  	pinMode(LED, OUTPUT);   
}

void loop()
{
	blink();
}

void blink(){
	if(millis()-precm >= (unsigned long) LED_DELAY){  
		timeLastTransition = millis();        
		digitalWrite(LED, !digitalRead(LED));		
	}
}
```

Una prima **versione FSM** del codice potrebbe essere:

```C++
void blink()
{
  const uint32_t LED_DELAY = 1000;
  static uint8_t state = 0;
  static uint32_t timeLastTransition = 0;
 
  switch (state)
  {
    case 0:   // toggle the LED 
      digitalWrite(LED, !digitalRead(LED));
      timeLastTransition = millis();
      state = 1;
      break;
 
    case 1:   // wait for the delay period
      if (millis() - timeLastTransition >= LED_DELAY)
        state = 0;
      break;
  }
}
```

Dove possiamo individuare due stati: 0 in cui avviene la transizione, 1 in cui si attende il momento giusto per la transizione 
Possiamo far evolvere questo codice FSM per renderlo più robusto, leggibile e gestibile.


```C++
void blink(bool reset = false)
{
  const uint32_t LED_DELAY = 1000;
  static enum { LED_TOGGLE, WAIT_DELAY } state = LED_TOGGLE;
  static uint32_t timeLastTransition = 0;
 
  if (reset)
  { 
    state = LED_TOGGLE;
    digitalWrite(LED_BUILTIN, LOW);
  }
 
  switch (state)
  {
    case LED_TOGGLE:   // toggle the LED
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      timeLastTransition = millis();
      state = WAIT_DELAY;
      break;
 
    case WAIT_DELAY:   // wait for the delay period
      if (millis() - timeLastTransition >= LED_DELAY)
        state = LED_TOGGLE;
      break;
 
    default:
      state = LED_TOGGLE;
      break
  }
}
```
Analizziamo le modifiche effettuate:
-	blink () accetta un parametro booleano opzionale che consente al codice di ripristinare l'FSM a uno stato noto. Questo può essere fatto in setup () o ogni volta che il lampeggiamento deve fermarsi e spegnere il LED.
-	lo stato è definito come un tipo enumerato. Ciò rende molto più facile leggere e seguire il codice, soprattutto quando ci sono più di una manciata di stati. Evita anche il problema pratico di rinumerare tutti gli stati quando vengono aggiunti o eliminati stati intermedi aggiuntivi (per esperienza, questo accadrà!).
-	lo stato predefinito nell'istruzione switch è una misura difensiva per garantire che qualsiasi stato non definito o danneggiato non arresti l'FSM - uno stato non gestito altrimenti salterebbe semplicemente l'intero blocco switch! Esempi di quando ciò può verificarsi è se viene aggiunto uno stato e si omette il codice nell'istruzione switch o se la memoria della variabile di stato viene danneggiata.

In realtà la definizione degli stati possiede dei margini di discrezionalità e quelli scelti per realizzare il blink possono infatti sembrare un tantino artificiosi.
>[Torna all'indice](indexstatifiniti.md)
