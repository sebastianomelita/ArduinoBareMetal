
>[Torna all'indice generale](index.md)

## **CONSIDERAZIONI GENERALI SULLA GESTIONE DEI TASK**

<img src="img\applicazMicro.jpg" alt="alt text" width="1000">

La logica degli algoritmi può essere pensata essenzialmente in due maniere: ad **eventi** e in modo **sequenziale**. 

Nei microcontrollori il **tempo** è un elemento portante della maggiorparte degli algoritmi. Il modo **sequenziale** di pensare il tempo in un algoritmo è sicuramente quello più intuitivo ed ha una sua naturale **corrispondenza** con le istruzioni di un programma. 

Infatti, in un listato sorgente, le istruzioni che **spazialmente** vengono dopo, sono eseguite dopo anche **temporalmente** (se non ci sono stati salti). Quantificare il **dopo** nel tempo tra una istruzione e l'altra è proprio il compito assegnato all'istruzione ```delay(x```), che non fa altro che bloccare l'esecuzione del loop principale (come di un qualsiasi loop secondario basato sui thread) per ```x``` millisecondi.

Finchè un programma consiste in un **unico task** avente un flusso temporale unico va tutto bene. Il modello lineare di programmazione è chiaro, intuitivo ed adeguato. Quando però, in un algoritmo coesistono più task separati, ciascuno composto da un proprio filo temporale che si interseca con quello degli altri, allora, il problema di codificarlo in un programma si complica. In questo caso, è quindi dirimente sapere se i task di cui si compone l'algoritmo sono **sequenziali** o procedono in maniera **parallela**, e se si, stabilire se, quando si incrociano, un task deve **aspettarne** qualcun altro prima di proseguire.

In linea di principio, se i task sono **sequenziali** nulla osta a metterli tutti nello **stesso loop** principale eseguendoli uno **di seguito** all'altro, dato che il loro sviluppo fisico nel codice è anche il loro sviluppo temporale.
sarà 
Se, invece, più task sono **paralleli**, allora sviluppare la loro codifica in un unico flusso di esecuzione necessita il coordinamento dei ritardi di tutti. Per ottenere ciò una strategia potrebbe essere stabilire il **massimo comune divisore** dei tempi che compaiono nei ```delay()```. Se questo MCD esiste, diventa il **tempo base** di tutti i task, cioè quello mediante il quale è possibile **programmare** i tempi dei vari task **incastrandoli** l'uno nell'altro. 

Ad esempio, posso far accadere ogni 2 tempi base il blink di un led rosso e ogni 3 quello di un led verde semplicemente dividendo il tempo in **slot** tramite un delay che vale esattamente un tempo base e scrivendo, per ogni slot, l'istruzione che **compete** ad ogni task. Se un task in uno slot non fa nulla allora li non compare alcuna sua istruzione (vedi Es2).

Quando, però, i task concorrenti diventano **tanti** od oppure sono **complicati** allora potrebbe risultare tedioso progettarli e realizzarli tutti insieme interlacciandoli nel tempo. Un algoritmo siffatto può essere ancora più difficile da realizzare se accade che uno o più task non sono proprio del tutto indipendenti, ma si aspettano l'uno con l'altro in certi istanti.

In questo caso le opzioni sono: o cambiare radicalmente approccio adoperando uno stile di programmazione **ad eventi**, perdendo il benefit della intuitività della programmazione sequenziale, oppure mantenere lo stile **lineare e sequenziale** del programma rendendo i task **indipendenti**, non solo concettualmente ma anche nel loro **flusso di esecuzione**.

Se scegliamo la seconda opzione, task indipendenti vengono mandati in esecuzione su **loop indipendenti** che procedono separatamente ma contemporaneamente, **in parallelo** l'uno con l'altro. I flussi di esecuzione paralleli ed indipendenti vengono chiamati **thread** o **processi**, a seconda dei casi. Noi useremo **solamente i thread**. Per una trattazione più completa della teoria dei thread e di come questi possono essere adoperati nel codice vedi [thread](threadsched.md).

Questo risultato è possibile ottenerlo se il microcontrollore ha la capacità di eseguire più flussi paralleli di codice e ciò è sempre possibile se a bordo è installato un **sistema operativo**. Il microcontrollore **ESP32** ha un su SO mentre **Arduino** no, per cui si possono usare le API dei thread nel codice di un programma solo su ESP32 e non su Arduino.

In **definitiva**, **non** adoperando, per adesso, tecniche di programmazione **ad eventi** dobbiamo limitarci a quelle **sequenziali** che hanno comunque il pregio di essere intrinsecamente più **intuitive** rispetto a quelle ad eventi per una larga categoria di problemi. Le **linee guida** per mantenere uno stile di programmazione **lineare** potrebbero essere:
- se i task procedono in **maniera sequenziale**, cioè, uno dopo l'altro, allora si possono pianificare **indipendentemente** mettendoli fisicamente uno di seguito all'altro all'**interno di uno stesso loop**.
- se i task procedono in maniera **parallela** e indipendente allora si possono pianificare in modo **indipendente** soltanto se si mandano in esecuzione su **thread separati**, all'interno di un **loop() dedicato**, assegnato solamente a loro.
- se i task procedono in maniera **parallela** ma non del tutto indipendente, perchè capita che uno rimane in attesa del risultato dell'altro prima di andare avanti, allora bisogna trovare il modo di **sincronizzarli** con delle istruzioni di **attesa**, non fisse come le ```delay()```, ma dinamiche e legate all'accadere di un certo evento. Un evento è sempre segnalato dalla verità di una certa **condizione**, valutabile da una istruzione come ```waitUntil(c)```, dove ```c``` è la condizione.

<img src="img\task.jpg" alt="alt text" width="1000">

## **Attivazione di una logica qualsiasi sul fronte di salita**

```C++
// APPROCCIO CON MILLIS() (NON BLOCCANTE)                  | // APPROCCIO CON DELAY() (BLOCCANTE)
                                                           |
unsigned long precm = 0;                                   | unsigned long precm = 0;
const unsigned long tbase = 50; // millisecondi            | const unsigned long tbase = 50; // millisecondi 
int val, precval = LOW;                                    | int val, precval = LOW;  
int stato, nuovoStato;                                     | int stato, nuovoStato;  
int P1 = 2; 			                           | int P1 = 2;   
                                                           |    
							   |	 
void loop() {                                              |   
  if((millis()-precm) >= tbase) {    			   |   
    precm = millis();  					   | void loop() {
                                                           |    // Lettura diretta del pulsante senza schedulazione
    val = digitalRead(P1);  			           |    val = digitalRead(P1);  
                                                           | 
    if(precval==LOW && val==HIGH) { /* fronte di salita*/  |    if(precval==LOW && val==HIGH) { 
      stato = nuovoStato;/*impostazione del nuovo stato*/  |       stato = nuovoStato; // impostazione del nuovo stato
      updateOutputsInP1(stato); /*scrittura uscite*/       |       updateOutputsInP1(stato); 
    }                                                      |    }
    precval=val; /*memorizza livello loop precedente*/     |    precval=val;  
  }                                                        |    
                                                           |    delay(50);  // debouncer
  // Qui il programma può eseguire altre operazioni        | }
  // mentre attende che passi il tempo tbase               | 
}                                                          |
```

- **```precval```**: rappresenta il valore dell'**ingresso** campionato al **timeout precedente**
- **```val```**: rappresenta il valore dell'**ingresso** campionato al **timeout corrente**. Se val è **diverso** da precval allora ho rilevato un **fronte** tra il valore attuale e il valore al tempo di riferimento (timeout precedente).
- **```precm```**: rappresenta il valore del **tempo** al **timeout**. E' **misurato** con ```millis()``` ed è il tempo di **riferimento** della condizione per determinare il **successivo timeout**.
- **```(millis()-precm)```**: rappresenta il tempo trascorso dall'ultimo timeout, cioè dall'ultimo riferimento temporale noto.
- **```(millis()-precm) >= tbase```**: è la condizione di rilevazione del **nuovo timeout**
- **```tbase```**: è il tempo tra un timeout e l'altro
- il **corpo dell'if**, le istruzioni contenute nel blocco then dell'if, viene esegito **periodicamente** ad ogni timeout. Gli eventi che si estinguono tra un timeout e l'altro non possono essere rilevati (rimbalzi).

## **Attivazione di una logica qualsiasi su un fronte con timer SW**

```C++
// APPROCCIO CON MILLIS() - FRONTE SALITA                  | // APPROCCIO CON MILLIS() - FRONTE DISCESA
                                                           |
                                                           | 
const unsigned long tbase = 50; // millisecondi            | const unsigned long tbase = 50; // millisecondi
int val, precval = LOW;                                    | int val, precval = HIGH;  
int stato, nuovoStato;                                     | int stato, nuovoStato;
int P1 = 2;                                         	   | int P1 = 2;
DiffTimer deb;                                             | DiffTimer deb;
                                                           | 
void loop() {                                              | void loop() {
  if(deb.get() >= tbase) {                                 |   if(deb.get() >= tbase) {
    deb.reset();                                           |     deb.reset();
                                                           |     
    val = digitalRead(P1);                          	   |     val = digitalRead(P1);
                                                           | 
    if(precval==LOW && val==HIGH) {                        |     if(precval==HIGH && val==LOW) { 
      stato = nuovoStato;                                  |       stato = nuovoStato; 
      updateOutputsInP1(stato);                            |       updateOutputsInP1(stato); 
    }                                                      |     }
    precval=val;                                           |     precval=val;  
  }                                                        |   }
                                                           | 
  // Altre operazioni possibili                            |   // Altre operazioni possibili
}                                                          | }
```

- **```precval```**: rappresenta il valore dell'**ingresso** campionato al **timeout precedente**
- **```val```**: rappresenta il valore dell'**ingresso** campionato al **timeout corrente**. Se val è **diverso** da precval allora ho rilevato un **fronte** tra il valore attuale e il valore al tempo di riferimento (timeout precedente).
- Il valore del **tempo** al **timeout** è **campionato** (internamente al timer) al momento della chiamta a ```deb.reset()``` ed è il tempo di **riferimento** della condizione per determinare il **successivo timeout**.
- **```deb.get()```**: rappresenta il tempo trascorso dall'ultimo timeout, cioè dall'ultimo riferimento temporale noto.
- **```deb.get() >= tbase```**: è la condizione di rilevazione del **nuovo timeout**
- **```tbase```**: è il tempo tra un timeout e l'altro
- il **corpo dell'if**, le istruzioni contenute nel blocco then dell'if, viene esegito **periodicamente** ad ogni timeout. Gli eventi che si estinguono tra un timeout e l'altro non possono essere rilevati (rimbalzi).

## **Attivazione immediata di una logica qualsiasi su un fronte**

```C++
// APPROCCIO CON TIMER - FRONTE SALITA                     | // APPROCCIO CON TIMER - FRONTE DISCESA
                                                           |
                                                           | 
int stato, nuovoStato;                                     | int stato, nuovoStato;
int P1 = 2;                                         	   | int P1 = 2;
DiffTimer deb;                                             | DiffTimer deb;
                                                           | 
void loop() {                                              | void loop() {
  if (digitalRead(P1)) {                                   |   if (!digitalRead(P1)) {
    if(!debt.timerState){                                  |     if(!debt.timerState){
      debt.start();                                        |        debt.start();
      doOnRise();                                          |        doOnFall();
    }                                                      |      }
  } else if(debt.get() > 50) {                             |   } else if(debt.get() > 50) { // disarmo del timer al timeout
    debt.stop(); // disarmo del timer                      | 	 debt.stop(); // disarmo del timer
    debt.reset();                                          |     debt.reset();
    doOnFall();                                            |     doOnRise();                                
    // Altre operazioni possibili                          |     // Altre operazioni possibili
 }                                                         |   }    
}                                                          | }    
```
- a differenza della soluzione precedente, la rilevazione del **primo fronte** è immediata, dato che non deve attendere mai il periodo stabilito da un timer.
- la rilevazione del **secondo fronte** (opposto al precedente) è invece possibile che venga **differita** se questo accade prima dello scadere del tempo minimo di garanzia necessario per mettere la rilevazione al riparo dal fenomeno dei rimbalzi.
- **```deb.get()```**: rappresenta il tempo trascorso dall'ultimo timeout, cioè dall'ultimo riferimento temporale noto.
- **```50 msec```**: è il tempo tra un timeout e l'altro
- il **corpo dell'if**, le istruzioni contenute nel blocco then dell'if, viene esegito **periodicamente** ad ogni timeout. Gli eventi che si estinguono tra un timeout e l'altro non possono essere rilevati (rimbalzi).


