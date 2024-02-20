>[Torna all'indice generale](index.md)

## **CONSIDERAZIONI GENERALI SUI PROBLEMI**

<img src="img\applicazMicro.jpg" alt="alt text" width="1000">

La logica degli algoritmi può essere pensata essenzialmente in due maniere: ad **eventi** e in modo **sequenziale**. 

Nei microcontrollori il **tempo** è un elemento portante della maggiorparte degli algoritmi. Il modo **sequenziale** di pensare il tempo in un algoritmo è sicuramente quello più intuitivo ed ha una sua naturale **corrispondenza** con le istruzioni di un programma. 

Infatti, in un listato sorgente, le istruzioni che **spazialmente** vengono dopo, sono eseguite dopo anche **temporalmente** (se non ci sono stati salti). Quantificare il **dopo** nel tempo tra una istruzione e l'altra è proprio il compito assegnato all'istruzione ```delay(x```), che non fa altro che bloccare l'esecuzione del loop principale (come di un qualsiasi loop secondario basato sui thread) per ```x``` millisecondi.

Finchè un programma consiste in un **unico task** avente un flusso temporale unico va tutto bene. Il modello lineare di programmazione è chiaro, intuitivo ed adeguato. Quando però, in un algoritmo coesistono più task separati, ciascuno composto da un proprio filo temporale che si interseca con quello degli altri, allora, il problema di codificarlo in un programma si complica. In questo caso, è quindi dirimente sapere se i task di cui si compone l'algoritmo sono **sequenziali** o procedono in maniera **parallela**, e se si, stabilire se, quando si incrociano, un task deve **aspettarne** qualcun altro prima di proseguire.

In linea di principio, se i task sono **sequenziali** nulla osta a metterli tutti nello **stesso loop** principale eseguendoli uno **di seguito** all'altro, dato che il loro sviluppo fisico nel codice è anche il loro sviluppo temporale.

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

## **TIMER UR**

E’ possibile realizzare dei timers, con cui programmare **nel futuro** lo stesso evento o una sequenza di eventi diversi, **senza attese**, cioè senza ```delay()```, eseguendo un polling del **tempo di conteggio** di un **timer**. 

Un **polling** è l'interrogazione periodica di una **variabile** o di un **ingresso** per leggerne il **valore**. Se il valore è maggiore di un **tempo massimo** allora si considera avvenuto un **timeout** e si esegue il **blocco** di codice associato ad esso.

Il polling serve per verificare, ad ogni loop(), che il tempo trascorso (**elapsed**) abbia raggiunto il **valore finale** (timeout) stabilito come obiettivo (target). Ciò si può ottenere eseguendo il **test** periodico di una **condizione** su una funzione di **misura** del tempo, nello specifico una ```get()```, che, ad ogni ```loop()```, restituisce il **tempo di conteggio**. Se la **condizione di uscita** dal conteggio è **vera** allora il **blocco** di istruzioni associato al **timeout del timer** viene **eseguito**.

Un **timeout** si controlla, valutando la **condizione di scadenza** del timer sulla funzione ```get()``` mediante una istruzione di selezione **if**. Ad esempio, eseguendo periodicamente (**polling**) nel ```loop()``` il controllo ```if(t.get() > 10000) {....}```, si può stabilire se, dal momento dell'attivazione del timer, sono pasasti 10 secondi e, in caso affermativo, eseguire le istruzioni nel blocco then che nel codice segue la condizione di scadenza.

La funzione ```get()```  **non è bloccante** (non causa alcun ritardo) e  **non interferisce** con nessun delay del loop corrente, sia esso il loop principale o quello secondario di un thread. Essa rappresenta, quindi, un **task** che si può tranquillamente adoperare in **sequenza** ad altri task di uno stesso loop, anche se questi contengono dei ritardi ```delay()```.

Il **polling** della funzione ```get()``` può essere effettuato in qualunque punto del loop, anche più volte se necessario, in genere si esegue per:
-  se il timer **è attivo**, controllare una **scadenza**, cioè vedere se è passato il tempo necessario per compiere una certa **azione** (modificare una variabile o chiamare una funzione)
-  se il timer **non è attivo**, impostare una **cadenza**, cioè se il timer è stato **bloccato** su un certo tempo con uno ```stop()```, utilizzare quel valore **programmato** di tempo per impostare dinamicamente un **ritardo delay**, oppure per confrontarlo con altri tempi che si stanno **accumulando** su una variabile all'interno del loop.  

