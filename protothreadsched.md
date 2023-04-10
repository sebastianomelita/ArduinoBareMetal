
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  

# **SCHEDULAZIONE CON I PROTOTHREAD**

### **Base teorica**

I protothread sono un tipo di thread leggeri senza stack progettati per sistemi con vincoli di memoria severi come sottosistemi embedded o
nodi di reti di sensori. I protothreads possono essere usati con o senza un RTOS. 

I **protothread** forniscono un meccanismo di **blocco dei task** (compiti) in cima a un **sistema ad eventi**, senza l'overhead di uno stack per ogni thread. Lo **scopo** dei protothread è quello di implementare un **flusso sequenziale di controllo** senza utilizzare complesse macchine a stati finiti ed evitando l'overhead di un multi-threading completo, cioè quello dotato anche del **modo preemptive**.  I protothread forniscono la sola **modalità cooperativa** e il **rilascio anticipato** delle risorse è realizzato **senza l'utilizzo di interrupt** che generino il **cambio di contesto** dei thread. 

In altre parole, la **gestione dei task** è **ad eventi** all'interno di un **singolo thread reale** ma, tramite delle **macro**, un algoritmo si può programmare in **maniera sequenziale** all'interno di funzioni che girano su dei **thread logici**, cioè emulati. La **concorrenza** dei task è gestita da uno **schedulatore** che, invocato durante il periodo di **sleep** di un task, o tramite il comando **yeld**, assegna la risorsa CPU a quei task che, in quell'intervallo, ne fanno richiesta.

### **Confronto con le altre tecniche**

Anche i **processi** e i **thread** sono flussi di esecuzione indipendenti che procedono in parallelo su una o più CPU, esiste però una **differenza pratica** notevole tra di essi:
- nei **processi** sia input/output, che **area dati globale** che **area dati locale** (stack) sono indipendenti e separate in zone diverse della memoria RAM.
- nei **thread**  input/output e **area dati globale** sono **in comune** nella stessa posizione in RAM mentre soltanto le **aree dati locali** (stack) sono indipendenti e separate in zone diverse della memoria RAM.
- nei **protothread** sia input/output che l'**area dati globale** ma anche le **aree dati locali** (stack) sono **in comune** nella stessa posizione in RAM. Pertanto le **variabili locali** non sono isolate in thread diversi (presenza di possibili ambiguità).

### **Utilizzo in pratica**

**Ogni protothread** realizza un **flusso** di esecuzione **parallelo** a quello degli altri thread, inoltre ognuno possiede un proprio **loop() principale** di esecuzione in cui realizzare le operazioni che tipicamente riguardano le **tre fasi** di lettura degli ingressi, calcolo dello stato del sistema e della sua risposta e la fase finale di scrittura della risposta sulle uscite. Il loop principale è definito sotto forma di **ciclo infinito** come ad esempio:

```C++
while(true) {
    // codice del protothread
    .........................
}
```
Le **fasi di lavoro** del loop possono essere **schedulate** (pianificate nel tempo) dal delay() non bloccante ```PT_SLEEP(pt)``` e dal rilascio spontaneo ```PT_YIELD(pt)```che permettono la progettazione **lineare** di un algoritmo nel tempo.

**Ogni protothread** è definito da un **descrittore** che è una variabile di tipo struct, cioè il tipo record del C, che rappresenta il protothread. Il **nome** del descrittore è arbitrario a discrezione del programmatore. Il descrittore deve essere passato come **argomento** ad ogni **comando** (macro) della libreria protothread. Il descrittore può essere definito **esattamente prima** della definizione della funzione del protothread tramite la dichiarazione **```pt ptNome_descr```**;

Il **flusso di esecuzione** di un protothread è **definito** all'interno di una **funzione** e può essere avviato passando allo schedulatore il riferimento a questa funzione sotto la forma di parametro. In sostanza la funzione **serve** al programmatore per definire il protothread e allo schedulatore per poterlo richiamare. All'interno della funzione il protothread deve sempre cominciare con il comando **```PT_BEGIN(pt)```** e deve sempre terminare con il comando  **```PT_END(pt)```**. 

