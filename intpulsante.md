>[Torna all'indice](indexinterrupts.md)
## **PULSANTE CON INTERRUPT**

Gli ingressi rilevati **tramite un interrupt** vengono sicuramente serviti in maniera **più rapida** rispetto ad altre soluzioni (delay, polling della millis(), thread), questo perchè l'arrivo di un segnale di interrupt blocca immediatamente l'esecuzione di un task direttamente sull'**istruzione che è attualmente in esecuzione** al momento dell'arrivo dell'interrupt. Mantiene la complessità (tutto sommato non elevata) di una normale chiamata di funzione ed è attivato a ridosso di un qualunque **evento asincrono**, del quale, per definizione, non è mai possibile prevedere in anticipo l'accadere. 

**In definitiva**, tra le tecniche più responsive, quella dell'interrupt è certamente la **più veloce** e quindi non solo garantisce una adeguata responsività ai comandi ma anche la massima velocità di risposta possibile che è un prerequisito necessario per il comando efficace dei **dispositivi di sicurezza**, cioè di quei dispositivi critici deputati alla protezione di beni e persone da danni irreparabili.

La **gestione** di un **pulsante** mediante gli **interrupts** passa sostanzialmente per **due fasi**:
- l'**attivazione** asincrona e immediata su un **fronte** del valore della porta digitale di ingresso (di salita o di discesa)
- l'**impostazione** del pulsante per una nuova attivazione (riarmo). Non è immediata, perchè, prima di consentire nuove pressioni, va speso del **tempo** per lasciare **estinguere le oscillazioni elettriche** causate dai **rimbalzi meccanici** del pulsante.

Le **tecniche individuate** nella presente dispensa sono sostanzialmente **le stesse** per quanto riguarda la prima fase di attivazione mentre **differiscono** nel modo con cui viene realizzato il **ritardo** necessario per il **debouncing**.

Per una discussione più completa sugli interrupt vedi [interrupt](interruptsbase.md).

<img src="img\interruptPulsante.jpg" alt="alt text" width="1200">



### **PULSANTE TOGGLE BASATO SU INTERRUPTS E DEBOUNCE CON TIMER SW NEL LOOP 1**

Per una discussione generale sugli interrupt si rimanda a [interrupt](indexinterrupts.md).

All'**ingresso** di una **porta digitale**, per ottenere la rilevazione **sicura** (senza rimbalzi) del solo **fronte di salita** è stata usata la **combinazione** di due tecniche di schedulazione:
- una **asincrona** (una ISR), non governata dal sistema, ma da un segnale di **interrupt** in ingresso proveniente dall'**esterno**, per la determinazione istantanea (o quasi) del suo fronte di salita per poter elaborare la risposta il più vicino possibile all'evento che la ha causata.
- una **sincrona** (un polling), gestita dal sistema tramite un il polling della funzione millis(), per la realizzazione della funzione di debouncing (antirimbalzo) del segnale in ingresso.

Il **rilevatore dei fronti** è realizzato **campionando** il valore del livello al momento dell'arrivo del segnale di interrupt e **confrontandolo** con il valore del livello campionato in istanti **periodici** successivi a quello, pianificati (schedulati) tramite un timer HW, allo scadere del quale viene chiamata l'istruzione ```waitUntilInputChange()```. La funzione, di fatto esegue il **filtraggio** delle segnalazioni di un flag (```numberOfButtonInterrupts```) impostato dalla ISR chiamata dagli interrupt sulla porta digitale. Le segnalazioni vengono filtrate se troppo in anticipo (funzione di debouncing) e se si riferiscono a letture dello stesso livello (non transizioni). La funzione di **debouncing** è garantita introducendo un **tempo minimo** di attesa tra un campionamento e l'altro.

Pur utilizzando gli interrupt, l'efficacia del codice precedente in termini di velocità e responsività è limitata dalla **componente nel loop()** del meccanismo che purtroppo è sensibile ai ritardi di esecuzione. I **ritardi** possono essere introdotti da istruzioni delay() o da blocchi di istruzioni troppo lente. E'probabilmente una realizzazione **poco pratica**, soprattutto per **dispositivi di sicurezza**, perchè **la velocità** degli interrupts potrebbe essere vanificata dalla potenziale **lentezza** del polling del flag. 

Una realizzazione di interrupt con debouncing SW che garantisce un intervento immediato è riportata in: [interruttore di sicurezza SW](intpulsante.md#**PULSANTE-DI-SICUREZZA-CON-DEBOUNCER-BASATO-SU-TIMER-SW-(POLLING)**)

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
int led = 13;
byte pulsante =12;
byte stato= LOW;  // variabile globale che memorizza lo stato del pulsante
volatile unsigned long previousMillis = 0;
volatile unsigned long lastintTime = 0;
bool started = false;
volatile bool pressed;
#define DEBOUNCETIME 50
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );  
  pressed = false;
}

// Interrupt Service Routine (ISR)
void switchPressed ()
{
  if(!pressed){
    // può essere omessa se viene tolto l'attach() nel loop()
    detachInterrupt(digitalPinToInterrupt(pulsante));
    Serial.println("disarmo interrupts timer");
    pressed = true; // disarmo del pulsante e riarmo del timer
  }
    
}  // end of switchPressed