## **Attivazione di una logica qualsiasi su un fronte con waitUntil()**

E' un approccio bloccante che però è molto pratico per la realizzazione di pulsanti con memoria. E' opportuno adoperare questo pattern insieme ad altri task solo se questi sono ad esso sequenziali. Se devono essere eseguiti, in parallelo alla gestione del pulsante, altri task allora è opportuno utilizzare una soluzione non bloccante, oppure isolare i task che devono procedere in parallelo su ```loop()``` **a parte**, realizzati, ad esempio, mediante **timer HW** o **threads**.

```C++
// ATTIVAZIONE SUL FRONTE DI DISCESA  (PULL DOWN)      | // ATTIVAZIONE SUL FRONTE DI SALITA (PULL UP)
-------------------------------------------------------|------------------------------------------------------
int stato, nuovoStato;                                 | int stato, nuovoStato;
int P1 = 2;                                            | int P1 = 2;
doOnFall(){                                            | doOnRise(){
   stato = nuovoStato;                                 |     stato = nuovoStato;
   updateOutputsInP1(stato);                                 updateOutputsInP1(stato);   
}                                                      | }
// loop principale                                     | // loop principale
void loop() {                                          | void loop() {
  if(digitalRead(P1) == HIGH){                         |   if(digitalRead(P1) == LOW){    
    doOnRise();                                        |     doOnFall();                        
    waitUntilInputLow(P1, 50);                         |     waitUntilInputHigh(P1, 50);   
    doOnFall();                                        |     doOnRise();  
  }                                                    |   }                                               
}                                                      | }
                                                       |
// Attivazione quando il pulsante                      | // Attivazione quando il pulsante
// viene RILASCIATO (fronte di discesa)                | // viene RILASCIATO (fronte di salita)
```

Ecco una scheda di **confronto** tra i due **approcci** per attivare una **logica** su **fronti diversi**:

Nella **colonna sinistra**:

- Codice che attiva la **logica** sul fronte di **DISCESA**
- Rileva prima quando il pulsante è premuto (HIGH)
- Esegue ```doOnRise()``` quando il pulsante viene premuto. E' **opzionale** e rappresenta quelle operazioni da fare **una sola volta** sul fronte di **salita** (pressione del pulsante), ad esempio, attivazione o reset di un timer.
- Attende con anti-rimbalzo che il pulsante venga **rilasciato** (LOW)
- **Cambia lo stato** quando il pulsante viene RILASCIATO
- ```updateOutputsInP1(stato)```, è eseguito una **sola volta**, quando il pulsante viene rilasciato. Serve a calcolare il **valore delle uscite** in funzione del valore della coppia (**ingresso, stato**).

Nella **colonna destra**:

- Codice che attiva la **logica** sul fronte di **SALITA**
- Rileva prima quando il pulsante è rilasciato (LOW)
- Esegue ```doOnFall()``` quando il pulsante è rilasciato. E' **opzionale** e rappresenta quelle operazioni da fare **una sola volta** sul fronte di **discesa** (pressione del pulsante), ad esempio, attivazione o reset di un timer.
- Attende con anti-rimbalzo che il pulsante venga **premuto** (HIGH)
- **Cambia lo stato** quando il pulsante viene PREMUTO
- ```updateOutputsInP1(stato)```, è eseguito una **sola volta**, quando il pulsante viene rilasciato. Serve a calcolare il **valore delle uscite** in funzione del valore della coppia (**ingresso, stato**).

La differenza fondamentale è nel momento in cui viene modificato lo stato: nella versione di sinistra avviene quando il pulsante viene rilasciato, mentre in quella di destra quando viene premuto.


## **TIMER UR**

E’ possibile realizzare dei timers, con cui programmare **nel futuro** lo stesso evento o una sequenza di eventi diversi, **senza attese**, cioè senza ```delay()```, eseguendo un polling del **tempo di conteggio** di un **timer**. 

Un **polling** è l'interrogazione periodica di una **variabile** o di un **ingresso** per leggerne il **valore**. Se il valore è maggiore di un **tempo massimo** allora si considera avvenuto il **timeout** e si esegue il **blocco** di codice associato ad esso.

Il polling serve per controllare, ad ogni loop(), il tempo trascorso (**elapsed**) per verificare **quando** questo raggiunge il **valore finale** (timeout) stabilito come obiettivo (target). 

Ciò si può ottenere eseguendo il **test** periodico di una **condizione** su una funzione di **misura** del tempo, nello specifico una ```get()```, che, ad ogni ```loop()```, restituisce il **tempo di conteggio**. Se la **condizione di uscita** dal conteggio è **vera** allora il **blocco** di istruzioni associato al **timeout del timer** viene **eseguito**.

Un **timeout** si controlla, valutando la **condizione di scadenza** del timer sulla funzione ```get()``` mediante una istruzione di selezione **if**. Ad esempio, eseguendo periodicamente (**polling**) nel ```loop()``` il controllo ```if(t.get() > 10000) {....}```, si può stabilire se, dal momento dell'attivazione del timer, sono pasasti 10 secondi e, in caso affermativo, eseguire le istruzioni nel blocco then che nel codice segue la condizione di scadenza.

La funzione ```get()```  **non è bloccante** (non causa alcun ritardo) e  **non interferisce** con nessun delay del loop corrente, sia esso il loop principale o quello secondario di un thread. Essa rappresenta, quindi, un **task** che si può tranquillamente adoperare in **sequenza** ad altri task di uno stesso loop, anche se questi contengono dei ritardi ```delay()```.

Il **polling** della funzione ```get()``` può essere effettuato in qualunque punto del loop, anche più volte se necessario, in genere si esegue per:
- se il timer **è attivo**, controllare una **cadenza**, cioè vedere se è trascorso il tempo **giusto** (periodo) per compiere una certa **azione** regolare (modificare una variabile o chiamare una funzione). La **periodicità** del timer può essere ottenuta invocando in sequenza due funzioni ```get() e reset()```. La **prima** serve a rilevare lo scadere del periodo e la **seconda** per far ripartire il timer da questo momento. 
- se il timer **è attivo**, controllare una **scadenza**, cioè vedere se è trascorso il tempo **massimo** (timeout) per compiere una certa **azione** prima che questa venga inibita. Il blocco del timer dopo una operazione una tantum (non periodica) può essere ottenuto invocando due funzioni ```get() e stop()``` consecutive. La **prima** serve a rilevare lo scadere del timeout e la **seconda** per far disattivare (disarmare) il timer a partire da questo momento. 
- se il timer **non è attivo**, memorizzare una **misura di tempo**, cioè quell'intervallo temporale che è stato **fissato** su un certo tempo con uno ```stop()```. Dopo si potrebbe utilizzare quel valore di tempo misurato, ad esempio, per impostare un **ritardo** variabile con una istruzione ```delay(t.get())```, per trasformarlo in una grandezza fisica **proporzionale** a quel tempo mediante la funzione ```map()```, oppure per confrontarlo con altri tempi che si stanno **accumulando** all'interno del loop.  

Si tratta di un **pattern** (tipo di API) per la realizzazione di timers **molto comune** nella programmazione di **bracci robotici** per uso industriale (vedi bracci Universal Robots) che si adatta bene ad essere impiegato sia in un contesto in cui la logica dell'algoritmo è realizzata in maniera sequenziale sia in quello in cui gli input sono gestiti con un modello ad eventi.

**In** **generale**, possiamo individuare alcune **componenti del timer**:

- **reset** del timer. Accade al verificarsi di una certa **condizione**. Determina sia l'azzeramento del **tempo di conteggio** che l'azzeramento della misura del ritardo dall'ultimo riavvio. L'azzeramento **non** è mai automatico ed è necessario impostarlo prima del riutilizzo del timer dopo uno o più comandi ```stop()``` (tipicamente al momento della valutazione della condizione di timeout) altrimenti è sufficiente richiamare la funzione ```start()```.  
- **start**. Avvia (arma) o riavvia (riarma) il timer quando si avvera la **condizione di attivazione** posta in punto qualsiasi del loop. Il suo scopo è anche quello di memorizzare il **tempo iniziale** di riferimento a partire dal quale la ```get()``` misurerà il **tempo trascorso** (o elapsed). Chiamate successive alla funzione ```start()``` non modificano questo **tempo iniziale** perchè questo viene marcato come già misurato da un **flag singleton** di guardia che si assicura che la chiamata alla sottostante funzione ```millis()``` non avvenga più di **una volta**. 
- **stop** del timer (disarmo). Accade al verificarsi di una certa **condizione** di sospensione del timer posta in punto qualsiasi del loop. In questa occasione il timer **campiona** il ritardo accumulato dall'ultimo riavvio e lo somma al **tempo di conteggio** cumulato in corrispondenza della sospensione precedente. In questo modo il tempo trascorso è memorizzato e può essere valutato in un momento successivo nel codice tramite la funzione ```get()```.
- **get** del **tempo di conteggio** (elapsed) dal momento del **primo avvio** del timer. E' una funzione **non bloccante**, cioè non genera attese dato che restituisce immediatamente il suo risultato. Viene **interrogata** periodicamente (polling) ad ogni ciclo di **loop** (meno se decimato ogni tot loop) e serve a realizzare una espressione di confronto con un tempo di **timeout** (```t1.get() > timeout```). Il **valore restituito** dipende dallo stato del timer:
     - se **```timerState == false```** allora restituisce l'ultimo **tempo di conteggio** misurato e memorizzato al momento della chiamata a stop() .
     - se **```timerState == true```** allora calcola il **tempo di conteggio** corrente campionando il ritardo attuale dall'ultimo riavvio e sommandolo ai ritardo cumulato prima dell'ultima sospensione.
- **istruzioni triggerate** (scatenate) dal timer. Vengono eseguite in base al **tempo di conteggio misurato** dal timer. Vengono eseguite in maniera sequenziale in un punto stabilito del loop (istruzioni sincrone) in corrispondenza della **verità** di una certa **condizione** sul tempo di conteggio che coinvolge la funzione ```get()``` come operando (ad es. ```t1.get() > 5000```). Contestualmente a queste si potrebbero eseguire :
  -    **reset()** per azzerare il timer **lasciandolo attivo**. Si usa per **schedulare nuovi timeout** dal momento del reset o per **rischedulare lo stesso timeout** periodicamente. 
  -    **stop()** per bloccare (disarmare) il timer, **congelando il tempo** trascorso fino allo stop. Un **nuovo start()**, senza reset(),  ricomincia dal tempo congelato.

Per consultare dettagli sulla sua implementazione vedi [Timer basati sul polling della millis()](polledtimer_seq.md).

## **PULSANTI CON MEMORIA E SENZA MEMORIA**

<img src="img\pulsanti.jpg" alt="alt text" width="1000">

### **CONTEGGIO DEL NUMERO DI PRESSIONI DI UN TASTO**

