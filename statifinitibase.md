**MACCHINE A STATI FINITI**

Un automa a stati finiti (o macchina a stati finiti) è un modello astratto utile per rappresentare il comportamento di un sistema. 
Si basa sull’assunto che un sistema sia modellabile con un insieme finito di stati interni e che esso passi da uno all’altro, a seguito di uno stimolo in ingresso. In letteratura si distinguono due tipi principali di automi, l’automa di Mealy e quallo di Moore. La differenza tra i due sta nella particolare maniera con cui si genera l’uscita: nel caso di Moore essa è dipendente solo dallo stato e da null’altro, nel caso di Mealy l’uscita, oltre che con lo stato, varia anche in base all’ingresso applicato.

In informatica privilegeremo la rappresentazione di Mealy che possiamo immaginare composta da un certo numero di stati (situazioni diverse in cui si può trovare il sistema), da un certo numero di ingressi (eventi utente o generati dall’HW) e da un certo numero di uscite (azioni svolte). 
Vale la relazione (stato, ingresso)  uscita cioè, fissati uno stato e un certo ingresso si genera sempre la stessa uscita.
L'usuale rappresentazione grafica di un automa a stati finiti è il grafo orientato, dove i cerchi rappresentano lo stato e gli archi la transizione da uno stato all’altro. Le etichette degli archi rappresentano l’ingresso che ha generato la transizione usualmente rappresentato abbinato all’uscita da esso scatenata. Ingressi (eventi) diversi si rappresentano con archi diversi e possono generare uscite diverse.

![statemachine](statemachine.png)                      

Una rappresentazione alternativa al grafo, buona per la memorizzazione in un PC, è la matrice di transizione, dove, per ogni stato (righe) e per ogni ingresso (colonne) si trova, all’incrocio, il valore dello stato successivo.
La macchina a stati finiti è una tecnica importante nella programmazione embedded per creare codice che non si blocchi pur facendo accadere molte cose contemporaneamente (multitasking). Comunemente, si rappresentano con macchine a stati la schedulazione dei processi e dei thread, ma anche l’evoluzione di quei protocolli di comunicazione in cui i dati, scambiati in un certo istante, sono correlati a quelli scambiati in istanti precedenti (ad esempio dati di acquisto solo dopo che sono stati forniti dati di login).
Ci concentreremo sugli aspetti pratici della programmazione di un compito utilizzando un FSM, utilizzando il LED lampeggiante come esempio iniziale. Per essere chiari, questa non è un'attività che necessita davvero di un approccio FSM, ma è utile illustrare come passare dal codice lineare a una struttura FSM con un esempio semplice e familiare.

Il codice “lineare” avrebbe grossomodo questa forma:

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

Una prima versione FSM del codice potrebbe essere:

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