void waitUntilInputChange()
{
    if(pressed){ 
      if(!started){
        started = true;// aggiorna il millis() solo alla prima di molte chiamate consecutive
        lastintTime = millis();
      }
      if((millis() - lastintTime > DEBOUNCETIME ) && digitalRead(pulsante) == LOW){
        stato = !stato; // logica da attivare sul fronte (toggle)
        // può essere omessa se eviene tolto il detach() nella ISR
        attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );
        Serial.println("riarmo interrupts timer");
        started = false; // disarmo del timer
        pressed = false; // riarmo del pulsante
      }
    }
}
// loop principale
void loop() {
	waitUntilInputChange();
	digitalWrite(led,stato);   	// inverti lo stato precedente del led
  delay(10);
}
```

Le variabili **condivise** tra una ISR e il loop() andrebbero protette da accessi **paralleli** da parte di quellew due funzioni tramite delle **corse critiche** che rendano l'accesso **strettamente sequenziale**. Inoltre le variabili condivise devono sempre essere dichiarate con il qualificatore ```volatile``` per forzarne la modifica istantanea anche sui registri della CPU. 

Gli **accessi paralleli** non sono un problema quando le **istruzioni** sono **atomiche**, cioè non interrompibili. Le istruzioni atomiche o sono eseguite per intero o non sono eseguite affatto. In questo caso gli **accessi**, sia in lettura che in scrittura, sono in realtà, a basso livello, **intrinsecamente sequenziali**.

Nei microcontrollori attuali, in genere **nessuna istruzione** gode della proprietà di essere **atomica** con una sola eccezione per la lettura e scrittura delle **variabili ad 8 bit**. Per le variabili codificate con **8 bit** l'accesso a basso livello (linguaggio macchina) è intrinsecamente garantito essere **atomico**. Per queste variabili rimane comunque la necessita dell'uso del qualificatore ```volatile```.

Le **modifiche** a valori con codifiche **maggiori di 8 bit** sono in genere **non atomiche**, pertanto le variabili a 16 o 32 bit andrebbero gestite con gli interrupt disabilitati (sezione critica). Tuttavia, gli interrupt vengono disabilitati di default durante una routine di servizio di interrupt, quindi, non potendo verificarsi il danneggiamento di una variabile multibyte in una ISR, le **sezioni critiche** vanno inserite soltanto nel ```loop()```.

Le variabili **condivise** tra ISR e loop() e **8 bit** sono ```numberOfButtonInterrupts```, ```prevState``` e ```lastState``` che sono stata semplicemente dichiarate come ```volatile``` senza sezioni critiche su di essa.

L'unica variabile **condivisa** tra ISR e loop() e **16 o 32 bit** sono ```previousMillis``` che è stata dichiarata come ```volatile``` e ha nel loop() una **sezione critica** intorno all'accesso in lettura su di essa.

Simulazione online su ESP32 del codice precedente con Wowki: https://wokwi.com/projects/388450490165203969

Simulazione online su ESP32 del codice precedente con Wowki: https://wokwi.com/projects/350016534055223891


Con timer non bloccante basato su libreria urutils: https://wokwi.com/projects/422059696428863489

Con timer basato su millis(): https://wokwi.com/projects/421497374229509121


### **Pulsante toggle basato su interrupts e con debounce basato sui delay()**

All'**ingresso** di una **porta digitale** viene associata una callback che viene invocata alla ricezione di un segnale di interrupt attivo su entrambi i fronti. Il fronte di salita, **selezionato** prendendo solo i valori HIGH, potrebbe essere rilevato molte volte consecutivamente a causa del fenomeno dei rimbalzi. 

Per evitare la rilevazione dei **fronti spuri** successivi al primo, viene disabilitata, dentro la ISR, la loro rilevazione **disarmando** gli interrupt  mediante l'istruzione ```detachInterrupt(digitalPinToInterrupt(pulsante))```. Contemporaneamente viene asserito un flag di segnalazione, ```pressed```, che comunica ad un loop() di attivare il timer per il riarmo dell'interrupt per rispondere a nuove pressioni dell'utente. 

Il tempo per la **riabilitazione** (riarmo) dell'interrupt non deve essere ne troppo presto, cioè minore di 50 msec, altrimenti si finisce per leggere dei rimbalzi ma neppure troppo tardi, altrimenti si perdono degli input dell'utente. Il momento migliore per riabilitare gli interrupt potrebbe essere il momento del rilascio del pulsante, dato che precede sempre una eventuale successiva pressione. In ogni caso, un timer impedisce quei tentativi di riabilitazione che potrebbero avvenire prima dei 50 msec utili ad evitare i rimbalzi.

```C++
#include "urutils.h"
int led = 13;
int pulsante =12;
volatile bool stato = false;  // variabile globale che memorizza lo stato del pulsante
volatile bool pressed;
#define DEBOUNCETIME 50
DiffTimer debounce;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  // arma l'interrupt
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );
  // disarma il timer 
  pressed = false;
}

// Interrupt Service Routine (ISR)
void switchPressed ()
{
  if(!pressed){// evita detach durante un attach nel loop
    // disarma l'interrupt
    // può essere omessa se viene tolto l'attach() nel loop()
    detachInterrupt(digitalPinToInterrupt(pulsante));
    // arma il timer
    pressed = true; 
  }
    
}  // end of switchPressed