```C++
/*il contatore di pressioni si può fare eseguendo ad ogni pressione
 (o rilascio) entrambe le cose seguenti:
il riarmo di un timer con t1.start() 
(tanto è attivo solo alla prima volta)
l'incremento del contatore
Allo scadere del timer viene eseguito il conteggio:
 se minore di uno, scelta A; se maggiore di 1, scelta B.*/

#include "urutils.h"
int led_rosso=13;
int led_verde=12;
int pulsante1=14;
int count1=0;
DiffTimer t1;

void setup(){
  pinMode(pulsante1, INPUT);
  pinMode(led_rosso, OUTPUT);
  pinMode(led_verde, OUTPUT);
  Serial.begin(115200);
}

void loop(){
  if(digitalRead(pulsante1)==HIGH)
  {
    waitUntilInputLow(pulsante1, 50); //fronte di discesa
    t1.start();
    count1++;
  }
  if(t1.get()>2000)
  {
    t1.reset();
    if(count1<2)
    {
      Serial.print(count1 );
      Serial.println(" scelta A");
      digitalWrite(led_rosso, HIGH);
      digitalWrite(led_verde, LOW);
    }
    else
    {
      Serial.print(count1 );
      Serial.println(" scelta B");
      digitalWrite(led_rosso, LOW);
      digitalWrite(led_verde, HIGH);
    }
    count1=0;
    t1.stop();
  }
  delay(10);
}
```

- Simulazione online su Esp32 con Wowki del codice precedente: https://wokwi.com/projects/422697318232000513
- Simulazione online su Arduino con Tinkercad del codice precedente: https://www.tinkercad.com/things/biMU3UbIs4G-conta-pressioni-per-scelta-opzioni


### **CONTEGGIO DEL TEMPO DI PRESSIONE DI UN TASTO  (BLOCCANTE)**

 La quantità di zucchero si può scegliere facendo  partire un timer alla pressione di un pulsante e facendolo stoppare al suo rilascio. 
 
 La lettura del get() del timer mi da la misura della durata della pressione in millisecondi.

 La funzione ```map(t1.get(), 0, timeout, 0, ledCount)``` trasforma la durata misurata, compresa tra 0 e il timeout, nel valore di una quantità qualsiasi compresa tra 0 e ledCount.
 
 ```C++
/*
Sceglie lo zucchero il base al tempo della 
pressione e si vuole aumentare la 
luminosità di una led bar da 0 a 10.

NB2: La quantità di zucchero si può scegliere facendo
 partire un timer alla pressione di un pulsante e 
 facendolo stoppare al suo rilascio. 
 La lettura del get() del timer mi da la misura 
 del ritardo. Decidendo che sopra 10 secondi 
 le misure valgono sempre 10 (tetto superiore), 
 un valore da 0 a 10 dice la quantità di zucchero 
 da stampare con Serial.println(). 
*/

#include "urutils.h"
int led_rosso=13;
int led_verde=12;
int led_3=5;
int pulsante1=14;
const int ledCount = 10;
const int timeout = 2000;
DiffTimer t1;
int ledPins[] = {
  27, 15, 2, 4, 5, 18, 19, 21, 22, 23
};   // an array of pin numbers to which LEDs are attached

void paintBar(int ledLevel){
  // loop over the LED array:
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    // if the array element's index is less than ledLevel,
    // turn the pin for this element on:
    if (thisLed < ledLevel) {
      digitalWrite(ledPins[thisLed], HIGH);
    }
    // turn off all pins higher than the ledLevel:
    else {
      digitalWrite(ledPins[thisLed], LOW);
    }
  }
}
void setup(){
  // loop over the pin array and set them all to output:
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    pinMode(ledPins[thisLed], OUTPUT);
  }
  pinMode(pulsante1, INPUT);
  Serial.begin(115200);
}

void loop(){
  if(digitalRead(pulsante1)==HIGH)
  {
    t1.reset();
    t1.start();
    paintBar(0);
    waitUntilInputLow(pulsante1, 50); //fronte di discesa
    t1.stop();
    int level = map(t1.get(), 0, timeout, 0, ledCount);
    Serial.println(level);
    paintBar(level);
  }
  delay(10);
}
```

- Simulazione online su Esp32 con Wowki del codice precedente: https://wokwi.com/projects/422865809533329409
- Simulazione online su Esp32 con Tinkercad del codice precedente: https://www.tinkercad.com/things/5wJTBdL726e-durata-pressione-di-un-pulsante

### **CONTEGGIO DEL TEMPO DI PRESSIONE DI UN TASTO (NON BLOCCANTE)**

 La quantità di zucchero si può scegliere facendo  partire un timer alla pressione di un pulsante e facendolo stoppare al suo rilascio. 
 
 La lettura del get() del timer mi da la misura della durata della pressione in millisecondi.

 La funzione ```map(t1.get(), 0, timeout, 0, ledCount)``` trasforma la durata misurata, compresa tra 0 e il timeout, nel valore di una quantità qualsiasi compresa tra 0 e ledCount.
 
 ```C++
/*
Sceglie lo zucchero il base al tempo della 
pressione e si vuole aumentare la 
luminosità di una led bar da 0 a 10.

NB2: La quantità di zucchero si può scegliere facendo
 partire un timer alla pressione di un pulsante e 
 facendolo stoppare al suo rilascio. 
 La lettura del get() del timer mi da la misura 
 del ritardo. Decidendo che sopra 10 secondi 
 le misure valgono sempre 10 (tetto superiore), 
 un valore da 0 a 10 dice la quantità di zucchero 
 da stampare con Serial.println(). 
 Variante non bloccante.
*/

#include "urutils.h"
int led_rosso=13;
int led_verde=12;
int led_3=5;
int pulsante1=14;
const int ledCount = 10;
const int timeout = 2000;
DiffTimer t1;
int ledPins[] = {
  27, 15, 2, 4, 5, 18, 19, 21, 22, 23
};   // an array of pin numbers to which LEDs are attached

void paintBar(int ledLevel){
  // loop over the LED array:
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    // if the array element's index is less than ledLevel,
    // turn the pin for this element on:
    if (thisLed < ledLevel) {
      digitalWrite(ledPins[thisLed], HIGH);
    }
    // turn off all pins higher than the ledLevel:
    else {
      digitalWrite(ledPins[thisLed], LOW);
    }
  }
}
void setup(){
  // loop over the pin array and set them all to output:
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    pinMode(ledPins[thisLed], OUTPUT);
  }
  pinMode(pulsante1, INPUT);
  Serial.begin(115200);
}

void loop(){
  if(digitalRead(pulsante1)==HIGH)
  {
    t1.start(); // funzione singleton (segna solo il tempo del primo start!)
    int level = map(t1.get(), 0, timeout, 0, ledCount);
    Serial.println(level);
    paintBar(level);
  }else{
    t1.reset();
    t1.stop();
  }
  delay(10);
}
```

- Simulazione online su Esp32 con Wowki del codice precedente: https://wokwi.com/projects/422891162960627713
- Simulazione online su Esp32 con Tinkercad del codice precedente: https://www.tinkercad.com/things/fxuGfsE0KcZ/editel

## **APPLICAZIONI BASATE SUL POLLING DEL TEMPO CORRENTE**

In questo caso, il **rilevatore dei fronti** è realizzato **campionando** il valore del livello al loop di CPU **attuale** e **confrontandolo** con il valore del livello campionato al **loop precedente** (o a uno dei loop precedenti). Se il valore attuale è HIGH e quello precedente è LOW si è rilevato un **fronte di salita**, mentre se il valore attuale è LOW e quello precedente è HIGH si è rilevato un **fronte di discesa**.  

### **Toggle non bloccante con attivazione sul fronte di discesa**

E' un esempio efficace nel realizzare un pulsante con memoria (**logica toggle**).

Ma soprattutto la realizzazione della logica di un **rilevatore di transito** di oggetti potrebbe essere ben ottenuta con questo esempio di utilizzo di un timer grazie alla sua proprietà di rilevare prontamente i **fronti di discesa**, cioè quelle situazioni in cui normalmente un oggetto lascia l'area del sensore. 

Questa proprietà potrebbe essere utilizzata pure per realizzare un efficace **contapezzi**.

Un fronte di **discesa disarma** immediatamente il timer. Un eventuale rimbalzo sul fronte di discesa creerebbe comunque anche un **fronte di salita** spurio che **riarmerebbe** nuovamente il timer attivando il **debouncer**. 

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
#include "urutils.h"
int led = 13;
int led2 = 27;
byte pulsante = 12;
bool stato = LOW;
DiffTimer t1;
DiffTimer debt;

void blink(byte led) {
  digitalWrite(led, !digitalRead(led));
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(pulsante, INPUT);
  t1.start();// attivazione blink
}

// loop principale
void loop() {
  if (digitalRead(pulsante)) {// polling pulsante non premuto
    debt.start();
  }else if (debt.get() > 50) { // disarmo del timer al timeout
    debt.stop(); // disarmo del timer
    debt.reset();
    stato = !stato;
    digitalWrite(led2, stato);
  }
  if (t1.get() > 1000) { // polling timer blink
    t1.reset(); // riarmo timer blink
    blink(led);
  }
  delay(10);
}
```
- Simulazione con esp32 su Wokwi: https://wokwi.com/projects/391110569888045057
- Simulazione con Arduino su Tinkercad: https://www.tinkercad.com/things/dRnl4iE20U1-toggle-timer-sw-su-fronte-di-discesa

### **Toggle non bloccante con attivazione sul fronte di salita**

Consiste nell'utilizzare un timer per generare una **cadenza periodica** di letture del **livello corrente** dell'ingresso sotto osservazione per valutare, grazie la confronto col **livello passato**, se ci si trova in presenza di un **cambiamento** e quindi di un **fronte** (di salita o di discesa). L'analisi delle variazioni dei livelli può essere utilizzata per diversi **scopi**:

- Se il **periodo** è sufficientemente grande (50 msec) è possibile filtrare i ribalzi meccanici del pulsante e quindi realizzare, mediante il timer, un efficace **debouncer**.

- Il **confronto** tra la misura di un **livello** al periodo **attuale** e quello misurato al periodo **precedente** ci mette in grado di evidenziare l'occorenza di un **fronte**.

- La **memoria** dello **stato** di un pulsante può essere utilizzata per realizzare una logica di inversione del suo stato precedente (**logica toggle**) o una logica di **conteggio** dei fronti di salita, o di quelli di discesa, oppure di entrambi.

L'utilizzo del timer in questo pulsante prevede che mediamente entrambi i fronti siano letti con la **stesso ritardo** e questo non costituisce un problema se si vuole realizzare un **pulsante toggle** in quanto, sia la logica toggle che il debouncing, sono ugualmente realizzati in maniera efficace.

Pur tuttavia, la realizzazione della logica di un **rilevatore di transito** di oggetti potrebbe essere un tantino penalizzata da un ritardo introdotto dal timer nella rilevazione dei **fronti di discesa**, cioè prorpio in quelle situazioni in cui normalmente un oggetto lascia l'area del sensore.

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
#include "urutils.h"
int led = 13;
byte pulsante = 12;
byte stato = LOW; // variabile globale che memorizza lo stato del pulsante
DiffTimer t1;
DiffTimer debt;

// oggetto pulsante senza debouncing
typedef struct
{
  byte state = LOW;
  byte val0 = LOW;
  bool toggle(byte val) {// toggle con debouncing
	if ((val == HIGH) && (val0 == LOW)){// rilevazione fronte di salita
		state = !state; // logica toggle
	}	
	val0 = val;	// aggiornamento livello precedente al livello attuale
	return val;     // ritorna il valore attuale del pulsante
  }
} ToggleBtn;

ToggleBtn bt1;// pulsante senza antirimbalzo incorporato

void blink(byte led){
	digitalWrite(led, !digitalRead(led)); 
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  t1.start();// attivazione blink
  debt.start();// attivazione debouncer
}

// loop principale
void loop() {
  if(debt.get() > 50){// polling timer debouncer
        debt.reset();
        bt1.toggle(digitalRead(pulsante));// polling pulsante
  }
  if(t1.get() > 500){// polling timer blink
	t1.reset(); // riarmo timer blink
	if(bt1.state){// polling stato del toggle
		blink(led);
	}else{
		digitalWrite(led, LOW);
	}    
  }
  delay(10);
}
```

Simulazione online su Esp32 con Wowki del codice precedente: https://wokwi.com/projects/390695281576032257

## **PULSANTI TOGGLE BASATI SU INTERRUPT**