Si tratta di un **pattern** (tipo di API) per la realizzazione di timers **molto comune** nella programmazione di **bracci robotici** per uso industriale (vedi bracci Universal Robots) che si adatta bene ad essere impiegato sia in un contesto in cui la logica dell'algoritmo è realizzata in maniera sequenziale sia in quello in cui gli input sono gestiti con un modello ad eventi.

**In** **generale**, possiamo individuare alcune **componenti del timer**:

- **reset** del timer. Accade al verificarsi di una certa **condizione**. Determina sia l'azzeramento del **tempo di conteggio** che l'azzeramento della misura del ritardo dall'ultimo riavvio. L'azzeramento **non** è mai automatico ed è necessario impostarlo prima del riutilizzo del timer dopo uno o più comandi ``stop()`` (tipicamente al momento della valutazione della condizione di timeout) altrimenti è sufficiente richiamare la funzione ```start()```.  
- **start**. Avvia o riavvia il timer quando si avvera la **condizione di attivazione** posta in punto qualsiasi del loop. Imposta il flag di stato ```timerState```. 
- **stop** del timer. Accade al verificarsi di una certa **condizione** di sospensione del timer posta in punto qualsiasi del loop. In questa occasione il timer **campiona** il ritardo accumulato dall'ultimo riavvio e lo somma al **tempo di conteggio** cumulato in corrispondenza della sospensione precedente. In questo modo il tempo trascorso è memorizzato e può essere valutato in un momento successivo nel codice tramite la funzione ```get()```.
- **get** del tempo **tempo di conteggio** (elapsed) dal momento del **primo avvio** del timer. E' una funzione **non bloccante**, cioè non genera attese dato che restituisce immediatamente il suo risultato. Viene **interrogata** periodicamente (polling) ad ogni ciclo di **loop** (meno se decimato ogni tot loop) e serve a realizzare una espressione di confronto con un tempo di **timeout** (```var.get() > timeout```). Il **valore restituito** dipende dallo stato del timer:
     - se **```timerState == false```** allora restituisce l'ultimo **tempo di conteggio** misurato e memorizzato al momento della chiamata a stop() .
     - se **```timerState == true```** allora calcola il **tempo di conteggio** corrente campionando il ritardo attuale dall'ultimo riavvio e sommandolo ai ritardo cumulato prima dell'ultima sospensione.
- **istruzioni triggerate** (scatenate) dal timer. Vengono eseguite in base al **tempo di conteggio misurato** dal timer. Vengono eseguite in maniera sequenziale in un punto stabilito del loop (istruzioni sincrone) in corrispondenza della **verità** di una certa **condizione** sul tempo di conteggio che coinvolge la funzione ```get()``` come operando. In corrispondenza si dovrebbero eseguire:
  -    **reset()**  per azzerare il timer.
  -    **stop()** per bloccare il timer.

Per consultare dettagli sulla sua implementazione vedi [timer sequenziali](polledtimer_seq.md).

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

<img src="img\pulsanti.jpg" alt="alt text" width="1000">

### **Toggle basato sul polling del tempo corrente get()**

In questo caso, il **rilevatore dei fronti** è realizzato **campionando** il valore del livello al loop di CPU **attuale** e **confrontandolo** con il valore del livello campionato al **loop precedente** (o a uno dei loop precedenti). Se il valore attuale è HIGH e quello precedente è LOW si è rilevato un **fronte di salita**, mentre se il valore attuale è LOW e quello precedente è HIGH si è rilevato un **fronte di discesa**.  

Pulsante toggle che realizza blink e  antirimbalzo realizzato con una **schedulazione ad eventi senza ritardi (time tick)**:
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
    if(!pressed){ // intervento immediato ma sul primo fronte di salita soltanto (causa disarmo pulsante)
        pressed = true; // disarmo del pulsante e riarmo del timer di debouncing
        stato = !stato; // logica da attivare sul fronte 
    }
}   // end of switchPressed