// loop principale
void loop() {
  if(pressed){// fronte di salita
    waitUntilInputLow(pulsante, 100); 
    // fronte di discesa
    stato = !stato; // logica da attivare sul fronte (toggle)
    // riarma l'interrupt
    // può essere omessa se eviene tolto il detach() nella ISR
    attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING ); 
    pressed = false; // disarma il timer
  }
  digitalWrite(led, stato);   	// inverti lo stato precedente del led
  delay(10);
}
```
La variabile **```pressed```** è di appena **un byte** per cui le operazioni di **assegnamento** e di **confronto** su di essa sono sempre **atomiche** in tutti i microcontrollori (quindi sia Arduino che ESP32). Per renderla **thread-safe** basta qualificarla **volatile** nella dichiarazione.

Anche **senza detach/attach**, il codice rimane **thread-safe** grazie alla **logica di guardia** sulla variabile **pressed**. Non è necessario proteggere waitUntilInputChange() con interrupts()/noInterrupts(), infatti:
- Inizialmente pressed = false
- La ISR può attivarsi solo quando pressed è false, e lo imposta a true
- Una volta che pressed è true, la ISR non può più modificare nulla (per la condizione if(!pressed))
- Il loop processa l'evento e alla fine ripristina pressed = false

L'unica differenza pratica tra le due versioni (con e senza detach/attach) è che:
- **Con detach/attach**: l'interrupt viene proprio **disabilitato fisicamente**
- **Senza detach/attach**: l'interrupt può attivarsi ma la sua **ISR non fa nulla** (esce subito per la condizione su pressed)

Dalle considerazioni precedenti si deduce che è possibile eliminare l'**overhead** delle chiamate alle funzioni **attach()** e **detach()** utilizzando solamente il **flag pressed**. Il prezzo da pagare è un maggiore **overhead** della **ISR** che, a seguito di un interrupt, potrebbe essere chiamata anche un **centinaio** di volte consecutivamente.

- Simulazione online su ESP32 del codice precedente con Wowki: https://wokwi.com/projects/390288516762524673
- Simulazione online su ESP32 del codice precedente con Tinkercad: https://www.tinkercad.com/things/4KnKm94hoP1-toggle-interrupt-delay


### **Pulsante toggle basato su interrupts e timer debounce con timer SW get()**

All'**ingresso** di una **porta digitale** viene associata una callback che viene invocata alla ricezione di un segnale di interrupt attivo su entrambi i fronti. Il fronte di salita, **selezionato** prendendo solo i valori HIGH, potrebbe essere rilevato molte volte consecutivamente a causa del fenomeno dei rimbalzi. 

Per evitare la rilevazione dei **fronti spuri** successivi al primo, viene disabilitata, dentro la ISR, la loro rilevazione **disarmando** gli interrupt  mediante l'istruzione ```detachInterrupt(digitalPinToInterrupt(pulsante))```. Contemporaneamente viene asserito un flag di segnalazione, ```pressed```, che comunica ad un loop() di attivare il timer per il riarmo dell'interrupt per rispondere a nuove pressioni dell'utente. 

Il tempo per la **riabilitazione** (riarmo) dell'interrupt non deve essere ne troppo presto, cioè minore di 50 msec, altrimenti si finisce per leggere dei rimbalzi ma neppure troppo tardi, altrimenti si perdono degli input dell'utente. Il momento migliore per riabilitare gli interrupt potrebbe essere il momento del rilascio del pulsante, dato che precede sempre una eventuale successiva pressione. In ogni caso, un timer impedisce quei tentativi di riabilitazione che potrebbero avvenire prima dei 50 msec utili ad evitare i rimbalzi.

Variante che **disabilita** gli interrupt spuri fino al rilascio del pulsante: 

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
#include "urutils.h"
int led = 13;
byte pulsante =12;
byte stato= LOW;  // variabile globale che memorizza lo stato del pulsante
volatile bool pressed;
#define DEBOUNCETIME 50
DiffTimer debounce;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );  
  pressed = false;
}

// Interrupt Service Routine (ISR)
void switchPressed ()
{
 if(!pressed){// evita detach durante un attach nel loop
    // disarma l'interrupt
    // può essere omessa se viene tolto l'attach() nel loop()
    detachInterrupt(digitalPinToInterrupt(pulsante));
    // arma il timer
    pressed = true; 
 }
}  // end of switchPressed

void waitUntilInputChange()
{
    if (pressed){ 
      debounce.start();// aggiorna il millis() interno solo alla prima di molte chiamate consecutive
      if(debounce.get() > DEBOUNCETIME  && digitalRead(pulsante) == LOW){// disarmo del timer al timeout
        debounce.stop(); // disarmo del timer
        debounce.reset();
        // fronte di discesa
        stato = !stato; // logica da attivare sul fronte (toggle)
        // riarma l'interrupt
        // può essere omessa se eviene tolto il detach() nella ISR
        attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );
        pressed = false; // riarmo del pulsante
      }
    }
}
// loop principale
void loop() {
	waitUntilInputChange();
	digitalWrite(led, stato);   	// inverti lo stato precedente del led
  delay(10);
}
```
La variabile **```pressed```** è di appena **un byte** per cui le operazioni di **assegnamento** e di **confronto** su di essa sono sempre **atomiche** in tutti i microcontrollori (quindi sia Arduino che ESP32). Per renderla **thread-safe** basta qualificarla **volatile** nella dichiarazione.

Anche **senza detach/attach**, il codice rimane **thread-safe** grazie alla **logica di guardia** sulla variabile **pressed**. Non è necessario proteggere waitUntilInputChange() con interrupts()/noInterrupts(), infatti:
- Inizialmente pressed = false
- La ISR può attivarsi solo quando pressed è false, e lo imposta a true
- Una volta che pressed è true, la ISR non può più modificare nulla (per la condizione if(!pressed))
- Il loop processa l'evento e alla fine ripristina pressed = false

L'unica differenza pratica tra le due versioni (con e senza detach/attach) è che:
- **Con detach/attach**: l'interrupt viene proprio **disabilitato fisicamente**
- **Senza detach/attach**: l'interrupt può attivarsi ma la sua **ISR non fa nulla** (esce subito per la condizione su pressed)

