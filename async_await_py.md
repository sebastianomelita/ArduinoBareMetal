
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  >[Versione in C++](async_await.md)

# **SCHEDULAZIONE CON ASYNC/AWAIT**

### **Base teorica**

Le funzioni asincrone async sono un tipo di thread leggeri senza stack progettati per sistemi con vincoli di memoria severi come sottosistemi embedded o
nodi di reti di sensori.

- Le funzioni asincrone sono un’alternativa alle catene di promesse e rendono il lavoro con queste molto più intuitivo.
- Nonostante il nome, Il blocco di codice async diventa per tutte le funzioni che restituiscono una promise, sincrono e bloccante nel senso che ciascuna funzione con await davanti rimane bloccata e non può passare all’istruzione successiva fino a che la sua promise non viene risolta.
- Await va usato soltanto dentro un blocco di codice async e davanti a funzioni che restituiscono una promise.
- Possiamo usare un blocco try...catch per la gestione degli errori, esattamente come si farebbe se il codice fosse sincrono.
- le funzioni dichiarate asincrone con async restituiscono sempre una promise e quindi, per accedervi bisogna usare la funzione then. 
- Hanno la stessa utilità di una catena di promesse, cioè await forza il completamento in serie delle operazioni asincrone quando il risultato dell'operazione successiva dipende dal risultato dell'ultima, in caso contrario qualcosa come Promise.all() potrebbe essere più appropriato.



### **Confronto con le altre tecniche**

Anche i **processi** e i **thread** sono flussi di esecuzione indipendenti che procedono in parallelo su una o più CPU, esiste però una **differenza pratica** notevole tra di essi:
- nei **processi** sia input/output, che **area dati globale** che **area dati locale** (stack) sono indipendenti e separate in zone diverse della memoria RAM.
- nei **thread**  input/output e **area dati globale** sono **in comune** nella stessa posizione in RAM mentre soltanto le **aree dati locali** (stack) sono indipendenti e separate in zone diverse della memoria RAM.
- nei **async/await** sia input/output che l'**area dati globale** ma anche le **aree dati locali** (stack) sono **in comune** nella stessa posizione in RAM. Pertanto le **variabili locali** non sono isolate in thread diversi (presenza di possibili ambiguità).

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
async ptMioScopo;
int mioScopoThread(struct async* pt) {
  async_begin(pt);

  // Loop forever
  while(true) {
	// codice del protothread
	.........................
  }
  async_end;
}
```

Ogni protothread è **inizializzato** nel **setup()** tramite la funzione **```PT_INIT(&ptMioScopo)```**, il passaggio del descrittore è per **riferimento** perchè questo deve poter essere **modificato** al momento della inizializzazione.
```C++
void setup() {
  async_init(&ptMioScopo1);
  async_init(&ptMioScopo2);
}
```

Ogni protothrad viene **schedulato** cioè, valutato periodicamente per stabilire se deve essere eseguito o meno, all'interno del **```loop()```** tramite il comando **```PT_SCHEDULE(mioScopoThread(&ptMioScopo))```** che ha per argomento la funzione che definisce il protothread.

```C++
void loop() {
	mioScopo1Thread(&ptMioScopo1);
	mioScopo2Thread(&ptMioScopo2);
}
```

Per quanto riguarda la **definizione** di un protothread va ricordato che ll'interno del loop del protothread ogni **ramo** di esecuzione va reso **non bloccante** inserendo, la funzione **```PT_SLEEP(pt)```** (mai la normale delay()) se il flusso di esecuzione deve essere **bloccato temporaneamente** per un certo tempo fissato, oppure la funzione **```async_yield```** se questo **non deve essere bloccato**. Ciò serve a richiamare lo schedulatore **almeno una volta**, qualunque **direzione** di  esecuzione prenda il codice, in modo da cedere **"spontaneamente"** il controllo ad un altro prothread al termine del loop() del prothread corrente. La **cessione del controllo** dello schedulatore ad ogni ramo di esecuzione **è necessario** altrimenti gli altri protothread non verrebbero mai eseguiti (il sistema **non è preemptive**).
Sia ```PT_YIELD(pt)``` che ```PT_SLEEP(pt)``` cedono il controllo della CPU allo schedulatore che lo assegna agli altri protothread che eventualmente in quel momento hanno scaduto il tempo di attesa di un loro delay.

**Blink sequenziali indipendenti**

```python
/*
  Blink
  Turns an LED on for one second, then off for one second, repeatedly. Rewritten with Protothreads.
*/
import uasyncio
from machine import Pin

async def blink(led, period_ms):
    while True:
        led.value(not led.value())
        await uasyncio.sleep_ms(period_ms)

led1 = Pin(12,Pin.OUT)
led2 = Pin(13,Pin.OUT)

event_loop = uasyncio.get_event_loop()
event_loop.create_task(blink(led1, 700))
event_loop.create_task(blink(led2, 400))
event_loop.run_forever()