void waitUntilInputChange()
{
    if (pressed){ 
      debounce.start();// aggiorna il millis() interno solo alla prima di molte chiamate consecutive
      if(debounce.get() > DEBOUNCETIME  && digitalRead(pulsante) == LOW){
        pressed = false; // riarmo del pulsante
        debounce.stop(); // disarmo del timer
        debounce.reset();
      }
    }
}
// loop principale
void loop() {
	waitUntilInputChange();
	if (stato) {
		digitalWrite(led, !digitalRead(led));   	// inverti lo stato precedente del led
		delay(1000);
	} else {
		digitalWrite(led, LOW);    	// turn the LED off by making the voltage LOW
    		delay(10);
	}
}
```
Simulazione online su Esp32 con Wowki del codice precedente: https://wokwi.com/projects/388481409829351425

### **Schedulatore generico realizzato con funzione get()**

Il codice precedente è basato su due timers SW indipendenti nel senso che ognuno realizza il polling del tempo corrente separatamente ad ogni ciclo di loop(), cioè, alla massima velocità consentita dal sistema. Se i polling sono in numero limitato questa soluzione non crea grossi problemi ma se invece i timers sono parecchi, allora potrebbe capitare che il carico computazionale del polling possa diventare comparabile al peso dei vari task eseguiti.

E' di aiuto in questo caso l'utilizzo di uno **schedulatore** dei compiti, ovvero di un codice che, a partire da un **tempo base comune**, sappia **programmare nel tempo** (schedulare) i vari compiti (task) affinchè vengano **eseguiti** esattamente nel tempo **assegnato** a ciascuno.

Il **timer schedulatore** è realizzato utilizzando una variante ```DiffTimer2``` del timer della libreria [urutils.h](urutils.h) in cui, ad ogni chiamata della funzione ```get()```, viene incrementato il tempo corrente di ciascun timer (elapsed) di una quantità fissa pari al **tempo base**. Il momento dell'incremento è contestuale a quello della chiamata a ```get()``` che, quindi, deve avvenire **ogni tempo base**. Il tempo base è il **M.C.D.** dei tempi **in gioco** nei vari task.

Il **timer schedulatore** è diverso per ciascun task, ma tutti i timer di questo tipo hanno a comune lo stesso tempo base in base al quale incrementano internamente il proprio **tempo corrente**. Il tempo corrente (elapsed) è calcolato separatamente per ogni timer schedulatore e quindi **azzerato**, tramite la funzione ```reset()```, con una **periodicità** che è **diversa** per ciascuno. 

La **periodicità** è stabilita dalla **condizione** sulla funzione ```get()``` che valuta l'avvenuto scadere del **timeout**. Il **polling** sulla condizione di timeout non avviene alla massima velocità del microprocessore (variabile da pezzo a pezzo) ma, in maniera più precisa e, soprattutto, più rada, ad ogni **tempo base**.

Il **timer schedulatore** non possedendo un riferimento che tiene il tempo internamente ad esso (ad es. una millis()), deve essere richiamato da una funzione esterna esattamente allo scoccare di ogni **tempo base**.

Il **tempo base** viene generato utilizzando il timer ```DiffTimer1``` che realizza, mediante il polling della sua funzione ```get()```, il polling della funzione ```millis()``` che restituisce il **tempo corrente** del sistema. 

Il **tempo base** viene **comunicato** a ciascun timer schedulatore attraverso un **argomento** della funzione ```get()```.

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
DiffTimer2 tmrblink1, tmrblink2;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(pulsante, INPUT);
  precval=LOW;
  tmrdeb.start();
  tmrblink2.start();
  tmrblink1.setBase(tbase);
  tmrblink2.setBase(tbase);
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
    if (tmrblink1.get() > 500) {
      digitalWrite(led, !digitalRead(led));
      tmrblink1.reset();
    } 
    //task_blink2
    if (tmrblink2.get() > 1000) {
      digitalWrite(led2, !digitalRead(led2));
      tmrblink2.reset();
    } 
  }
  delay(10);
}
```
Di seguito il link della simulazione online con ESP32 su Wokwi: https://wokwi.com/projects/388359604541585409