<img src="img\interruptPulsante.jpg" alt="alt text" width="1200">

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

## **SCHEDULATORE DI COMPITI BASATO SUL POLLING DEL TEMPO CORRENTE**

Una categoria di **schedulatori di compiti** che, a differenza di quelli basati sulla ```delay()```, possiede la peculiare proprietà di **non bloccare** l'esecuzione degli atri task del ```loop()``` si può ottenere realizzando,  all'interno del ```loop()```, il **polling periodico** della funzione ```millis()```. 

Il **polling** della funzione ```millis()``` serve a stabilire in **quale ciclo** di loop il **tempo corrente** assume il valore di **tempo limite** (timeout) oltre il quale **eseguire** un certo **compito**.

E’ buona norma evitare, all'interno dei task, l’esecuzione frequente di **operazioni lente** quando queste non sono strettamente necessarie, in modo da lasciare spazio ad altre operazioni, quali ad esempio gestione degli eventi di input, che richiedono una velocità elevata per essere usufruite in modo più interattivo.

**In particolare**, all'interno di un task, andrebbe accuratamente evitata l'introduzione di ritardi prolungati tramite **delay()**. Possono esserci per **breve tempo** (qualche mSec) ma, in ogni caso, non dovrebbero mai far parte della maniera (**algoritmo**) con cui la logica di comando **pianifica i suoi eventi**.

Utilizzando la tecnica della **schedulazione esplicita dei task** nel loop(), la **realizzazione di un algoritmo** non dovrebbe essere pensata in **maniera sequenziale (o lineare)** ma, piuttosto, come una **successione di eventi** a cui corrispondono dei **compiti** (task) che **comunicano** con le periferiche di input/output ed, eventualmente, dialogano tra loro in qualche modo (ad es. variabili globali, buffer, ecc.).


### **Momento del campionamento**

Il **polling** (o campionamento periodico) della millis() può essere fatto:
- all'**inizio di ogni ```loop()```** per cui la misura del tempo attuale è **la stessa** per tutti i task e con questa, task per task, ci si regola per stabilire il ciclo di loop() in cui un determinato task diventa "maturo", cioè, quello in cui per lui il tempo misurato supera il **timeout** oltre il quale deve andare in esecuzione.
  ```C++
  // task 1
  unsigned long currMillis = millis();
  if ((currMillis - precs[0]) >= period[0]) {
	precs[0] += period[0]; 
	//................
  }	
  // task 2
  if (((currMillis - precs[1]) >= period[1]) {
	precs[1] += period[1]; 
	//................
  }
  ```
- all'**inizio di ogni task** in modo da **compensare**, nella valutazione precedente del tempo del task, anche di un eventuale **ritardo** cumulato dai task precedentemente eseguiti. 
  E' una variante **più precisa** della precedente anche se **più costosa computazionalmente** (eccessivi polling della millis() quando ci sono molti task).
  ```C++
	// un timer SW indipendente per ogni task
  	// task 1
	if ((millis() - precs[0]) >= period[0]) {
		precs[0] += period[0]; 
        	//..........
	}	
	// task 2
	if (((millis() - precs[1]) >= period[1]) {
		precs[1] += period[1]; 
        	//...........
	} 
    ```
  
### **Precisione del timer SW**

I timer SW basati sul polling possono essere realizzati in due forme:
- senza compensazione del ritardo di esecuzione della millis():
	```C++
	unsigned long tassoluto = 0;
	if ((millis() - tassoluto) >= periodo)
	{
		tassoluto = millis();
		....
	}
	```
 	E' poco preciso sulle lunghe corse se si è interessati anche al tempo assoluto (deriva dopo pochi minuti)
  
 - Con compensazione del ritardo di esecuzione della assegnazione che coinvolge la millis():
	```C++
	unsigned long tassoluto = 0;
	if ((millis() - tassoluto) >= periodo)
	{
		tassoluto += periodo;
		....
	}
	```
	E' più preciso sulle lunghe corse se si è interessati anche al tempo assoluto (deriva dopo parecchie ore)


### **Tipi di schedulazione**

Una volta stabilito ad ogni loop() il valore del tempo corrente, ci sono molti modi per determininare da esso i tempi dei vari task. Ognuna di queste tecniche, in realtà, è adoperabile a prescindere dal campionamento della millis(), tutte richiedono soltanto che lo schedulatore venga chiamato al tempo usato come **riferimento** per la generazione di tutti gli altri tempi. Questo tempo è detto **tempo base**. 

Il **tempo base** è un tempo fisso comune a tutti i task, la loro **periodicità** deve quindi essere un **multiplo intero** del tempo base.

Gli **schedulatori** di questo tipo sono **periodici** e si possono realizzare sostanzialmente in **due modi**:
- mediante ripetizione di una **sequenza di tick** periodici che fissano il **tempo base**. Un successivo **conteggio** o **filtraggio** dei tick discrimina il **momento** in cui deve essere eseguito **un task** piuttosto che un altro.
- mediante **timer periodici** che, per **ogni task**, stabiliscono un **timeout** oltre il quale il **polling** della funzione millis() **segnala** il momento buono per **eseguire** quel task.

<img src="schdulatore_generico.jpg" alt="alt text" width="1000">


### **SCHEDULATORE DI COMPITI BASATO SU TIMER PERIODICI**

Uno schedulatore di compiti (task) si può realizzare anche utilizzando **più timers** basati sul polling della funzione millis(). 

```C++
byte led1 = 13;
byte led2 = 12;
unsigned long period[2];
unsigned long precs[2];

void setup()
{
	pinMode(led1, OUTPUT);
  	pinMode(led2, OUTPUT);
	precs[0]=0;
	precs[1]=0;
	period[0] = 500;
	period[1] = 1000;
}

void loop()
{
	unsigned long current_millis = millis();
	// task 1
	if ((current_millis - precs[0]) >= period[0]) {
		precs[0] += period[0]; 
        	digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
	}	
	// task 2
	if ((current_millis - precs[1]) >= period[1]) {
		precs[1] += period[1]; 
        	digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
	}
	// il codice eseguito al tempo massimo della CPU va qui
}
```

Si noti che:
- il timer SW con il polling viene eseguito ad **ogni ciclo** di ```loop()```
- i timer SW sono N (uno per ogni tempo futuro)
- il calcolo degli N tempi futuri è eseguito N volte (una per ogni tempo) **ad ogni ciclo** di ```loop()```
- il calcolo può essere reso estremamente **preciso** e realizzato in maniera indipendente tra un tempo e l'altro.

Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/embed/fcbmLkC10ms?editbtn=1

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/351933794966569551

### **Versione dello schedulatore precedente realizzato con il timer get()**

```C++
/*
Alla pressione del pulsante si attiva o disattiva il lampeggo di un led, mentre un
altro led lampeggia indisturbato.
*/
#include "urutils.h"
int led = 13;
int led2 = 14;
byte pulsante =12;
byte precval, val;
unsigned long tbase = 50;
DiffTimer tmrdeb;
DiffTimer tmrblink1, tmrblink2;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(pulsante, INPUT);
  precval=LOW;
  tmrdeb.start();
  tmrblink2.start();
}

// loop principale
void loop() {
  if(tmrdeb.get() > tbase){  	
    tmrdeb.reset();   
    //task_toggle
    val = digitalRead(pulsante);		
    if(precval==LOW && val==HIGH){ 		//rivelatore di fronte di salita
      tmrblink1.toggle();		
    }
  }
  precval=val;	

  //task_blink1
  if (tmrblink1.get() > 500) {
    digitalWrite(led, !digitalRead(led));
    tmrblink1.reset();
  } 
  //task_blink2
  if (tmrblink2.get() > 1000) {
    digitalWrite(led2, !digitalRead(led2));
    tmrblink2.reset();
  } 
  
  delay(10);
}
```
- Di seguito il link della simulazione online con ESP32 su Wokwi: https://wokwi.com/projects/423595054703779841
- Di seguito il link della simulazione online con Arduino su Tinkercad: https://www.tinkercad.com/things/8y61Srk2Ezd-schedulatore-periodico-con-get

## **SCHEDULATORE DI COMPITI BASATO SU FILTRAGGIO DEL TEMPO BASE**

E' possibile realizzare uno schedulatore di **più task** che agiscono con periodicità **diverse** in tempi diversi a partire da un timer che esegue periodicamente un **task comune** si sincronizzazione che agisce su un **tempo** base comune **sottomultiplo** del tempo di tutti gli altri task. In altre parole, il **periodo** di ciascun task è un **multiplo intero** del periodo base di sincronizzazione comune:
- il **tick** del **tempo base** viene ottenuto mediante il polling della funzione ```millis()``` eseguito ad **ogni ```loop()```**
- ad ogni **tick** del **tempo base** viene aggiornato il momento del prossimo tick mediante l'espressione ```precm += tbase```
- Il **tempo base** comune a tutti i task, si può calcolare col **massimo comune divisore** (M.C.D. o G.C.D) di tutti i tempi dei singoli task.
- **ad ogni tick** del tempo base viene misurato se è maturato il tick del tempo di ciascun task valutando il valore del tempo base fino a quel momento mediante ```(precm - precs[i]) >= period[i]```
- se la **condizione di scatto** del tick di un task è verificata allora viene calcolato il **tempo** del tick **successivo** sommando al tempo del tick corrente la periodicità del task: ```precs[i] += period[i]```


```C++
byte led1 = 13;
byte led2 = 12;
unsigned long period[2];
unsigned long precs[2];
unsigned long precm;
unsigned long tbase;

void setup()
{
	pinMode(led1, OUTPUT);
  	pinMode(led2, OUTPUT);
	period[0] = 500;
	period[1] = 1000;
	precm = 0;
	tbase = 500;
	// inizializzazione dei task
	for(int i=0; i<2; i++){
		precs[i] = precm -period[i];
	}
}

void loop()
{
	if(millis()-precm >= tbase){ 	
		precm += tbase;  			
		// task 1
		if ((precm - precs[0]) >= period[0]) {
			precs[0] += period[0]; 
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		}	
		// task 2
		if ((precm - precs[1]) >= period[1]) {
			precs[1] += period[1]; 
			digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
		}
	}
	// il codice eseguito al tempo massimo della CPU va qui
	delay(1);
}
```
- Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/352691213474403329
- Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/things/7IIKSJQCbIr-scheduler-by-millis-con-tick-base

**Tempo assoluto**

La generazione del tempo assoluto di riferimento mediante```precm += tbase``` è basata su considerazioni riportate in [Generazione di tempi assoluti](absolutetime.md)

**Inizializzazione dei task**

Per ottenere la partenza sincronizzata di tutti i task al primo tick del sistema bisogna prima inizializzare il tempo trascorso (```precs[i]```) di ogni task al valore del suo intervallo di esecuzione (``period[i]``):

```C++
//Inizializzazione dei task
for(int i=0; i<2; i++){
	precs[i] = precm -period[i];
}
```

### **Versione dello schedulatore precedente realizzato con il timer get()**

Schedulatore realizzato utilizzando una variante ```DiffTimer2``` del timer della libreria [urutils.h](urutils.h) in cui, ad ogni chiamata della funzione ```get()```, viene incrementato il tempo corrente di ciascun timer (elapsed) di una quantità fissa pari al **tempo base**. Il momento dell'incremento è contestuale a quello della chiamata a ```get()``` che, quindi, deve avvenire esattamente ogni tempo base. Il tempo base è il M.C.D. dei tempi in gioco nei vari task.

Il **tempo base** viene generato utilizzando il timer ```DiffTimer1``` che realizza, mediante il polling della sua funzione ```get()``` il polling della funzione ```millis()``` che restituisce il tempo corrente del sistema. 