```
Link simulazione online: https://wokwi.com/projects/361715047715438593

Osservazioni:
- il codice non è specifico di alcuna macchina, è realizzato in C puro ed è altamente portabile.
- Un protothread viene eseguito all'interno di una singola funzione C e non può estendersi ad altre funzioni. Un protothread può chiamare normali funzioni C, ma non può bloccore all'interno di una funzione chiamata da altre funzioni. Il blocco all'interno di chiamate di funzioni annidate
è invece ottenuto generando un protothread separato per ciascuna funzione potenzialmente bloccante. Il vantaggio di questo approccio è che
il blocco è esplicito: il programmatore sa esattamente quali funzioni un protothread blocca e quali invece non blocca mai.
- poichè i protothread non salvano il contesto dello stack durante la chiamata di una funzione bloccante, ad es. una delay(), le **variabili locali** non vengono conservate quando protothread blocca un task. Ciò significa che le variabili locali dovrebbero essere **usate con la massima
attenzione**. In caso di dubbio, non utilizzare variabili locali all'interno di un protothread!
- Un protothread è guidato da ripetute chiamate alla funzione in cui il protothread è in esecuzione. Ogni volta che viene chiamata la funzione, il 
protothread verrà eseguito fino a quando non si blocca o esce. Pertanto la schedulazione dei protothreads viene eseguita dall'applicazione che utilizza i protothreads.
- Porre attenzione con le istruzioni switch all'interno di un protothread. Regola empirica: posiziona tutte le istruzioni switch in una loro funzione.
- non è possibile effettuare chiamate di sistema bloccanti e preservare la semantica asincrona. Questi devono essere cambiati in chiamate non bloccanti che testano una condizione.

**SOS + blink**

Simulazione su Esp32 in Wowki: : https://wokwi.com/projects/361667447263659009

```C++
/*
Realizzare il blink di un led insieme al lampeggio di un'altro led che codifica il messaggio morse dell'SOS.
*/
#include <limits.h>
typedef enum ASYNC_EVT { ASYNC_INIT = 0, ASYNC_CONT = ASYNC_INIT, ASYNC_DONE = 1 } async2;
#define async_state unsigned _async_k
struct async { async_state; };
#define async_begin(k) unsigned *_async_k = &(k)->_async_k; switch(*_async_k) { default:
#define async_end *_async_k=ASYNC_DONE; case ASYNC_DONE: return ASYNC_DONE; }
#define await(cond) await_while(!(cond))
#define await_while(cond) *_async_k = __LINE__; case __LINE__: if (cond) return ASYNC_CONT
#define async_yield *_async_k = __LINE__; return ASYNC_CONT; case __LINE__:
#define async_exit *_async_k = ASYNC_DONE; return ASYNC_DONE
#define async_init(state) (state)->_async_k=ASYNC_INIT
#define async_done(state) (state)->_async_k==ASYNC_DONE
#define async_call(f, state) (async_done(state) || (f)(state))
#define PT_SLEEP(delay) \
{ \
  do { \
    static unsigned long protothreads_sleep; \
    protothreads_sleep = millis(); \
    await( millis() - protothreads_sleep > delay); \
  } while(false); \
}
//-----------------------------------------------------------------------------------------------------------
// se si usa questa libreria al posto delle macro sopra, togliere il commento iniziale all'include 
// e commentare le macro sopra
//#include "protothreads.h"
#define PDELAY  500
#define LDELAY  1500
#define BLINKDELAY  300
byte led1 = 12;
byte led2 = 13;

// definizione protothread del pulsante
async ptSos;
int SOSThread(struct async *pt) {
  async_begin(pt);

  // Loop del protothread
  while(true) {
		// 3 punti
		digitalWrite(led1, HIGH); // 1
		PT_SLEEP(PDELAY);
		digitalWrite(led1, LOW);
		PT_SLEEP(PDELAY);
		digitalWrite(led1, HIGH); // 2
		PT_SLEEP(PDELAY);
		digitalWrite(led1, LOW);
		PT_SLEEP(PDELAY);
		digitalWrite(led1, HIGH); // 3
		PT_SLEEP(PDELAY);
		digitalWrite(led1, LOW);
		PT_SLEEP(PDELAY);
		// 1 linea
		digitalWrite(led1, HIGH); // 1 linea
		PT_SLEEP(LDELAY);
		digitalWrite(led1, LOW);
		PT_SLEEP(PDELAY);
		// 3 punti
		digitalWrite(led1, HIGH); // 1
		PT_SLEEP(PDELAY);
		digitalWrite(led1, LOW);
		PT_SLEEP(PDELAY);
		digitalWrite(led1, HIGH); // 2
		PT_SLEEP(PDELAY);
		digitalWrite(led1, LOW);
		PT_SLEEP(PDELAY);
		digitalWrite(led1, HIGH); // 3
		PT_SLEEP(PDELAY);
		digitalWrite(led1, LOW);
		PT_SLEEP(LDELAY);
  }
  async_end;
}

// definizione protothread del lampeggio
async ptBlink;
int blinkThread(struct async *pt) {
  async_begin(pt);

  // Loop del protothread
  while(true) {
		digitalWrite(led2, HIGH);   	// turn the LED on (HIGH is the voltage level)
		PT_SLEEP(BLINKDELAY);
		digitalWrite(led2, LOW);    	// turn the LED off by making the voltage LOW
		PT_SLEEP(BLINKDELAY);
  }
  async_end;
}

void setup()
{
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  async_init(&ptSos);
  async_init(&ptBlink);
}

// loop principale
void loop()
{
	SOSThread(&ptSos); 	// esecuzione schedulatore protothreads
	blinkThread(&ptBlink); 	// esecuzione schedulatore protothreads
}
```

### **Sitografia:**
- http://dunkels.com/adam/pt/examples.html
- https://gitlab.com/airbornemint/arduino-protothreads
- http://dunkels.com/adam/pt/expansion.html
- https://roboticsbackend.com/arduino-protothreads-tutorial/
- https://news.ycombinator.com/item?id=4772229
- https://github.com/naasking/async.h

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)   >[Versione in C++](async_await.md)