In definitiva la **dichiarazione e definizione** di **descrittore e funzione** del protothread possono assumere la forma:

```C++
pt ptMioScopo;
int mioScopoThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  while(true) {
	// codice del protothread
	.........................
  }
  PT_END(pt);
}
```

Ogni protothread è **inizializzato** nel **setup()** tramite la funzione **```PT_INIT(&ptMioScopo)```**, il passaggio del descrittore è per **riferimento** perchè questo deve poter essere **modificato** al momento della inizializzazione.
```C++
void setup() {
  PT_INIT(&ptMioScopo1);
  PT_INIT(&ptMioScopo2);
}
```

Ogni protothrad viene **schedulato** cioè, valutato periodicamente per stabilire se deve essere eseguito o meno, all'interno del **```loop()```** tramite il comando **```PT_SCHEDULE(mioScopoThread(&ptMioScopo))```** che ha per argomento la funzione che definisce il protothread.

```C++
void loop() {
	PT_SCHEDULE(mioScopo1Thread(&ptMioScopo1));
	PT_SCHEDULE(mioScopo2Thread(&ptMioScopo2));
}
```

Per quanto riguarda la **definizione** di un protothread va ricordato che ll'interno del loop del protothread ogni **ramo** di esecuzione va reso **non bloccante** inserendo, la funzione **```PT_SLEEP(pt)```** (mai la normale delay()) se il flusso di esecuzione deve essere **bloccato temporaneamente** per un certo tempo fissato, oppure la funzione **```PT_YIELD(pt)```** se questo **non deve essere bloccato**. Ciò serve a richiamare lo schedulatore **almeno una volta**, qualunque **direzione** di  esecuzione prenda il codice, in modo da cedere **"spontaneamente"** il controllo ad un altro prothread al termine del loop() del prothread corrente. La **cessione del controllo** dello schedulatore ad ogni ramo di esecuzione **è necessario** altrimenti gli altri protothread non verrebbero mai eseguiti (il sistema **non è preemptive**).
Sia ```PT_YIELD(pt)``` che ```PT_SLEEP(pt)``` cedono il controllo della CPU allo schedulatore che lo assegna agli altri protothread che eventualmente in quel momento hanno scaduto il tempo di attesa di un loro delay.

**Blink sequenziali interagenti**