```C++
/*
Alla pressione del pulsante si attiva o disattiva il lampeggo di un led, mentre un
altro led lampeggia indisturbato.
*/
#include "urutils.h"
int led = 13;
int led2 = 14;
byte pulsante =12;
byte precval, val;
unsigned long tbase = 50;
DiffTimer tmrdeb;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(pulsante, INPUT);
  precval=LOW;
  tmrdeb.start();
  tmrblink2.start();
}

// loop principale
void loop() {
  if(tmrdeb.get() > tbase){  	
    tmrdeb.reset();   
    //task_toggle
    val = digitalRead(pulsante);		
    if(precval==LOW && val==HIGH){ 		//rivelatore di fronte di salita
      tmrblink1.toggle();		
    }
    precval=val;	
    //task_blink1
    if (tmrblink1.get(tbase) > 500) {
      digitalWrite(led, !digitalRead(led));
      tmrblink1.reset();
    } 
    //task_blink2
    if (tmrblink2.get(tbase) > 1000) {
      digitalWrite(led2, !digitalRead(led2));
      tmrblink2.reset();
    } 
  }
  delay(10);
}
```
Di seguito il link della simulazione online con ESP32 su Wokwi: https://wokwi.com/projects/388359604541585409

## **SCHEDULAZIONE CON I TIMERS HW**

La **schedulazione dei task** normalmente riguarda quei compiti che devono essere ripetuti in **maniera periodica**, infatti si parla di **loop() principale** e di **loop secondari** eventualmente sullo stesso thread (**protothread** o mediante costrutti **async/await**) o su altri **thread**. Lo stesso risultato può essere ottenuto con dei timer HW che realizzano un loop su **base evento**. L'**evento** è l'**interrupt** di un timer HW, il **loop** è rappresentato dalla **calback** associata al timer e che viene viene da esso **periodicamente richiamata**.

Ma un **timer** si può impostare per **generare**:
- **eventi sincroni** cioè **periodici** dopo intervalli fissi e programmabili
- **eventi asincroni** cioè **aperiodici** da eseguire una sola volta (monostabili o one-shot) dopo un certo tempo calcolato **nel futuro** a partire dal **momento** in cui avviene il comando di **attivazione** (start).

Per cui un **timer HW** offre una versatilità in più se, nello stesso progetto, si vogliono **combinare insieme** eventi periodici con eventi aperiodici triggerati da eventi su un **ingresso** o da altri **eventi** (scadenza di timeout o contatori).

La **stessa cosa** è in realtà possibile realizzarla anche con i **timer SW** basati sul polling nel loop principale della funzione millis(). La loro **versatilità** è uguale se non **superiore**, ma sono soggetti ad alcune limitazioni che potrebbero renderli non adatti in certi contesti. L'ambito che **penalizza** di più i timer SW è quello delle applicazioni **mission-critical** (o **critical-time**). In questo tipo di applicazioni si deve prevedere che l'esecuzione di certi compiti avvenga in maniera estremamente puntuale, pena l'introduzione di **instabilità** nel sistema o di **perdita di sicurezza** per chi lo adopera.

Rispetto agli altri metodi di creazione di base dei tempi (polling della millis(), thread e protothread), è tendenzialmente più legato ad uno specifico vendor di HW e ad una specifica linea di prodotti. Le **API dei timer**, pur esendo **molto simili** tra loro, **non sono standardizzate** e la **portabilità** del SW nel tempo potrebbe non essere garantita. In ogni caso **semplificano** parecchio la **gestione delle ISR** associate a timer HW che altrimenti, eseguita a basso livello, richiede una impostazione di **registri interni** della CPU che necessita di conoscenze di dettaglio molto specifiche.

 ### **Modalità di utilizzo**

La **modalità di utilizzo** è simile a quella di una **usuale ISR**. Una **funzione di callback** viene associata ad un evento di un timer tramite un metodo **attach()**. La dissociazione si fa con il metodo contrario **detach()**. Gli **eventi possibili** sono una chiamata una tantum (**one shot** o timer monostabile) o una chiamata **periodica** o timer bistabile. Nella definizione dell'attach() viene anche impostato il **tempo di scadenza** del timer.  Gli **eventi possibili** sono:
- una chiamata una tantum (**one shot** o timer monostabile)
- una chiamata **periodica** o timer bistabile.  

I **timer HW** possono essere:
- **fisici**. Cioè **limitati in numero** ai dispositivi HW dedicati a questo scopo effettivamente presenti nel sistema (nell'ESP32 sono 4) e inoltre devono essere riferiti esplicitamente nel codice con un loro specifico id (un numero da 1 a 4).
- **logici o virtuali**. Sono virtualmente di **numero infinito** e rappresentano delle **astrazioni SW** del sottostante HW fisico che è condiviso in qualche modo da uno **strato di gestione** trasparente al programmatore. 

La possibilità di poter **instanziare** un **timer logico** per **ogni task**, a sua volta definito all'interno di una certa **callback**, rende l'utilizzo dei timer una strada **effettivamente percorribile** per la realizzazione di uno **schedulatore di compiti** sia periodici che one shot (esecuzione singola non ripetuta). La **schedulazione** dei compiti inoltre rimane **molto precisa** perchè collegata a ISR eseguite da segnali di interrupt lanciati da **timer fisici**.

Si ricorda che in C e in C++ le variabili globali a comune tra ISR e loop() principale e suscettibili di modifiche da parte di entrambi andrebbero dichiarate tutte con il qualificatore ```volatile```.

### **Limitazioni nell'utilizzo**

I timer richiamano particolari funzioni di callback , le ISR, che sono attivate a seguito di  un interrupt HW (timeout del timer). Nei sistemi multi-core questi interrupt possono essere chiamati solo dal core che ha eseguito il timer. Ciò significa che se si utilizza un timer per attivare un interrupt, è necessario assicurarsi che l'interrupt venga chiamato solo dal core che sta eseguendo il timer.

le ISR di base **non** sono **interrompibili** a meno di impostare apposite istruzioni che lo consentono. **Interrompere una ISR** potrebbe causare inconsistenze nella gestione delle risorse condivise con il loop o altri thread. D'altra parte, una ISR che non si interrompe per molto tempo impedisce la tempestiva esecuzione delle altre ISR dovute ad interruzioni **simultanee** o **temporalmente vicine**.

Esistono **limitazioni speciali** su ciò che può e non può essere fatto all'interno delle ISR nella maggior parte dei controllori: 
- Ad esempio, non è consentito allocare **memoria dinamica** all'interno di una ISR. 
- una ISR che esegue una **logica complessa** potrebbe essere eseguita così lentamente da creare instabilità del sistema dovuta al fatto che altre interruzioni, che gestiscono servizi essenziali del sistema, non sono state prontamente soddisfatte. Un gestore di interrupt dovrebbe essere sempre una funzione **breve** che esegue il **lavoro minimo** necessario per **modificare** delle **variabili esterne**.
- In genere, in molte implementazioni, callback diverse di uno stesso timer vengono eseguite **in sequenza** e non su thread paralleli per cui operazioni bloccanti come le ```delay()```, oltre a causare possibili **instabilità** (sono ISR basate su interrupt), **ritardano** l'esecuzione delle callback **a seguire**.
- eseguire **task complessi** con un timer HW è possibile a patto che questi vengano resi interrompibili senza creare problemi, e ciò si può ottenere eseguendoli in un **altro thread** o nel **loop principale**. Nel **loop principale**, un **task complesso** può sempre essere immediatamente attivato da una ISR che asserisce un opportuno **flag di avvio**.
- nel **multitasking** con i **timer logici**, il parallelismo degli **eventi** è garantito mentre il parallelismo percepito dei **task** è tanto maggiore quanto più questi sono veloci (dato che in realtà questi sono memorizzati in una coda da dove vengono estratti e mandati in esecuzione uno ad uno in sequenza). Nel **multitasking con i timer** il task di una callback viene eseguito solamente **dopo** il completamento di una eventuale callback **attualmente** in coda.
- nel **multitasking** con i **thread**, sia il parallelismo degli **eventi** che quello dei **task** è **completo** perchè nella coda di esecuzione stavolta in sequenza non vengono eseguiti interi task ma, a turno, gruppi di istruzioni macchina di ciascun task (meccanismo dell'interleaving).  Nel **multitasking con i thread** il task di un thread viene eseguito solamente **dopo** il completamento del quanto di tempo assegnato al task **attualmente** presente nella coda dei **thread in esecuzione**.
- nel **multitasking** con **l'interrupt sugli ingressi**, sia il parallelismo degli **eventi** che quello dei **task** è **completo** perchè la coda di esecuzione stavolta viene bloccata (mettendo thread e processi in wait) lasciando giusto il tempo di completare l'**istruzione macchina** corrente.  Nel **multitasking con gli interrupt a basso livello** il task di una ISR viene eseguito immediatamente **dopo** il completamento dell'istruzione macchina del task **attualmente** presente nella coda dei **thread in esecuzione**.

### **Timers SW vs Timer HW**

Se con la funzione ```get()``` di un **timer SW** si desiderasse controllare una **scadenza**, cioè vedere se è passato il tempo necessario per compiere una certa **azione** (modificare una variabile o chiamare una funzione), allora si dovrebbero soddisfare **due esigenze**:
- verificare la verità della condizione ```get() > timeout``` ed eseguire, eventualmente, l'azione prevista al timeout (scadenza) del timer.
- eseguire la valutazione al punto precedente **periodicamente**, finchè essa non accade.

I **due requisiti** precedenti si traducono nell'azione di eseguire il **polling** della funzione ```get()``` nel ```loop()```, alla massima velocità o, più lentamente, ogni tot millisecondi. Nella vita reale, si può assimilare ad un polling l'**osservazione periodica** di un orologio a muro effettuata con lo scopo di individuare il momento esatto in cui deve essere eseguita una certa azione come, ad esempio, l'estrazione dal forno di un ciambella.

Se si volesse fare la stessa cosa con un **timer HW** allora ci si renderebbe conto che il polling non è più necessario perchè, **al timeout**, attraverso, un **segnale** proveniente dal timer HW,  viene attivato l'**ISR** associata a quel segnale che, a sua volta, comanda l'esecuzione di una **callback** definita al suo interno. Per rimanere alla metafora precedente, adesso non è più necessario osservare periodicamente l'orologio alla parete, perchè un timer, impostato ad inizio cottura, avviserà con un segnale acustico il pasticciere quando il momento di togliere il dolce dal forno sarà arrivato . 

Si noti che, sia per timer HW che per timers SW, l'**ordine di apparizione** dei task all'interno del codice del loop potrebbe non rispecchiare l'**ordine** con cui gli stessi vengono **eseguiti**, cioè l'ordine di esecuzione avviene in maniera **asincrona** rispetto al codice, secondo il principio della cosidetta **programmazione ad eventi**. Eventi possono attivare o disattivare altri eventi in base ad input esterni oppure in base al valore raggiunto da certe variabili di stato.


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

In questo esempio, l'encoder rotativo è stato gestito con l'algoritmo con cui tipicamente si possono discriminare i fronti di un interruttore mediante interrupt. In questo caso il **fronte** che viene rilevato **direttamente** è quello di **discesa** del piedino **CK**. L'avvenuto passaggio del fronte sul segnale **DT** è valutato **indirettamente** osservando il **livello** letto sul piedino corrisposndente tramite l'istruzione ```digitalRead(ENCODER_DT)```.


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
- Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/389923052943921153

Le variabili **condivise** tra una ISR e il loop() andrebbero protette, da accessi **paralleli** e concorrenti da parte di entrambe, tramite delle **corse critiche** che rendano l'accesso **strettamente sequenziale**. Inoltre le variabili condivise devono sempre essere dichiarate con il qualificatore ```volatile``` per forzarne la modifica istantanea anche sui registri della CPU. 

Gli **accessi paralleli** non sono un problema quando le **istruzioni** sono **atomiche**, cioè non interrompibili. Le istruzioni atomiche o sono eseguite per intero o non sono eseguite affatto. In questo caso gli **accessi**, sia in lettura che in scrittura, sono in realtà, a basso livello, **intrinsecamente sequenziali**.

Nei microcontrollori attuali, in genere **nessuna istruzione** gode della proprietà di essere **atomica** con una sola eccezione per la lettura e scrittura delle **variabili ad 8 bit**. Per le variabili codificate con **8 bit** l'accesso a basso livello (linguaggio macchina) è intrinsecamente garantito essere **atomico**. Per queste variabili rimane comunque la necessita dell'uso del qualificatore ```volatile```.

Le **modifiche** a valori con codifiche **maggiori di 8 bit** sono in genere **non atomiche**, pertanto le variabili a 16 o 32 bit andrebbero gestite con gli interrupt disabilitati (sezione critica). Tuttavia, gli interrupt vengono disabilitati di default durante una routine di servizio di interrupt, quindi, non potendo verificarsi il danneggiamento di una variabile multibyte in una ISR, le **sezioni critiche** vanno inserite soltanto nel ```loop()```.

Le variabile **condivisa** tra ISR e loop() e **8 bit** è ```stato``` ed è stata dichiarata come ```volatile``` e ogni suo accesso (in lettura e scrittura) è  stato racchiuso in una  **sezione critica** dove l'acccesso parallelo è stato disabilitato in favore di uno **strettamente sequenziale** che garantisce l'atomicità delle istruzioni in essa racchiuse.

Per quanto riguarda le sezioni critiche si può approfondire in [sezioni critiche](intsezionicritiche.md)

Gli encoder differenziali, poichè internamente sono dei normalissimi interruttori, sono soggetti anch'essi aal fenomeno dei rimbalzi e all'usura che progressivamente accentua quel fenomeno. Per applicazioni pratiche effettive, è conveniente utilizzare delle implementazioni capaci di tenerne conto. Per questo e altro vedere <a href="encoder.md">Encoder rotativo</a>


## **JOISTICK ANALOGICO**

<img src="img\A85262_Joystick-Module_3.webp" alt="alt text" width="300">

Pins:
- **Vcc.** Alimentazione positiva
- **VERT.**  Uscita verticale analogica
- **HORZ.**  Uscita orizzontale analogica
- **SEL.** 	Pulsante
- **GND.**	Massa

Il joistick è realizzando combinando, su due assi separati ed ortogonali, due potenziometri rotativi che realizzano due partitori di tensione. L'ingresso dei partitori è polarizzato con l'alimentazione positiva Vcc, per cui l'uscita restituisce per entrambi una tensione compresa tra Vcc e 0V, a seconda dell'ammontare della rotazione. 

La tensione in uscita al partitore rappresenta la quantità della rotazione e deve misurata dal microcontrollore tramite una periferica **ADC** di conversione del segnale da analogico a digitale. L'ADC è collegato di base a tutte le porte analogiche di un microcontrollore. 


<img src="img\joyrange.png" alt="alt text" width="900">

La **quantizzazione** della conversione per la maggior parte dell MCU, come Arduino o ESP32, è a 10 bit, circostanza che limita la risoluzione della misura a 1024 **livelli di tensione**, e quindi saranno dello stesso numero anche i diversi campioni di rotazione misurabili con una escursione da un estremo all'altro di uno dei due assi del joistick.


Anche se sembra strano, l'ADC a 10 bit di Arduino (1024 valori) è più preciso e affidabile di quello a 12 bit dell'ESP32 (4096 valori). La quantizzazzione nella MCU ESP32 è sensibilmente non lineare, soprattutto in prossimità dei valori estremi. In sostanza, ciò significa che l'ESP32 non è in grado di distinguere un segnale di 3,2 V e 3,3 V: il valore misurato sarà lo stesso (4095). Allo stesso modo, l'ESP32 non distinguerà tra un segnale 0 V e 0,2 V per piccole tensioni. Si può provare a calibrare l'ADC per ridurre le non linearità attarverso un mappaggio SW, come decritto [quì](https://github.com/e-tinkers/esp32-adc-calibrate).

<img src="img\ESP32 Joystick Interfacing.webp" alt="alt text" width="500">

```C++
#define VERT_PIN A3
#define HORZ_PIN A0
#define SEL_PIN  2

void setup() {
  Serial.begin(115200);
  pinMode(VERT_PIN, INPUT);
  pinMode(HORZ_PIN, INPUT);
}


void loop() {
  int x = analogRead(VERT_PIN);
  int y = analogRead(HORZ_PIN);
  bool selPressed = digitalRead(SEL_PIN) == LOW;
  
  Serial.print("x: ");Serial.print(x); Serial.print(" y: ");Serial.println(y);
  delay(500);
}
```
Simulazione di una MCU ESP32 con Wokwi: https://wokwi.com/projects/391096564707796993

Per altre informazioni sul joistick analogico va su [joistick](joistick.md).

## ESERCIZI SU ADC

- [Consegne esercizi ADC](consegneEserciziADC.md)
- [Soluzioni esercizi ADC](soluzioniEserciziADC.md)

## ESERCIZI LED E PULSANTI
- [Esercizi led base](esercizi_led_base.md)

## ESERCIZI SU PULSANTI (NORMALI E TOGGLE) E TASK CONCORRENTI

### **Es1**
1) Creare un programma che realizzi l'accesione di due led. Uno ogni mezzo secondo in modalità "normale" ed uno ogni scendo in modalità "drain".
2) Svolgere lo stesso esercizio di cui sopra ma fare in modo che il blink di uno dei led duri soltanto 10 lampeggi.
- https://wokwi.com/projects/378094177285037057
- https://wokwi.com/projects/378093455257452545