Dalle considerazioni precedenti si deduce che è possibile eliminare l'**overhead** delle chiamate alle funzioni **attach()** e **detach()** utilizzando solamente il **flag pressed**. Il prezzo da pagare è un maggiore **overhead** della **ISR** che, a seguito di un interrupt, potrebbe essere chiamata anche un **centinaio** di volte consecutivamente.

- Simulazione online su ESP32 del codice precedente con Wowki: https://wokwi.com/projects/423543840234258433
- Simulazione online su ESP32 del codice precedente con Tinkercad: https://www.tinkercad.com/things/dyrIzrr7OlE-toggle-interrupt-no-blocking

Variante del codice precedente ottenuta utilizzando direttamente la sola funzione millis(), senza adoperare alcuna libreria di timer.

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
int led = 13;
byte pulsante =12;
byte stato= LOW;  // variabile globale che memorizza lo stato del pulsante
volatile bool pressed;
volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );  
  pressed = false;
}

// Interrupt Service Routine (ISR)
void switchPressed ()
{
 if(!pressed){// evita detach durante un attach nel loop
    // disarma l'interrupt
    // può essere omessa se viene tolto l'attach() nel loop()
    detachInterrupt(digitalPinToInterrupt(pulsante));
    // arma il timer
    lastDebounceTime = millis();
    pressed = true; 
 }
}  // end of switchPressed

void waitUntilInputChange()
{
    if (pressed){ 
      if(digitalRead(pulsante)==LOW && (millis() - lastDebounceTime) > debounceDelay){// disarmo del timer al timeout
        // fronte di discesa
        stato = !stato; // logica da attivare sul fronte (toggle)
        // riarma l'interrupt
        // può essere omessa se eviene tolto il detach() nella ISR
        attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING ); 
        pressed = false; // disarma il timer
      }
    }
}
// loop principale
void loop() {
	waitUntilInputChange();
	digitalWrite(led, stato);   	// inverti lo stato precedente del led
  delay(10);
}
```
La variabile a 32 bit ```lastDebounceTime``` in ESP32 è atomica per lettura, scrittura e confronto. Negli altri microcontrollori come Arduino no.

Analizziamo la questione dell'atomicità di lastDebounceTime in questo contesto:

```lastDebounceTime``` viene:
- Scritto SOLO nella ISR: ```lastDebounceTime = millis();```
- Letto SOLO nel loop: ```(millis() - lastDebounceTime) > debounceDelay```


La protezione delle sezioni critiche è garantita dalla variabile pressed:
- La ISR scrive ```lastDebounceTime``` solo quando ```pressed``` è ```false```
- Il loop legge ```lastDebounceTime``` solo quando ```pressed``` è ```true```

La sequenza è sempre:

```C++
ISR (quando pressed = false):
   lastDebounceTime = millis()
   pressed = true

Loop (quando pressed = true):
   legge lastDebounceTime
   ...eventualmente pressed = false
```

Quindi, anche se ```lastDebounceTime``` è un long (tipicamente 4 byte su Arduino) e la sua scrittura/lettura non è atomica, non c'è rischio di race condition perché:

Non c'è mai sovrapposizione temporale tra scrittura e lettura grazie alla **guardia pressed**. La ISR non può intervenire durante la lettura di lastDebounceTime nel loop perché:
- ```pressed``` è true e quindi la ISR esce subito
- ```pressed``` è false ma allora il loop non sta leggendo lastDebounceTime


Il codice è quindi corretto anche senza accesso atomico a ```lastDebounceTime```. La variabile ```volatile``` è sufficiente per garantire la visibilità delle modifiche tra ISR e loop.

L'unico caso teorico di problemi sarebbe se l'architettura non garantisse la coerenza dell'ordine delle scritture in memoria (memory ordering), ma su Arduino (AVR) questo non è un problema perché le scritture mantengono l'ordine di programma.

- Simulazione online su ESP32 del codice precedente con Wowki: https://wokwi.com/projects/423680930381449217
- Simulazione online su ESP32 del codice precedente con Tinkercad: https://www.tinkercad.com/things/0jUHZOIdhvP-toggle-interrupt-no-blocking-con-millis
  
### **Pulsante toggle basato su interrupts e debounce nella ISR**

Per una discussione generale sugli interrupt si rimanda a [interrupt](indexinterrupts.md).

Il **funzionamento** è agevolmente comprensibile alla luce delle seguenti considerazioni:
- L'**interrupt** è attivo su **un solo fronte**. 
- Al momento del **primo bounce in rising** del pulsante la condizione ```debtimer.get() > DEBOUNCETIMERISE``` è **sempre vera** (per cui il flag ```stato``` commuta immediatamente) perchè, dato che la funzione reset() del timer era stata eseguita molto tempo prima (al momento del rilascio del pulsante da parte dell'utente), il tempo attuale misurato dalla ```get()``` è molto più grande di un timeout. I bounce successivi al primo, in rising non possono più accadere (pulsante disarmato) dato che adesso l'attachInterrupt è impostata in FALLING, se in falling sono filtrati dal timer impostato nella callback ```debounceFall()``` che, sebbene diversa, viene chiamata temporalmente molto vicina al ```reset()``` di quella in rising precedente.
- Al momento del **primo bounce in falling** del pulsante la condizione ```debtimer.get() > DEBOUNCETIMEFALL``` è **sempre vera** (per cui il flag ```stato``` commuta immediatamente) perchè, dato che la funzione reset() del timer era stata eseguita molto tempo prima (al momento della pressione del pulsante da parte dell'utente), il tempo attuale misurato dalla ```get()``` è molto più grande di un timeout. I bounce successivi al primo, in falling non possono più accadere dato che adesso l'attachInterrupt è impostata in RISING (pulsante riarmato), se in rising sono filtrati dal timer impostato nella callback ```debounceRise()``` che, sebbene diversa, viene chiamata temporalmente molto vicina al ```reset()``` di quella in falling precedente.
  
Un esempio con l'**attuazione nel loop** del task di accenesione/spegnimento potrebbe essere:

```C++
#include "urutils.h"
const unsigned long DEBOUNCETIMERISE = 50;
const unsigned long DEBOUNCETIMEFALL = 50;
const byte ENGINE = 13;
const byte BUTTONPIN = 12;
volatile bool stato = false;
volatile int count1 = 0;
DiffTimer debtimer;