Si noti che l'**ordine di apparizione** dei task all'interno del codice del loop potrebbe non rispecchiare l'**ordine** con cui gli stessi vengono **eseguiti**, cioè l'ordine di esecuzione avviene in maniera **asincrona** rispetto al codice, secondo il principio della cosidetta programmazione ad eventi. Eventi possono attivare o disattivare altri eventi in base ad input esterni oppure in base al valore raggiunto da certe variabili di stato.

Questo esempio conclude tutte le **tecniche** possibili per programmare eventi nel tempo in un sistema a microprocessore che quindi potrebbero essere riassunte in: **delay** nel loop(), delay nei **thread**, **timer SW** nel loop(), **timer HW**, timer schedulatori per progettare complessivamente uno **scheduler** che realizzi la tecnica del multitasking a partire da un **riferimento** temporale **esterno** scelto tra millis(), delay(), interrupts di timer HW.

<img src="img\interruptPulsante.jpg" alt="alt text" width="1000">

### **Pulsante toggle basato su interrupts e timer HW**

Per una discussione generale sugli interrupt si rimanda a [interrupt](indexinterrupts.md).

All'**ingresso** di una **porta digitale** viene associata una callback che viene invocata alla ricezione di un segnale di interrupt attivo su entrambi i fronti. Il fronte di salita, **selezionato** prendendo solo i valori HIGH, potrebbe essere rilevato molte volte consecutivamente a causa del fenomeno dei rimbalzi. Per evitare la rilevazione dei **fronti spuri** viene realizzata la disabilitazione della loro rilevazione asserendo il **flag di disabilitazione** ```pressed``` alla ricezione del **primo fronte**, contemporaneamente viene avviato, in modalità one shot, il timer per la rilevazione del fronte di discesa, fissando un tempo congruo di almeno 50 msec affinchè sia data il tempo al transitorio dei rimbalzi di estinguersi completamente.

Allo scadere del timeout viene eseguita la callback ```waitUntilInputLow()``` all'interno della ISR del timer. La funzione esegue una nuova lettura del valore della porta:
- se è **HIGH** allora deduce che il pulsante è **ancora premuto** e decide di aspettare ancora altri 50 msec riavviando il timer di rilevazione del fronte di discesa
- se è **LOW** allore deduce che il pulsante è **stato rilasciato** e decide di riabilitarlo ad una nuova pressione (riarmo) disasserendo il fag di disabilitazione ```pressed```.

La funzione di **debouncing** è garantita introducendo un **tempo minimo** di attesa tra un campionamento e l'altro.

Per mantenere la ISR chiamante il più veloce possibile, viene spostato nel ```loop()```  l'algoritmo di blibk basato sui ```delay()```, dove può fare il suo lavoro industurbato essendo l'unico task (suscettibile ai ritardi) presente. 

Le attese sono tutte **non bloccanti** e realizzate tramite un timer HW che adopera esso stesso gli **interrupt** per richiamare la funzione di servizio (callback) da eseguire allo scadere del **timeout**. Il timer, utilizzando gli interrupt, è in grado di intervenire in tempo in **tutte le situazioni**, eventualmente anche interrompendo l'esecuzione di eventuale codice che impegnasse intensamente il loop(). Si tratta sicuramente di una realizzazione che, avendo la massima efficacia possibile in tutte le situazioni, si presta alla realizzazione di **dispositivi di sicurezza**.