### **Es2**
Creare un programma che gestisca l'accensione di due led (uno in modo drain e l'altro normale) in maniera tale che:
    - led1 blinki mezzo secondo per 5 volte dopo di che blinki ogni secondo
    - led2 blinki un secondo per 5 volte dopo di che blinki ogni mezzo secondo
  
  - https://wokwi.com/projects/381214153548600321

### **Es3**  
Creare un programma che gestisca l'accensione di due led (uno in modo drain e l'altro normale) in maniera tale che:
  - il led1 blinki ogni mezzo secondo
  - il led2 blinki ogni secondo
  - il led2 parta dopo che il led1 abbia blinkato per 3 volte e si sia spento
  - il led1 si riaccenda dopo che il led2 abbia blinkato per due volte e si sia spento
    
https://wokwi.com/projects/381214508523057153

### **Es4**
Realizzare un programma arduino che faccia lampeggiare un led mentre un altri tre led (rosso, verde e giallo) realizzano la tempistica di un semaforo.
- https://wokwi.com/projects/379451315100484609
- https://wokwi.com/projects/379449185988576257
- https://wokwi.com/projects/379393885414092801
  
### **Es5**
Scrivere e simulare un programma (arduino o esp32) che realizzi l'accensione di ul led funchè si preme un pulsante e contemporaneamente il blink di un'altro.
- https://wokwi.com/projects/379577438032393217

### **Es6**
Scrivere un programma (arduino o esp32) che realizzi un blink di un led giallo fino a che si tiene premuto un pulsante e realizzi pure il blink di un led verde quando viene premuto un altro pulsante.
- https://wokwi.com/projects/381194244081573889
- https://wokwi.com/projects/381194871814782977

### **Es7**
Scrivere e simulare un programma (arduino o esp32) che realizzi l'accensione di un led finchè si preme un pulsante e, contemporaneamente, il blink di un altro led.
- https://wokwi.com/projects/379577438032393217
- https://wokwi.com/projects/381114962105698305

### **Es8**
Compilare un progetto con esp32 che realizzi il comando di accensione/spegnimento di un led in modo toggle con una pressione sullo stesso tasto (una pressione accende, la successiva spegne).
- https://wokwi.com/projects/381115546873501697
  
### **Es9**
Come sopra, ma con la variante che, solo quando il primo led è accesso, la pressione di un secondo pulsante accende/spegne in modo toggle il blink di un secondo led. Se il primo led è spento il secondo pulsante non fa nulla.
- https://wokwi.com/projects/381120076068796417
- https://wokwi.com/projects/381116393250548737

## **URUTILS**
I prossimi esercizi sono realizzati adoperando la libreria che si può scaricare cliccando col tasto sinistro sul link [urutils.h](urutils.h). 

Una volta aperta la pagina, per scaricarla premere la freccetta di download in alto a destra.

### **Es10**
Creare un programma per esp32 che realizzi con 4 led l'effetto supercar inserbile e disinseribile in modo toggle con un pulsante.
- https://wokwi.com/projects/381192229624334337
  
### **Es11**
Lo stesso di prima ma con un blink di un led a parte che deve rimanere insensibile ai comandi che agiscono sui led supercar (nessun ritardo).
- https://wokwi.com/projects/381192787395509249
  
### **Es12**
Fare un toggle che accende un led dopo 4 volte che si preme un pulsante e lo spegne dopo 2 (suggerimento: prevedere un tempo massimo complessivo per le pressioni di 1 sec).
- https://wokwi.com/projects/381191721792664577

### **Es13**
Creare un programma per esp32 che realizzi l'accensione di un led su 4 alla prima pressione, di un secondo led alla seconda, di un terzo alla terza. 
- https://wokwi.com/projects/381146888228329473

### **Es14**
1. Scrivere un programma in esp32 che realizzi un pulsante che accende un led solo finchè esso è premuto (pulsante senza memoria). 
    - https://wokwi.com/projects/348779417449792082
2. Al codice precedente aggiungere che alla pressione del tasto, oltre ad accendere un led, venga anche mantenuto il blink (lampeggio) continuo di un secondo led. Sia accensione continua del primo led che il blink del secondo, ovviamente, si interrompono non appena si rilascia il pulsante.
    - https://wokwi.com/projects/348779417449792082
3.  Al codice di cui sopra, aggiungere un secondo tasto che realizzi, in modo toggle (tasto con memoria), l'accensione di un terzo led.
    - https://wokwi.com/projects/381144987021586433
    - https://wokwi.com/projects/381122295298210817
4.  Al codice precedente aggiungere che il toggle inibisca completamente l'effetto del primo tasto, lasciando tutto spento, quando lo stato del toggle è false mentre lo riabilita quando lo stato del toggle è true.
    - https://wokwi.com/projects/381145527357009921

### **Es15**
1. Scrivere un programma in esp32 che realizzi un pulsante che quando esso è premuto accenda un led verde e ne spenga uno rosso. Quando si rilascia deve accadere esattamente il viceversa.
    - https://wokwi.com/projects/381379298406892545
2. Al codice di cui sopra, aggiungere un secondo tasto che realizzi, in modo toggle (tasto con memoria), il blink di un terzo led.
    - https://wokwi.com/projects/381379835699414017
3. Al codice precedente aggiungere che alla pressione del primo tasto, parta pure un contatore di secondi. Dopo 10 secondi si deve attivare il toggle del terzo led.
    - https://wokwi.com/projects/381380050178326529
    - https://wokwi.com/projects/381380815530919937

### **Es16**
1. Scrivere un programma in esp32 che realizzi un pulsante che quando esso è premuto accenda il blink alternato di due led giallo e rosso. Quando si rilascia deve rimanere attivo solo il led rosso.
   -    https://wokwi.com/projects/381412873080576001
3. Al codice di cui sopra, aggiungere un secondo tasto che realizzi, in modo toggle (tasto con memoria), il blink di un terzo led ma solo se il pulsante precedente è rilasciato.
   -    https://wokwi.com/projects/381413194370570241
4. Al codice precedente aggiungere che alla pressione del primo tasto, parta pure un contatore di secondi. Dopo 5 secondi si deve attivare il toggle del terzo led.
   -    https://wokwi.com/projects/381413625688702977

### **Es17**
1. Scrivere un programma che accenda due led con un tasto senza memoria.  Quando il tasto è premuto, il primo  led lampeggia due volte dopodichè il secondo lampeggia tre volte, dopodichè entrambi lampeggiano indefinitivamente.
   -    https://wokwi.com/projects/381751605001285633
2. Dato l'esercizio al punto precedente, aggiungere un secondo tasto e un terzo led. Il nuovo tasto azionato in modo Toggle, fa lampeggiare il terzo led oppure lo spegne. Fare in modo che quando i primi due led lampeggiano insieme il secondo tasto cambi stato una sola volta.
   -    https://wokwi.com/projects/381413194370570241