void debounceRise() {
  if (debtimer.get() > DEBOUNCETIMERISE) {// al primo bounce (in rise o in fall) è sempre vero!
    Serial.println(count1);
    count1 = 0;
    stato = !stato; // logica da attivare sul fronte (toggle)
    Serial.println("I have catched a RISE");
    attachInterrupt(digitalPinToInterrupt(BUTTONPIN), debounceFall, FALLING);// pulsante disarmato!
    debtimer.reset();// ogni tipo di fronte resetta il timer
  } else {
    count1++;
  }
}

void debounceFall() {
  if (debtimer.get() > DEBOUNCETIMEFALL) {// al primo bounce (in rise o in fall) è sempre vero!
    Serial.println(count1);
    count1 = 0;
    Serial.println("I have catched a FALL");
    attachInterrupt(digitalPinToInterrupt(BUTTONPIN), debounceRise, RISING);// pulsante riarmato!
    debtimer.reset();// ogni tipo di fronte resetta il timer
  } else {
    count1++;
  }
}

void setup ()
{
  Serial.begin(115200);
  pinMode(BUTTONPIN, INPUT);
  pinMode(ENGINE, OUTPUT);  	  // so we can update the LED
  digitalWrite(ENGINE, LOW);
  // attach interrupt handler
  debtimer.start();
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), debounceRise, RISING);
}  // end of setup

void loop ()
{
  //Serial.println(pressed);
  if (stato) {
    digitalWrite(ENGINE, HIGH);
  } else {
    digitalWrite(ENGINE, LOW);
  }
  delay(10);
}
```

Le variabili **condivise** tra una ISR e il loop() andrebbero protette, da accessi **paralleli** e concorrenti da parte di entrambe, tramite delle **corse critiche** che rendano l'accesso **strettamente sequenziale**. Inoltre le variabili condivise devono sempre essere dichiarate con il qualificatore ```volatile``` per forzarne la modifica istantanea anche sui registri della CPU. 

Gli **accessi paralleli** non sono un problema quando le **istruzioni** sono **atomiche**, cioè non interrompibili. Le istruzioni atomiche o sono eseguite per intero o non sono eseguite affatto. In questo caso gli **accessi**, sia in lettura che in scrittura, sono in realtà, a basso livello, **intrinsecamente sequenziali**.

Nei microcontrollori attuali, in genere **nessuna istruzione** gode della proprietà di essere **atomica** con una sola eccezione per la lettura e scrittura delle **variabili ad 8 bit**. Per le variabili codificate con **8 bit** l'accesso a basso livello (linguaggio macchina) è intrinsecamente garantito essere **atomico**. Per queste variabili rimane comunque la necessita dell'uso del qualificatore ```volatile```.

Le **modifiche** a valori con codifiche **maggiori di 8 bit** sono in genere **non atomiche**, pertanto le variabili a 16 o 32 bit andrebbero gestite con gli interrupt disabilitati (sezione critica). Tuttavia, gli interrupt vengono disabilitati di default durante una routine di servizio di interrupt, quindi, non potendo verificarsi il danneggiamento di una variabile multibyte in una ISR, le **sezioni critiche** vanno inserite soltanto nel ```loop()```.

Le variabili **condivise** tra ISR e loop() e **8 bit** sono ```stato``` e ```count1``` che sono state semplicemente dichiarate come ```volatile``` senza sezioni critiche su di essa.

- Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/388638737495929857
- Simulazione online su Arduino di una del codice precedente con Tinkercad: https://www.tinkercad.com/things/4YKrcH053EV-toggle-interrupt-con-debounce-in-isr

### **Pulsante toggle basato su interrupts e timer HW**

Per una discussione generale sugli interrupt si rimanda a [interrupt](indexinterrupts.md).

All'**ingresso** di una **porta digitale** viene associata una callback che viene invocata alla ricezione di un segnale di interrupt attivo su entrambi i fronti. Il fronte di salita, **selezionato** prendendo solo i valori HIGH, potrebbe essere rilevato molte volte consecutivamente a causa del fenomeno dei rimbalzi. Per evitare la rilevazione dei **fronti spuri** successivi al primo, viene disabilitata, dentro la ISR, la loro rilevazione **disarmando** gli interrupt  mediante l'istruzione ```detachInterrupt(digitalPinToInterrupt(pulsante))```. 

Il tempo per la **riabilitazione** (riarmo) dell'interrupt non deve essere ne troppo presto, cioè minore di 50 msec, altrimenti si finisce per leggere dei rimbalzi ma neppure troppo tardi, cioè dopo la pressione di un tasto, altrimenti si perdono degli input dell'utente. Il momento migliore per riabilitare gli interrupt potrebbe essere il momento del rilascio del pulsante, dato che precede sempre una eventuale successiva pressione. In ogni caso, un timer impedisce quei tentativi di riabilitazione che potrebbero avvenire prima dei 50 msec utili ad evitare i rimbalzi.

Il **timer** è di tipo **one shot** e viene riarmato solo se un polling della porta del tasto fornisce ancora **valore alto** (tasto premuto se in pull down). Se invece fornisce **valore basso**, non viene riarmato il timer ma viene riarmato al suo posto l'interrupt del tasto mediante l'istruzione  ```attachInterrupt() ```.

Allo scadere del timeout viene eseguita la callback ```waitUntilInputLow()``` all'interno della ISR del timer. La funzione esegue una nuova lettura del valore della porta:
- se è **HIGH** allora deduce che il pulsante è **ancora premuto** e decide di aspettare ancora altri 50 msec riavviando il timer di rilevazione del fronte di discesa
- se è **LOW** allore deduce che il pulsante è **stato rilasciato** e decide di riabilitarlo ad una nuova pressione (riarmo) disasserendo il fag di disabilitazione ```pressed```.

La funzione di **debouncing** è garantita introducendo un **tempo minimo** di attesa tra un campionamento e l'altro.

Per mantenere la ISR chiamante il più veloce possibile, viene spostato nel ```loop()```  l'algoritmo di blink basato sui ```delay()```, dove può fare il suo lavoro industurbato essendo l'unico task (suscettibile ai ritardi) presente. 

Le attese sono tutte **non bloccanti** e realizzate tramite un timer HW che adopera esso stesso gli **interrupt** per richiamare la funzione di servizio (callback) da eseguire allo scadere del **timeout**. Il timer, utilizzando gli interrupt, è in grado di intervenire in tempo in **tutte le situazioni**, eventualmente anche interrompendo l'esecuzione di istruzioni che impegnino intensamente il loop(). Si tratta sicuramente di una realizzazione che, avendo la massima efficacia possibile in tutte le situazioni, si presta alla realizzazione di **dispositivi di sicurezza**.

```C++
#include <Ticker.h>
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
int led = 13;
byte pulsante =12;
volatile bool stato;
#define DEBOUNCETIME 50
Ticker debounceTicker;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );
  stato = false;
}