Di seguito è riportato un esempio di **blink sequenziale** in esecuzione su **due thread** separati su scheda **Arduino Uno**, con **IDE Arduino** e  con la libreria **protothread.h**  (https://gitlab.com/airbornemint/arduino-protothreads). I thread sono senza stack e **non preemptive** (solo collaborativi). La **programmazione sequenziale** del blink del led è **emulata** tramite una funzione delay() **non bloccante** ``` PT_SLEEP(pt, 200) ``` fornita dalla libreria ``` protothreads.h ```.


```C++
/*
  Blink
  Turns an LED on for one second, then off for one second, repeatedly. Rewritten with Protothreads.
*/
/*
#define LC_INIT(lc)
struct pt { unsigned short lc; };
#define PT_THREAD(name_args)  char name_args
#define PT_BEGIN(pt)          switch(pt->lc) { case 0:
#define PT_WAIT_UNTIL(pt, c)  pt->lc = __LINE__; case __LINE__: \
                              if(!(c)) return 0
#define PT_END(pt)            } pt->lc = 0; return 2
#define PT_INIT(pt)   LC_INIT((pt)->lc)
#define PT_SLEEP(pt, delay) \
{ \
  do { \
    static unsigned long protothreads_sleep; \
    protothreads_sleep = millis(); \
    PT_WAIT_UNTIL(pt, millis() - protothreads_sleep > delay); \
  } while(false); \
}
#define PT_EXITED  2
#define PT_SCHEDULE(f) ((f) < PT_EXITED)
#define PT_YIELD(pt) PT_SLEEP(pt, 0)
//-----------------------------------------------------------------------------------------------------------
// se si usa questa libreria al posto delle macro sopra, togliere il commento iniziale all'include 
// e commentare le macro sopra
*/
#include "protothreads.h"

bool blink1_running = true;
bool blink2_running = true;
int led1 = 13;
int led2 = 12;
int count = 0;

pt ptBlink1;
int blinkThread1(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  while(true) {
	if (blink1_running == true) {
		digitalWrite(led1, HIGH);   // turn the LED on (HIGH is the voltage level)
		PT_SLEEP(pt, 500);
		digitalWrite(led1, LOW);    // turn the LED off by making the voltage LOW
		PT_SLEEP(pt, 500);
		count += 1;
		Serial.println(count);
	} else {
		digitalWrite(led1, LOW);    // turn the LED off by making the voltage LOW
		PT_YIELD(pt);
	}
  }
  PT_END(pt);
}

pt ptBlink2;
int blinkThread2(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  while(true) {
	if (blink2_running == true) {
		digitalWrite(led2, HIGH);   // turn the LED on (HIGH is the voltage level)
		PT_SLEEP(pt, 1000);
		digitalWrite(led2, LOW);    // turn the LED off by making the voltage LOW
		PT_SLEEP(pt, 1000);
	} else {
		digitalWrite(led2, LOW);    // turn the LED off by making the voltage LOW
		PT_YIELD(pt);
	}
  }
  PT_END(pt);
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  PT_INIT(&ptBlink1);
  PT_INIT(&ptBlink2);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  int count = 0;
}

// the loop function runs over and over again forever
void loop() {
	PT_SCHEDULE(blinkThread1(&ptBlink1));
	PT_SCHEDULE(blinkThread2(&ptBlink2));
  
	if(count >= 10){
		Serial.println("Ending threads...");
		blink1_running = false;
		blink2_running = false;
	}
}
```
Link simulazione online: https://wokwi.com/projects/348712126874911315

Osservazioni:
- il codice non è specifico di alcuna macchina, è realizzato in C puro ed è altamente portabile.
- Un protothread viene eseguito all'interno di una singola funzione C e non può estendersi ad altre funzioni. Un protothread può chiamare normali funzioni C, ma non può bloccore all'interno di una funzione chiamata da altre funzioni. Il blocco all'interno di chiamate di funzioni annidate
è invece ottenuto generando un protothread separato per ciascuna funzione potenzialmente bloccante. Il vantaggio di questo approccio è che
il blocco è esplicito: il programmatore sa esattamente quali funzioni un protothread blocca e quali invece non blocca mai.
- poichè i protothread non salvano il contesto dello stack durante la chiamata di una funzione bloccante, ad es. una delay(), le **variabili locali** non vengono conservate quando protothread blocca un task. Ciò significa che le variabili locali dovrebbero essere **usate con la massima
attenzione**. In caso di dubbio, non utilizzare variabili locali all'interno di un protothread!
- Un protothread è guidato da ripetute chiamate alla funzione in cui il protothread è in esecuzione. Ogni volta che viene chiamata la funzione, il 
protothread verrà eseguito fino a quando non si blocca o esce. Pertanto la schedulazione dei protothreads viene eseguita dall'applicazione che utilizza i protothreads.

**Pulsante responsivo + blink**

Di seguito è riportato un esempio su scheda **Arduino Uno**, con **IDE Arduino** e  con la libreria **protothread.h** di un **blink sequenziale** in esecuzione su **un thread** e di gestione del pulsante sul **loop principale**. I ritardi **sleep** agiscono sul thread secondario ma non bloccano la lettura dello stato del pulsante che rimane responsivo nell'accendere il secondo led durante entrambe le fasi del blink del primo led.
(Link simulatore online: https://wokwi.com/projects/348713258463527506)

```C++
#include "protothreads.h"

bool blink1_running = true;
int led1 = 13;
int led2 = 12;
byte pulsante=2;

// definizione protothread del lampeggio
pt ptBlink1;
int blinkThread1(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop secondario protothread
  while(true) {
	digitalWrite(led1, HIGH);   // turn the LED on (HIGH is the voltage level)
	PT_SLEEP(pt, 500);			// delay non bloccanti
	digitalWrite(led1, LOW);    // turn the LED off by making the voltage LOW
	PT_SLEEP(pt, 500);			// delay non bloccanti
  }
  PT_END(pt);
}

// the setup function runs once when you press reset or power the board
void setup() {
  PT_INIT(&ptBlink1);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
}

// the loop function runs over and over again forever
void loop() { // loop principale
	PT_SCHEDULE(blinkThread1(&ptBlink1)); // esecuzione schedulatore protothreads
    
	// gestione pulsante
	if (digitalRead(pulsante) == HIGH) {
		// turn LED on:
		digitalWrite(led2, HIGH);
	}else{
		// turn LED off:
		digitalWrite(led2, LOW);
	}
}
```

Lo schedulatore dei protothred può essere utilizzato pure su piattaforme di emulazione (ad es. Tinkercad) senza la necessità di includere librerie, basta inserire in cima al codice la definizione delle macro dello schedularore.

**SOS + blink**

Link simulazione su Arduino in Tinkercad: https://www.tinkercad.com/embed/2duYFwQzJgf?editbtn=1

Simulazione su Esp32 in Wowki: https://wokwi.com/projects/348709819084964435

```C++
/*
Realizzare il blink di un led insieme al lampeggio di un'altro led che codifica il messaggio morse dell'SOS.
*/
#define LC_INIT(lc)
struct pt { unsigned short lc; };
#define PT_THREAD(name_args)  char name_args
#define PT_BEGIN(pt)          switch(pt->lc) { case 0:
#define PT_WAIT_UNTIL(pt, c)  pt->lc = __LINE__; case __LINE__: \
                              if(!(c)) return 0
#define PT_END(pt)            } pt->lc = 0; return 2
#define PT_INIT(pt)   LC_INIT((pt)->lc)
#define PT_SLEEP(pt, delay) \
{ \
  do { \
    static unsigned long protothreads_sleep; \
    protothreads_sleep = millis(); \
    PT_WAIT_UNTIL(pt, millis() - protothreads_sleep > delay); \
  } while(false); \
}
#define PT_EXITED  2
#define PT_SCHEDULE(f) ((f) < PT_EXITED)
#define PT_YIELD(pt) PT_SLEEP(pt, 0)
//-----------------------------------------------------------------------------------------------------------
// se si usa questa libreria al posto delle macro sopra, togliere il commento iniziale all'include 
// e commentare le macro sopra
//#include "protothreads.h"
#define PDELAY  500
#define LDELAY  1500
#define BLINKDELAY  100
byte led1 = 12;
byte led2 = 13;

void setup()
{
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  PT_INIT(&ptSos);
  PT_INIT(&ptBlink);
}
 

// definizione protothread del pulsante
pt ptSos;
int SOSThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop del protothread
  while(true) {
	// 3 punti
	digitalWrite(led1, HIGH); // 1
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, HIGH); // 2
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, HIGH); // 3
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	// 1 linea
	digitalWrite(led1, HIGH); // 1 linea
	PT_SLEEP(pt, LDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	// 3 punti
	digitalWrite(led1, HIGH); // 1
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, HIGH); // 2
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, HIGH); // 3
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, LDELAY);
  }
  PT_END(pt);
}

// definizione protothread del lampeggio
pt ptBlink;
int blinkThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop del protothread
  while(true) {
	digitalWrite(led2, HIGH);   	// turn the LED on (HIGH is the voltage level)
	PT_SLEEP(pt, BLINKDELAY);
	digitalWrite(led2, LOW);    	// turn the LED off by making the voltage LOW
	PT_SLEEP(pt, BLINKDELAY);
  }
  PT_END(pt);
}

// loop principale
void loop()
{
	PT_SCHEDULE(SOSThread(&ptSos)); 		// esecuzione schedulatore protothreads
	PT_SCHEDULE(blinkThread(&ptBlink)); 	// esecuzione schedulatore protothreads
}
```

**Lampeggi multipli**

LInk simulazione in Tinkercad: https://www.tinkercad.com/embed/gCkCBuwlY8E?editbtn=1

```C++
/*
Realizzare il blink di un led insieme al lampeggio di un'altro led che codifica il messaggio morse dell'SOS.
*/
#define LC_INIT(lc)
struct pt { unsigned short lc; };
#define PT_THREAD(name_args)  char name_args
#define PT_BEGIN(pt)          switch(pt->lc) { case 0:
#define PT_WAIT_UNTIL(pt, c)  pt->lc = __LINE__; case __LINE__: \
                              if(!(c)) return 0
#define PT_END(pt)            } pt->lc = 0; return 2
#define PT_INIT(pt)   LC_INIT((pt)->lc)
#define PT_SLEEP(pt, delay) \
{ \
  do { \
    static unsigned long protothreads_sleep; \
    protothreads_sleep = millis(); \
    PT_WAIT_UNTIL(pt, millis() - protothreads_sleep > delay); \
  } while(false); \
}
#define PT_EXITED  2
#define PT_SCHEDULE(f) ((f) < PT_EXITED)
//-----------------------------------------------------------------------------------------------------------
// se si usa questa libreria al posto delle macro sopra, togliere il commento iniziale all'include 
// e commentare le macro sopra
//#include "protothreads.h"
#define DELAY1  1000
#define DELAY2  500
#define DELAY3  300
byte led1 = 10;
byte led2 = 11;
byte led3 = 12;
byte led4 = 13;

void setup()
{
  pinMode(led1, OUTPUT);
  
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  PT_INIT(&ptBlink2);
  PT_INIT(&ptBlink1);
}
 

// definizione protothread del pulsante
pt ptBlink1;
int blink1Thread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop del protothread
  while(true) {//for(;;)
	// 3 punti
	digitalWrite(led1, HIGH);   	// turn the LED on (HIGH is the voltage level)
	PT_SLEEP(pt, DELAY1);
	digitalWrite(led1, LOW);    	// turn the LED off by making the voltage LOW
	PT_SLEEP(pt, DELAY1);
  }
  PT_END(pt);
}

// definizione protothread del lampeggio
pt ptBlink2;
int blink2Thread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop del protothread
  while(true) {
	digitalWrite(led2, HIGH);   	// turn the LED on (HIGH is the voltage level)
	PT_SLEEP(pt, DELAY2);
	digitalWrite(led2, LOW);    	// turn the LED off by making the voltage LOW
	PT_SLEEP(pt, DELAY2);
  }
  PT_END(pt);
}

// definizione protothread del pulsante
pt ptBlink3;
int blink3Thread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop del protothread
  while(true) {//for(;;)
	// 3 punti
	digitalWrite(led3, HIGH);   	// turn the LED on (HIGH is the voltage level)
	PT_SLEEP(pt, DELAY3);
	digitalWrite(led3, LOW);    	// turn the LED off by making the voltage LOW
	PT_SLEEP(pt, DELAY3);
  }
  PT_END(pt);
}

// loop principale
void loop()
{
	PT_SCHEDULE(blink1Thread(&ptBlink1)); 		// esecuzione schedulatore protothreads
	PT_SCHEDULE(blink2Thread(&ptBlink2)); 	// esecuzione schedulatore protothreads
	PT_SCHEDULE(blink3Thread(&ptBlink3)); 	// esecuzione schedulatore protothreads
}
```

### **Sitografia:**
- http://dunkels.com/adam/pt/examples.html
- https://gitlab.com/airbornemint/arduino-protothreads
- http://dunkels.com/adam/pt/expansion.html
- https://roboticsbackend.com/arduino-protothreads-tutorial/
- https://github.com/naasking/async.h

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  