3. Al codice precedente aggiungere la funzione che spegne il lampeggio sul terzo led dopo un tempo di 10 secondi (timer UR in dispensa).
   -    https://wokwi.com/projects/381754353576022017

### **Es18**
1. Scrivere un programma che accenda due led con un tasto senza memoria. Quando il tasto è premuto, il primo  led lampeggia dieci volte, dopodichè entrambi lampeggiano alternativamente indefinitivamente.
    -    https://wokwi.com/projects/381756214350758913
2. Dato l'esercizio al punto precedente, aggiungere un secondo tasto e un terzo led. Il nuovo tasto azionato in modo Toggle, fa lampeggiare il terzo led con il codice morse delll'SOS (tre impulsi brevi, tre lunghi e di nuovo tre brevi). Una pressione successiva del tasto lo fa blinkare normalmente.
    -    https://wokwi.com/projects/381758116826337281
3. Al codice precedente aggiungere la funzione che spegne l'esecuzione dell'SOS dopo 10 secondi. Dopo 20 secondi il led SOS smette pure di blinkare (timer UR in dispensa).
    -    https://wokwi.com/projects/381759436108030977

### **Es19**
1. Scrivere un programma che accenda due led con un tasto senza memoria.  Quando il tasto è premuto, il led lampeggiano 5 volte alternativamente in maniera rapida, dopodichè entrambi lampeggiano alternativamente in maniera più lenta per un tempo indefinito.
     -    https://wokwi.com/projects/381764658556760065
3. Dato l'esercizio al punto precedente, aggiungere un secondo tasto e un terzo led. Il nuovo tasto azionato in modo Toggle, fa lampeggiare i l terzo led in maniera alternata e rapida per un tempo indefinito. Una seconda pressione lo fa lampeggiare indefinitivamente in maniera più lenta. 
     -    https://wokwi.com/projects/381767106528833537
4. Al codice precedente aggiungere la funzione che misura il tempo x in cui si preme il tasto con memoria e al suo rilascio fa lampeggiare periodicamente il terzo led con un periodo pari ad x  (timer UR in dispensa).
     -    https://wokwi.com/projects/381773540992974849

### **Es20**
1. Scrivere un programma che accenda insieme led con un tasto senza memoria.  Quando il tasto è premuto, il led lampeggiano 4 volte insieme, dopodichè entrambi lampeggiano alternativamente uno alla volta 4 volte per un tempo indefinito. A tasto rilasciato rimane acceso il secondo led.
     -    https://wokwi.com/projects/381781410707068929
3. Dato l'esercizio al punto precedente, aggiungere un secondo tasto e un terzo led. Il nuovo tasto azionato in modo Toggle, fa lampeggiare il secondo led secondo la logica al punto 1) se lo stato è true, altrimenti sta spento.
     -    https://wokwi.com/projects/381782303090374657
4. Al codice precedente aggiungere la funzione che misura il tempo x per cui si preme il primo tasto con memoria. Questo tempo diventa la durata complessiva del blink del terzo tasto (timer UR in dispensa).
     -    https://wokwi.com/projects/381782735128863745

### **Es21. Timer monostabile**
Scrivere un programma che realizzi l'accensione di un led tramite un pulsante temporizzato che spegne il led 
dopo un numero di ms impostati da setup. La logica del tasto deve essere senza stato e deve essere sensibile 
al fronte di salita del segnale. Immaginare un collegamento pull down del tasto.
- https://www.tinkercad.com/things/fCpauVnNUZh-accensione-led-monostabile/editel
  
###  **Es22. Curva pericolosa1**

Realizzare un programma Arduino che dati 4 led (led1, led2, led3, led4) realizza un indicatore di curva pericolosa 
che accende i led in sequenza con un incremento circolare. Usare un tasto che può essere premuto in successione 
per accendere o spegnere l’indicatore (toggleH). Supponiamo che il tasto è collegato secondo lo schema pull down.
- https://www.tinkercad.com/embed/fypoZVpvuSa?editbtn=1

###  **Es23. Selezione luci**

Scrivere un programma che realizzi l'accensione di un led, due led o tre led tramite la pressione consecutiva di un pulsante 
una, due o tre volte all'interno di un intervallo temporale di un secondo.
Col la pressione prolungata dello stesso pulsante si spengono tutti e tre contemporaneamente.
(Realizzazione del timer senza schedulatore)
-  https://www.tinkercad.com/things/1EIcEp5BkZt-copy-of-selezione-luci-e-spegnimento-con-un-solo-tasto/editel?tenant=circuits

###  **Es24. Scegli chi con un tasto**
Scrivere un programma che realizzi l'accensione del led1, oppure del led2 oppure del led3 led 
tramite la pressione consecutiva di un pulsante una, due o tre volte all'interno di 
un intervallo temporale di un secondo. Col la pressione per almeno un secondo, ma meno di tre, dello stesso pulsante si 
accendono tutti i led, con la pressione dello stesso tasto per più di tre secondi si spengono tutti i led.
- https://www.tinkercad.com/things/92WnWXH0OvB-copy-of-scegli-chi-onoff-con-un-tasto/editel?tenant=circuits
  
###  **Es25. Scegli chi con due tasti**
Scrivere un programma che realizzi l'accensione del led1, oppure del led2 oppure del led3 led 
tramite la pressione consecutiva di un pulsante una, due o tre volte all'interno di 
un intervallo temporale di un secondo. Col la pressione di un altro pulsante si fa una cosa analoga per spegnerli.
- [tenant=circuits](https://www.tinkercad.com/things/1eJwTOD7t8K-copy-of-scegli-chi-onoff/editel?tenant=circuits)https://www.tinkercad.com/things/1eJwTOD7t8K-copy-of-scegli-chi-onoff/editel?tenant=circuits
  
### **Es26. Singolo nastro trasportatore**
1) Scrivere un programma che realizzi la gestione di un nastro traportatore attraverso la lettura di tre sensori di transito (barriere ad infrarossi) e il comando di un motore.
I sensori permangono al livello alto finchè un oggetto ingombra l'area del sensore, dopodichè vanno a livello basso. I sensori sono due all'inizio del nastro (uno per i pezzi bassi ed un'altro per quelli alti) ed uno alla fine del nastro che rileva il pezzo pronto per essere prelevato.
Esiste un timer di volo, così chiamato perchè dura esattamente per il tempo che impiegherebbe il pezzo più lontano dall'uscita, cioè quello sulla barriera di ingresso, per arrivare sulla barriera di uscita.
All'attivazione di un qualsiasi sensore di ingresso parte il motore e si resetta e pure si blocca il timer di volo.
All'attivazione del sensore di uscita si blocca il nastro, alla sua disattivazione riparte il nastro e parte il timer di volo.
Allo scadere del timer di volo si spegne il motore.
3) Aggiungere la funzione di blink di un led quando il nastro trasportatore è in movimento.
- https://www.tinkercad.com/things/bKP671nY2MU-copy-of-nastrouno/editel?tenant=circuits
- algoritmo nastro trasposrtatore [nastro](nastro.md)

### **Es27. Conta numeri**
Realizzare un programma Arduino che dati due led, led1 e led2 ed un tasto, conta il numero di volte che 
si preme il tasto e si accende il led1 quando si raggiungono 10 pressioni e quando si raggiungono le 20 
si accende il led2, mentre quando si raggiungono le 30 si spegne tutto e si ricomincia da 0. Se è possibile, 
rendere, in qualche modo, le pressioni insensibili ai rimbalzi del tasto.
- https://www.tinkercad.com/things/6toP0Dc3v0x-copy-of-conta-numeri-schedulato/editel?tenant=circuits

### **Es28. Timers HW 1**
1) Concepire un SW per ESP32 che realizzi un blink di due led con tempi diversi e l'accensione di un led dopo 5 secondi dalla pressione di un pulsante e il suo automatico spegnimento dopo 10 secondi dalla sua accensione.

   https://wokwi.com/projects/387795448199864321
   
4) Concepire un SW per ESP32 che realizzi un blink periodico di un led fino a che l'utente non preme un pulsante di spento. Con uno di acceso riprende. Realizzarlo in due modi:
usando solo .once_ms
usando solo .attach_ms

### **Es29. Semaforo**
Realizzare un semaforo utilizzando soltanto timer HW.

https://wokwi.com/projects/387790093070138369

### **Es30. Orchestra**

Scrivere un programma per esp32 che realizzi la gestione di 4 led:
uno che fa 100 bpm (battiti per munuto) 
uno che fa 25 bpm 
uno ogni tre battiti del primo led
Realizzare il tutto con la schedulazione generica basata sui get (no delay, no timer HW, no thread).

https://wokwi.com/projects/388980248362009601

### **Es31. Simulazione vita**

Scrivere un programma per esp32 che realizzi la gestione di un led che fa dieci blink da mezzo secondo e poi 5 da uno e ricomincia. Realizzare il tutto con la schedulazione basata sui get (no delay, no timer HW, no thread).

https://wokwi.com/projects/388978976877511681

### **Es32. Luminosità led con encoder rotativo**

Scrivere un programma per esp32 che realizzi la regolazione della luminosità di un led attraverso la variazione degli scatti di un encoder rotativo. Fare in modo che la luminosità sia nulla a zero scatti e massima a 20 (1 giro completo).

https://wokwi.com/projects/390527818943920129

### **Es33. Fading successivo di 3 led**

Scrivere un programma che accenda completamente un led dopo una rotazione completa di un encoder rotativo e, dopo, accenda completamente un altro led dopo 4 giri, ed accenda un terzo led dopo dieci giri. Rifare lo spegnimento con le operazioni contrarie.

https://wokwi.com/projects/390876776704923649

### **Es34. Navigazione mappa interrupt**

Consegnare un esempio completo per ogni scenario possibile di gestione presente nella mappa concettuale degli interrupt utilizzandoli tutti per accendere e spegnere un led mentre contemporaneamente ne blinka un'altro. Valutare e segnare pro e contro di ogni soluzione.

Interrupt:
- Con polling del rilascio (e debouncing) in un loop generato con timer HW: https://wokwi.com/projects/390920656468342785
- Con polling del rilascio (e debouncing) in un loop generato con timer SW: https://wokwi.com/projects/390921685594360833
- Con polling del rilascio (e debouncing) nel loop principale con le attese (delay): https://wokwi.com/projects/390922370464465921
- Con polling del rilascio (e debouncing) nella ISR: https://wokwi.com/projects/390923700676500481

### **Es35. Ancora fading successivo di 3 led**

Scrivere un programma che accenda completamente un led dopo due rotazioni complete di un encoder rotativo e, dopo, accenda completamente un altro led dopo 3 giri, ed accenda un terzo led dopo 5 giri. Rifare lo spegnimento con le operazioni contrarie.

https://wokwi.com/projects/390925418313291777

### **Es36. Navigazione mappa pulsanti**

Consegnare un esempio completo per ogni scenario possibile di gestione presente nella mappa concettuale dei pulsanti utilizzandoli tutti per accendere e spegnere un led (con o senza memoria)  mentre contemporaneamente ne blinka un'altro. Fare presente e segnare pro e contro di ogni soluzione.

- Pulsante senza memoria non responsivo https://wokwi.com/projects/391102164130146305
- Pulsante senza memoria responsivo con get https://wokwi.com/projects/391105027631947777
- Pulsante senza memoria responsivo con i thread https://wokwi.com/projects/391106418377249793
- Pulsante senza memoria responsivo con i timer HW https://wokwi.com/projects/391107103111045121
- pulsante con memoria bloccante  con delay: https://wokwi.com/projects/391108833341231105
- pulsante con memoria non bloccante con attivazione sul fronte di salita con get(): https://wokwi.com/projects/391108412935624705
- pulsante con memoria non bloccante  con attivazione sul fronte di discesa con get(): https://wokwi.com/projects/391110569888045057
- pulsante con memoria non bloccante  con i thread: https://wokwi.com/projects/391109183918999553
- pulsante con memoria non bloccante  con i timer HW: https://wokwi.com/projects/391109387683542017