void switchPressed ()
{
  detachInterrupt(digitalPinToInterrupt(pulsante));
  debounceTicker.once_ms(50, waitUntilInputLow);  
  Serial.println("SALITA disarmo pulsante");
  stato = !stato; 	 // logica da attivare sul fronte (toggle)

}  // end of switchPressed

void waitUntilInputLow()
{
    if (digitalRead(pulsante) == HIGH)//se coincide con il valore di un polling
    { 
        Serial.print("Aspetto");
        debounceTicker.once_ms(50, waitUntilInputLow);  
    }else{
        Serial.print("DISCESA riarmo pulsante\n");
        attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );
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
Di seguito il link della simulazione online con ESP32 su Wokwi: https://wokwi.com/projects/390289622147259393

### **Pulsante toggle basato su interrupts e con debounce basato sui delay()**

All'**ingresso** di una **porta digitale** viene associata una callback che viene invocata alla ricezione di un segnale di interrupt attivo su entrambi i fronti. Il fronte di salita, **selezionato** prendendo solo i valori HIGH, potrebbe essere rilevato molte volte consecutivamente a causa del fenomeno dei rimbalzi. 

Per evitare la rilevazione dei **fronti spuri** successivi al primo, viene disabilitata, dentro la ISR, la loro rilevazione **disarmando** gli interrupt  mediante l'istruzione ```detachInterrupt(digitalPinToInterrupt(pulsante))```. Contemporaneamente viene asserito un flag di segnalazione, ```pressed```, che comunica ad un loop() di attivare il timer per il riarmo dell'interrupt per rispondere a nuove pressioni dell'utente. 

Il tempo per la **riabilitazione** (riarmo) dell'interrupt non deve essere ne troppo presto, cioè minore di 50 msec, altrimenti si finisce per leggere dei rimbalzi ma neppure troppo tardi, altrimenti si perdono degli input dell'utente. Il momento migliore per riabilitare gli interrupt potrebbe essere il momento del rilascio del pulsante, dato che precede sempre una eventuale successiva pressione. In ogni caso, un timer impedisce quei tentativi di riabilitazione che potrebbero avvenire prima dei 50 msec utili ad evitare i rimbalzi.

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
#include "urutils.h"
int led = 13;
int pulsante =12;
volatile bool stato = false;  // variabile globale che memorizza lo stato del pulsante
volatile bool pressed;
#define DEBOUNCETIME 50
DiffTimer debounce;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  // arma l'interrupt
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );
  // disarma il timer 
  pressed = false;
}

// Interrupt Service Routine (ISR)
void switchPressed ()
{
  if(!pressed){// evita detach durante un attach nel loop
    // disarma l'interrupt
    detachInterrupt(digitalPinToInterrupt(pulsante));
    // arma il timer
    pressed = true; 
  }
    
}  // end of switchPressed