```C++
#include <Ticker.h>
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
int led = 13;
byte pulsante =12;
volatile bool pressed;
volatile bool stato;
#define DEBOUNCETIME 50
Ticker debounceTicker;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsante), switchPressed, RISING );
  pressed = false;
  stato = false;
}

void switchPressed ()
{
  if(!pressed){ // fronte di salita
    pressed = true; // disarmo il pulsante
    debounceTicker.once_ms(50, waitUntilInputLow);  // riarmo del timer
    stato = !stato; 	 // logica da attivare sul fronte (toggle)
  }
}  // end of switchPressed

void waitUntilInputLow()
{
    if (digitalRead(pulsante) == HIGH)//se il pulsante è ancora premuto
    { 
      debounceTicker.once_ms(50, waitUntilInputLow);  // riarmo del timer
    }else{
      pressed = false; // riarmo del pulsante e disarmo (automatico) del timer
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
Di seguito il link della simulazione online con ESP32 su Wokwi: https://wokwi.com/projects/388438685024222209

### **Pulsante toggle basato su interrupts e timer debounce get()**

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
    if(!pressed){ // intervento immediato ma sul primo fronte di salita soltanto (causa disarmo pulsante)
        pressed = true; // disarmo del pulsante e riarmo del timer di debouncing
        stato = !stato; // logica da attivare sul fronte (toggle)
    }
}   // end of switchPressed

void waitUntilInputChange()
{
    if (pressed){ 
      debounce.start();// aggiorna il millis() interno solo alla prima di molte chiamate consecutive
      if(debounce.get() > DEBOUNCETIME  && digitalRead(pulsante) == LOW){// disarmo del timer al timeout
        pressed = false; // riarmo del pulsante
        debounce.stop();
	debounce.reset();
      }
    }
}
// loop principale
void loop() {
	waitUntilInputChange();
	if (stato) {
		digitalWrite(led, !digitalRead(led));   	// inverti lo stato precedente del led
		delay(1000);
	} else {
		digitalWrite(led, LOW);    	// turn the LED off by making the voltage LOW
   		 delay(10);
	}
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
- Simulazione online su ESP32 di una del codice precedente con Wowki: https://wokwi.com/projects/389923052943921153

Le variabili **condivise** tra una ISR e il loop() andrebbero protette, da accessi **paralleli** e concorrenti da parte di entrambe, tramite delle **corse critiche** che rendano l'accesso **strettamente sequenziale**. Inoltre le variabili condivise devono sempre essere dichiarate con il qualificatore ```volatile``` per forzarne la modifica istantanea anche sui registri della CPU. 

Gli **accessi paralleli** non sono un problema quando le **istruzioni** sono **atomiche**, cioè non interrompibili. Le istruzioni atomiche o sono eseguite per intero o non sono eseguite affatto. In questo caso gli **accessi**, sia in lettura che in scrittura, sono in realtà, a basso livello, **intrinsecamente sequenziali**.

Nei microcontrollori attuali, in genere **nessuna istruzione** gode della proprietà di essere **atomica** con una sola eccezione per la lettura e scrittura delle **variabili ad 8 bit**. Per le variabili codificate con **8 bit** l'accesso a basso livello (linguaggio macchina) è intrinsecamente garantito essere **atomico**. Per queste variabili rimane comunque la necessita dell'uso del qualificatore ```volatile```.

Le **modifiche** a valori con codifiche **maggiori di 8 bit** sono in genere **non atomiche**, pertanto le variabili a 16 o 32 bit andrebbero gestite con gli interrupt disabilitati (sezione critica). Tuttavia, gli interrupt vengono disabilitati di default durante una routine di servizio di interrupt, quindi, non potendo verificarsi il danneggiamento di una variabile multibyte in una ISR, le **sezioni critiche** vanno inserite soltanto nel ```loop()```.

Le variabile **condivisa** tra ISR e loop() e **8 bit** è ```stato``` ed è stata dichiarata come ```volatile``` e ogni suo accesso (in lettura e scrittura) è  stato racchiuso in una  **sezione critica** dove l'acccesso parallelo è stato disabilitato in favore di uno **strettamente sequenziale** che garantisce l'atomicità delle istruzioni in essa racchiuse.

Per quanto riguarda le sezioni critiche si può approfondire in [sezioni critiche](intsezionicritiche.md)

Gli encoder differenziali, poichè internamente sono dei normalissimi interruttori, sono soggetti anch'essi aal fenomeno dei rimbalzi e all'usura che progressivamente accentua quel fenomeno. Per applicazioni pratiche effettive, è conveniente utilizzare delle implementazioni capaci di tenerne conto. Per questo e altro vedere <a href="encoder.md">Encoder rotativo</a>


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


Sitografia:
- https://docs.wokwi.com/parts/wokwi-ky-040
- https://www.lombardoandrea.com/utilizzare-un-encoder-rotativo-con-arduino/
  
>[Torna all'indice generale](index.md)