### **Es37. Erogatore per gatti**

Automatizzare un erogatore di crocchette che vengono distribuite fa un ugello a tempo. 30 g sono erogati in 5 secondi. L'erogatore é attivato dalla pressione del musetto del gatto. Erogati il cibo l'erogatore si disattiva per un'ora (emulati con 10 secondi). Le erogazioni sono contate e non devono superare le 6. L'erogatore viene riabilitato (resettando il contatore) dal padrone una vilta al giorno premendo un pulsante.

Simulazione in Tinkercad: https://www.tinkercad.com/things/6A7FLApJYYA/editel

### **Es38. Trappola per topi**

Una scatola con una porta a scatto elettrica contiene un'esca. Quando il topo é entrato tutto la porta si chiude. Con un pulsante la porta si apre e fa uscire il topo a Topolinia.

NB: Il pulsante é premuto il topo sta entrando. Ho letto il fronte di salita e so che sta entrando. Leggo dopo il fonte di discesa, dello stesso pulsante (barriera luminosa) e so che é conpletamente entrato.

- Con toggle non bloccante e timer sul fronte di discesa su Tinkercad: https://www.tinkercad.com/things/jAM2qVg43KU/editel
- Con rilevazione del fronte di discesa e senza debounce su Tinkercad: https://www.tinkercad.com/things/5l6LDBg8K4g/editel
- Con rilevazione del fronte di discesa e senza debounce su Wokwi: https://wokwi.com/projects/392867901924989953



### **Es39. Macchina del caffè**

Un distributore automatico di caffè ha 4 pulsanti per erogare caffé. Se premi una volta il primo pulsante il caffè é corto, se premi la seconda volta é lungo. Il secondo pulsante sceglie lo zucchero il base al tempo della pressione (Se si vuole si può aumentare la luminosità di un led da 0 a 10 con il PWM se no si stampa con Serial.println()). Il terzo pulsante sceglie se decaffeinato oppure no. Il quarto serve caffè macchiato o meno. Ogni opzione binaria (con due valori) é segnalata dallo stato di un led. Il caffé é pronto dopo un minuto e un cicalino suona.

- **NB1:** il contatore di pressioni si può fare eseguendo ad ogni pressione (o rilascio) entrambe le cose seguenti:
il riarmo di un timer con t1.start() (tanto è attivo solo alla prima volta)
l'incremento del contatore
Allo scadere del timer viene eseguito il conteggio: se minore di uno, scelta A; se maggiore di 1, scelta B.

- **NB2:** La quantità di zucchero si può scegliere facendo partire un timer alla pressione di un pulsante e facendolo stoppare al suo rilascio. La lettura del get() del timer mi da la misura del ritardo. Decidendo che sopra 10 secondi le misure valgono sempre 10 (tetto superiore), un valore da 0 a 10 dice la quantità di zucchero da stampare con Serial.println(). Ricordarsi che il get() deve stare in un loop non bloccato per potere contare.

- https://www.tinkercad.com/things/7TvzaavTq4A/editel
- https://wokwi.com/projects/391440818569488385

### **Es40. Cassaforte**

Scrivere un programma che verifichi la combinazione di apertura di una cassaforte con un pomello rotante. La cassaforte si apre dopo 3 sequenze esatte segnalate dall'accensione successiva di 4 led. Le sequenze di scatti sono valutate con un encoder rotativo e visualizzate con una stampa su seriale. La combinazione segreta è 4 scatti a destra, 1 giro completo a sinistra, 3 scatti a destra. All'apertura della cassaforte si accende un led. Dopo 3 tentativi sbagliati suona un allarme sotto forma di cicalino continuo.

Suggerimenti:
- per il pulsante usare modalità INPUT_PULLUP e fare un debouncer con la funzione di urutils.h waitUntilInputHigh()
- Creare due array globali: uno per memorizzare la combinazione e uno per memorizzare la sequenza inserita dall'utente.
- Implementare la funzione controllo() che confronta i due array e rerstituisce vero se sono uguali cella per cella, falso saltrimenti (usare una variabile bandierina).

https://wokwi.com/projects/392701811094769665?authuser=0


### **Es41. Teiera**

Scrivere un programma che realizzi una teiera automatica tale che:
- si accenda premendo il pulsante A, alla cui pressione si mette a blinkare il led S di stato dell'acqua che così indica che la temperatura ancora non è quella giusta. 
- Una pressione del pulsante A mentre il led S blinka non ha alcun effetto.
- l'acqua è a temperatura quando, dopo 10 secondi, il led S non blinka più ma è a luminosità fissa
- premere il pulsante A con il  led S è fisso mette in azione la erogazione del te da un ugello, segnalata da un led E acceso
- l'erogazione termina dopo 3 secondi, passati i quali il led E si spegne.
- una pressione prolungata del pulsante A (più di 3 secondi) spegne la teiera disattivando tutti i led.

Link al progetto: [Teiera](teiera.md)

N.B.: Vedi :
- [Definizione macchine a stati finiti](statifinitibase.md)
- [Linee guida sviluppo macchine a stati finiti](statifinitisviluppo.md)

- Con esp32 su Wokwi: https://wokwi.com/projects/423804874250866689
- Con Arduino su Tinkercad: https://www.tinkercad.com/things/khLCxFsFdrk-teiera-automatica

### **Es42. Macchina per il caffè**

Realizzare una macchina per il caffè che funziona nella seguente maniera:
- Quando premo il pulsante 1 si accende il led A(accensione) da questo momento in poi l’acqua è in riscaldamento.
- Terminerà quando bollerà, all’incirca dopo 10s si accende il led B(acqua bolle) e viene abilitato il pulsante 2 è un toggle, alla prima pressione inizia l’erogazione indicata da un led E e termina l’erogazione con la seconda pressione e spengimento del led E.
(IL PRIMO PULSANTE ALLA SECONDA PRESSIONE SPENGNE LA MACCHINA ESSENDO UN TOGGLE).

Disegnare la mappa degli stati.

Link al progetto: [Macchina per il caffè](caffè.md)

- Simulazione con Arduino su Tinkercad: https://www.tinkercad.com/things/fK0ZdtcDER0-caffe

### **Es43. Lettiera automatica**

Scrivere un programma che realizzi una lettiera per gatti autopulente. Tre volte al giorno la lettiera si pulisce automaticamente attivando una 
centrifuga. La centrifuga viene avviata solamente quando si ha la certezza che il gatto è uscito dalla lettiera mediante la valutazione 
di un sensore di transito (pulsante con memoria) e di un sensore di presenza (deviatore a slitta senza memoria). Indicare la presenza 
del gatto con un led verde acceso. La pulizia della lettiera con un led rosso acceso. La pulizia si attiva automaticamente ogni 
tre volte che il gatto esce dalla lettiera.

Link al progetto: [Lettiera autopulente](lettiera.md)


- Simulazione con Arduino su Tinkercad mediante FSM su stati: https://www.tinkercad.com/things/kL0VNCCbB3B-lettiera-gatti
- Simulazione con Arduino su Tinkercad variante con FSM su ingressi: https://www.tinkercad.com/things/3F2C6FC6ozg-lettiera-gatti-no-fsm
- Simulazione con Arduino su Tinkercad variante con FSM su ingressi e senza delay: https://www.tinkercad.com/things/cYQtH4sy94O-lettiera-gatti-no-fsm-no-delay
- Simulazione con Arduino su Tinkercad variante con FSM su ingressi e senza delay ma con fronte di discesa rapido: https://www.tinkercad.com/things/7s1lQs9BxUi-lettiera-gatti-no-fsm-no-delay-e-disccesa-rapida
- Simulazione con Arduino su Tinkercad di una variante del precedente: https://www.tinkercad.com/things/49tCYEOMBP3-lettiera-gatti-no-fsm-no-delay-e-discesa-rapida-2


### **Es44. Palestra per cani**

 Realizzare una palestra per cani che funzioni in questo modo:
- un osso da mordere viene portato ad una certa altezza tramite un motore che agisce per 5 secondi. 
- il cane salta, afferra l'oggetto con i denti e fa scattare un interruttore (con memoria) che conta le prese.
- dopo tre prese con successo consecutive l'osso viene fatto salire di una altezza corrispondente ad altri 5 secondi
- il processo ricomincia fino a 3 step consecutivi, cioè 15 secondi complessivi.
- dopo l'osso scende ad un'altezza di 5 secondi e viene rilasciato un croccantino.
- tre led diversi (giallo, rosso e verde) indicano le tre altezze, un led indica i croccantini pronti.
- se entro dieci secondi il cane non  effettua un tentativo, si da per scontato che rinuncia e si riparte, 
  col conteggio delle prese azzerato, dal livello più basso.

Link al progetto: [Palestra per cani](palestracani.md)
 
Simulazione con Tinkercad con FSM su ingressi: https://www.tinkercad.com/things/hmOB6kgqlLS-palestra-per-cani

### **Es45. Autolavaggio**

Un'azienda di autolavaggi ha la necessità di automatizzare il processo di lavaggio dei veicoli per migliorare l'efficienza operativa e garantire un servizio uniforme. Il sistema deve gestire automaticamente le varie fasi del lavaggio, assicurandosi che il veicolo sia correttamente posizionato e che ogni fase venga completata prima di passare alla successiva.

Il sistema deve rilevare quando un veicolo entra nell'area di lavaggio e avviarsi solo quando il veicolo è fermo nella posizione corretta. Il ciclo di lavaggio comprende diverse fasi, tra cui prelavaggio di un minuto, lavaggio principale di 5 minuti e asciugatura di un minuto. Al termine del processo, il sistema deve notificare al conducente quando è possibile uscire.

Gli studenti dovranno:
- Identificare gli stati necessari per descrivere il funzionamento del sistema.
- Definire le transizioni tra gli stati in base agli eventi rilevati dai sensori e alle azioni richieste dal sistema.
- Individuare quali siano gli ingressi (input) e le uscite (output) che il sistema deve gestire.
- Implementare la logica della FSM in un microcontrollore (Arduino, ESP32 o altro) utilizzando un linguaggio di programmazione adeguato.
- Documentare il lavoro svolto con un diagramma a stati e una breve relazione che descriva il funzionamento del sistema e le scelte progettuali adottate.

Link al progetto: [Autolavaggio](autolavaggio.md)

### **Es46. Pulsante luci**

Scrivere un programma che realizzi una lampada intelligente
che funzioni in questo modo:
- Si accenda premendo il pulsante P1, alla cui pressione si attiva la luce a bassa intensità (stato 1) e si accende il LED indicatore L1.
- Una seconda pressione del pulsante P1 mentre la lampada è accesa a bassa intensità la porta a media intensità (stato 2), spegnendo L1 e accendendo L2.
- Una terza pressione del pulsante P1 porta la lampada ad alta intensità (stato 3), spegnendo L2 e accendendo L3.
- Una quarta pressione del pulsante P1 spegne completamente la lampada (stato 0), spegnendo tutti i LED indicatori.
- Se la lampada rileva che non ci sono movimenti per più di 5 minuti (tramite un sensore di movimento PIR), si spegne automaticamente passando allo stato 0.

Gli studenti dovranno:     
- Identificare gli stati necessari per descrivere il funzionamento del sistema.
- Definire le transizioni tra gli stati in base agli eventi rilevati dai sensori e alle azioni richieste dal sistema.
- Individuare quali siano gli ingressi (input) e le uscite (output) che il sistema deve gestire.
- Implementare la logica della FSM in un microcontrollore (Arduino, ESP32 o altro) utilizzando un linguaggio di programmazione adeguato.
- Documentare il lavoro svolto con un diagramma a stati e una tabella delle  transizioni.

Link al progetto: [Pulsante luci](luci.md)

**Sitografia:**
- https://docs.wokwi.com/parts/wokwi-ky-040
- https://www.lombardoandrea.com/utilizzare-un-encoder-rotativo-con-arduino/

>[Torna all'indice generale](index.md)