// loop principale
void loop() {
  if(pressed){// fronte di salita
    waitUntilInputLow(pulsante, 100); 
    // fronte di discesa
    stato = !stato; // logica da attivare sul fronte (toggle)
    // riarma l'interrupt
    attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING ); 
    pressed = false; // disarma il timer
  }
  digitalWrite(led, stato);   	// inverti lo stato precedente del led
  delay(10);
}
```

Simulazione online su ESP32 del codice precedente con Wowki: https://wokwi.com/projects/390288516762524673


### **Pulsante toggle basato su interrupts e timer debounce con timer SW get()**

All'**ingresso** di una **porta digitale** viene associata una callback che viene invocata alla ricezione di un segnale di interrupt attivo su entrambi i fronti. Il fronte di salita, **selezionato** prendendo solo i valori HIGH, potrebbe essere rilevato molte volte consecutivamente a causa del fenomeno dei rimbalzi. 

Per evitare la rilevazione dei **fronti spuri** successivi al primo, viene disabilitata, dentro la ISR, la loro rilevazione **disarmando** gli interrupt  mediante l'istruzione ```detachInterrupt(digitalPinToInterrupt(pulsante))```. Contemporaneamente viene asserito un flag di segnalazione, ```pressed```, che comunica ad un loop() di attivare il timer per il riarmo dell'interrupt per rispondere a nuove pressioni dell'utente. 

Il tempo per la **riabilitazione** (riarmo) dell'interrupt non deve essere ne troppo presto, cioè minore di 50 msec, altrimenti si finisce per leggere dei rimbalzi ma neppure troppo tardi, altrimenti si perdono degli input dell'utente. Il momento migliore per riabilitare gli interrupt potrebbe essere il momento del rilascio del pulsante, dato che precede sempre una eventuale successiva pressione. In ogni caso, un timer impedisce quei tentativi di riabilitazione che potrebbero avvenire prima dei 50 msec utili ad evitare i rimbalzi.

Variante che **disabilita** gli interrupt spuri fino al rilascio del pulsante: 

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
#include "urutils.h"
int led = 13;
byte pulsante =12;
byte stato= LOW;  // variabile globale che memorizza lo stato del pulsante
volatile bool pressed;
#define DEBOUNCETIME 50
DiffTimer debounce;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );  
  pressed = false;
}

// Interrupt Service Routine (ISR)
void switchPressed ()
{
 if(!pressed){// evita detach durante un attach nel loop
    // disarma l'interrupt
    detachInterrupt(digitalPinToInterrupt(pulsante));
    // arma il timer
    pressed = true; 
 }
}  // end of switchPressed

void waitUntilInputChange()
{
    if (pressed){ 
      debounce.start();// aggiorna il millis() interno solo alla prima di molte chiamate consecutive
      if(debounce.get() > DEBOUNCETIME  && digitalRead(pulsante) == LOW){// disarmo del timer al timeout
        debounce.stop(); // disarmo del timer
        debounce.reset();
        // fronte di discesa
        stato = !stato; // logica da attivare sul fronte (toggle)
        // riarma l'interrupt
        attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING ); 
        pressed = false; // riarmo del pulsante
      }
    }
}
// loop principale
void loop() {
	waitUntilInputChange();
	digitalWrite(led, stato);   	// inverti lo stato precedente del led
  delay(10);
}
```

Simulazione online su ESP32 del codice precedente con Wowki: https://wokwi.com/projects/423543840234258433

Variante che **ignora** gli interrupt spuri fino al rilascio del pulsante: 

All'**ingresso** di una **porta digitale** viene associata una callback che viene invocata alla ricezione di un segnale di interrupt attivo su entrambi i fronti. Il fronte di salita, **selezionato** prendendo solo i valori HIGH, potrebbe essere rilevato molte volte consecutivamente a causa del fenomeno dei rimbalzi. 

Per evitare la rilevazione dei **fronti spuri** successivi al primo, viene disabilitata, dentro la ISR, la loro rilevazione non disabilitando gli interrupt ma disabilitando la loro gestione  mediante la condizione ```if(!pressed)``` che, dopo il primo fronte, non è più soddisfatta. Il flag di segnalazione, ```pressed```, oltre a disabilitare la gestione degli interrupt, comunica ad un loop() di attivare il timer per il riarmo dell'interrupt per rispondere a nuove pressioni dell'utente. 

Il tempo per la **riabilitazione** (riarmo) dell'interrupt non deve essere ne troppo presto, cioè minore di 50 msec, altrimenti si finisce per leggere dei rimbalzi ma neppure troppo tardi, cioè dopo la pressione di un tasto, altrimenti si perdono degli input dell'utente. Il momento migliore per riabilitare gli interrupt potrebbe essere il momento del rilascio del pulsante, dato che precede sempre una eventuale successiva pressione. In ogni caso, un timer impedisce quei tentativi di riabilitazione che potrebbero avvenire prima dei 50 msec utili ad evitare i rimbalzi.

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
#include "urutils.h"
int led = 13;
byte pulsante =12;
byte stato= LOW;  // variabile globale che memorizza lo stato del pulsante
volatile bool pressed;
#define DEBOUNCETIME 50
DiffTimer debounce;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );  
  pressed = false;
}

// Interrupt Service Routine (ISR)
void switchPressed ()// si attiva sul fronte di salita
{
    if(!pressed){ // intervento immediato ma sul primo fronte di salita soltanto (causa disarmo pulsante)
        pressed = true;// disarmo del pulsante e riarmo del timer di debouncing
        stato = !stato;// logica da attivare sul fronte (toggle)
    }
}  // end of switchPressed

void waitUntilInputChange()
{
    if (pressed){ 
      debounce.start();// aggiorna il millis() interno solo alla prima di molte chiamate consecutive
      if(debounce.get() > DEBOUNCETIME  && digitalRead(pulsante) == LOW){
        pressed = false; // riarmo del pulsante e disarmo del timer di debouncing
        debounce.stop();
	      debounce.reset();
      }
    }
}
// loop principale
// loop principale
void loop() {
	waitUntilInputChange();
	digitalWrite(led, stato);   	// inverti lo stato precedente del led
  delay(10);
}
```

Simulazione online su ESP32 del codice precedente con Wowki: https://wokwi.com/projects/388481409829351425

### **Pulsante toggle basato su interrupts e debounce nella ISR**

Per una discussione generale sugli interrupt si rimanda a [interrupt](indexinterrupts.md).

Il **funzionamento** è agevolmente comprensibile alla luce delle seguenti considerazioni:
- L'**interrupt** è attivo su **un solo fronte**. 
- Al momento del **primo bounce in rising** del pulsante la condizione ```debtimer.get() > DEBOUNCETIMERISE``` è **sempre vera** (per cui il flag ```stato``` commuta immediatamente) perchè, dato che la funzione reset() del timer era stata eseguita molto tempo prima (al momento del rilascio del pulsante da parte dell'utente), il tempo attuale misurato dalla ```get()``` è molto più grande di un timeout. I bounce successivi al primo, in rising non possono più accadere (pulsante disarmato) dato che adesso l'attachInterrupt è impostata in FALLING, se in falling sono filtrati dal timer impostato nella callback ```debounceFall()``` che, sebbene diversa, viene chiamata temporalmente molto vicina al ```reset()``` di quella in rising precedente.
- Al momento del **primo bounce in falling** del pulsante la condizione ```debtimer.get() > DEBOUNCETIMEFALL``` è **sempre vera** (per cui il flag ```stato``` commuta immediatamente) perchè, dato che la funzione reset() del timer era stata eseguita molto tempo prima (al momento della pressione del pulsante da parte dell'utente), il tempo attuale misurato dalla ```get()``` è molto più grande di un timeout. I bounce successivi al primo, in falling non possono più accadere dato che adesso l'attachInterrupt è impostata in RISING (pulsante riarmato), se in rising sono filtrati dal timer impostato nella callback ```debounceRise()``` che, sebbene diversa, viene chiamata temporalmente molto vicina al ```reset()``` di quella in falling precedente.
  
Un esempio con l'**attuazione nel loop** del task di accenesione/spegnimento potrebbe essere:

```C++
#include "urutils.h"
const unsigned long DEBOUNCETIMERISE = 50;
const unsigned long DEBOUNCETIMEFALL = 50;
const byte ENGINE = 13;
const byte BUTTONPIN = 12;
volatile bool stato = false;
volatile int count1 = 0;
DiffTimer debtimer;

void debounceRise() {
  if (debtimer.get() > DEBOUNCETIMERISE) {// al primo bounce (in rise o in fall) è sempre vero!
    Serial.println(count1);
    count1 = 0;
    stato = !stato; // logica da attivare sul fronte (toggle)
    Serial.println("I have catched a RISE");
    attachInterrupt(digitalPinToInterrupt(BUTTONPIN), debounceFall, FALLING);// pulsante disarmato!
    debtimer.reset();// ogni tipo di fronte resetta il timer
  } else {
    count1++;
  }
}

void debounceFall() {
  if (debtimer.get() > DEBOUNCETIMEFALL) {// al primo bounce (in rise o in fall) è sempre vero!
    Serial.println(count1);
    count1 = 0;
    Serial.println("I have catched a FALL");
    attachInterrupt(digitalPinToInterrupt(BUTTONPIN), debounceRise, RISING);// pulsante riarmato!
    debtimer.reset();// ogni tipo di fronte resetta il timer
  } else {
    count1++;
  }
}

void setup ()
{
  Serial.begin(115200);
  pinMode(BUTTONPIN, INPUT);
  pinMode(ENGINE, OUTPUT);  	  // so we can update the LED
  digitalWrite(ENGINE, LOW);
  // attach interrupt handler
  debtimer.start();
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), debounceRise, RISING);
}  // end of setup

void loop ()
{
  //Serial.println(pressed);
  if (stato) {
    digitalWrite(ENGINE, HIGH);
  } else {
    digitalWrite(ENGINE, LOW);
  }
  delay(10);
}
```

Le variabili **condivise** tra una ISR e il loop() andrebbero protette, da accessi **paralleli** e concorrenti da parte di entrambe, tramite delle **corse critiche** che rendano l'accesso **strettamente sequenziale**. Inoltre le variabili condivise devono sempre essere dichiarate con il qualificatore ```volatile``` per forzarne la modifica istantanea anche sui registri della CPU. 

Gli **accessi paralleli** non sono un problema quando le **istruzioni** sono **atomiche**, cioè non interrompibili. Le istruzioni atomiche o sono eseguite per intero o non sono eseguite affatto. In questo caso gli **accessi**, sia in lettura che in scrittura, sono in realtà, a basso livello, **intrinsecamente sequenziali**.

Nei microcontrollori attuali, in genere **nessuna istruzione** gode della proprietà di essere **atomica** con una sola eccezione per la lettura e scrittura delle **variabili ad 8 bit**. Per le variabili codificate con **8 bit** l'accesso a basso livello (linguaggio macchina) è intrinsecamente garantito essere **atomico**. Per queste variabili rimane comunque la necessita dell'uso del qualificatore ```volatile```.

Le **modifiche** a valori con codifiche **maggiori di 8 bit** sono in genere **non atomiche**, pertanto le variabili a 16 o 32 bit andrebbero gestite con gli interrupt disabilitati (sezione critica). Tuttavia, gli interrupt vengono disabilitati di default durante una routine di servizio di interrupt, quindi, non potendo verificarsi il danneggiamento di una variabile multibyte in una ISR, le **sezioni critiche** vanno inserite soltanto nel ```loop()```.

Le variabili **condivise** tra ISR e loop() e **8 bit** sono ```stato``` e ```count1``` che sono state semplicemente dichiarate come ```volatile``` senza sezioni critiche su di essa.

- Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/388638737495929857

### **PULSANTE DI SICUREZZA CON DEBOUNCER BASATO SUI DELAY SU UN THREAD A PARTE**

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
