
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  >[Versione in Python](async_await_py.md)

# **SCHEDULAZIONE CON ASYNC/AWAIT**

### **Base teorica**

### **Eventi vs thread**
Il modello di gestione della CPU nei SO normalmente è di tipo **multithreading  preemptive**, cioè con interruzione anticipata del task in esecuzione con riassegnazione della risorsa CPU ad un altro task, per almeno due motivi:
- Evitare lo spreco della risorsa CPU in attesa di un input bloccante che tarda la sua risposta di un tempo non prevedibile
- per realizzare un multitasking equo (fair) tramite l’esecuzione concorrente di più task in tempi uguali

Il modello di gestione della CPU in ambienti server come node JS e client come l’ambiente javascript di un browser web, invece, è normalmente a **singolo thread** dove il **multitasking** è generato non utilizzando il multithreading ma un modello di esecuzione **ad eventi (event driven runtime)** composto da:
- Un singolo thread
- Un singolo task in esecuzione alla volta (esecuzione seriale dei task)
- Più input in elaborazione contemporaneamente (esecuzione parallela degli input)

### **Modello ad eventi**

La libreria async.io ha un modello di runtime basato su un ciclo di eventi (event loop), che è responsabile:
- dell'esecuzione del codice
- della raccolta e dell'elaborazione degli eventi
- dell'esecuzione delle dei sotto task (sub-tasks) in coda.
   
Questo modello è abbastanza diverso dai modelli in altri linguaggi come C e Java basati su processi e thread.
Una proprietà molto interessante è che un **linguaggio ad eventi**, a differenza di molti altri linguaggi, **non blocca** mai gli altri task quando si è in attesa di un input sul task corrente.

La **gestione dell'I/O** viene in genere eseguita tramite **eventi** e **callback**:
- ad un evento sono associate una o più callback.
- Un evento è un’azione eseguita sulla GUI dall’utente. 
- Una callback è una funzione che viene richiamata quando viene servito l’evento ad essa associata.
- Gli eventi che occorrono (accadono) contemporaneamente e che sono pronti per essere processati dalla CPU vengono ospitati in una coda di messaggi. In questa attesa il sistema può ancora elaborare altri eventi immagazzinandoli in coda rimanendo così responsivo. 

Il primo messaggio in coda viene di volta in volta estratto e processato inserendo la sua callback, e tutte le funzioni ad essa annidate, in altrettanti frame sullo stack. La callback correntemente sullo stack, viene eseguita fino a che non ritornano tutte le sottofunzioni ad essa annidate.

### **Modello di esecuzione**

Ad un certo punto durante il ciclo di eventi, il processo runtime inizia a gestire i messaggi sulla coda, a partire da quello più vecchio. Per fare ciò:
- Il messaggio viene rimosso dalla coda
- la sua funzione corrispondente (callback listener) viene chiamata con il messaggio come parametro di input. 
- come sempre, la chiamata di una funzione crea un nuovo stack frame per l'uso di quella funzione.

L'elaborazione delle funzioni continua finché lo stack non è nuovamente vuoto. Quindi, il ciclo di eventi elaborerà il messaggio successivo nella coda (se ce n'è uno).

Loop di gestione degli eventi:
```python
while (queue.waitForMessage()) {
  queue.processNextMessage();
}
```
La funzione queue.waitForMessage() attende in modo sincrono l'arrivo di un messaggio (se uno non è già disponibile e in attesa di essere gestito).

### **Run to completition**

Ogni messaggio viene elaborato completamente prima che venga elaborato qualsiasi altro messaggio.

Questo comportamento induce alcune proprietà desiderabili sui programmi, incluso il fatto che, ogni volta che una funzione viene eseguita, non può essere terminata in anticipo e verrà eseguita interamente prima dell'esecuzione di qualsiasi altro codice (chee potrebbe alterare i dati manipolati dalla funzione). Ciò differisce da C, ad esempio, dove se una funzione viene eseguita in un thread, può essere interrotta in qualsiasi momento dal sistema di runtime (un timer HW) per eseguire altro codice in un altro thread.

Uno svantaggio di questo modello è che se un messaggio richiede troppo tempo per essere completato, l'applicazione Web non è in grado di elaborare le interazioni dell'utente come il clic di un pulsante.  Una buona pratica da seguire è rendere breve l'elaborazione dei messaggi e, se possibile, scomporre un messaggio in più messaggi.


I **async/await** fornise un meccanismo di **blocco dei task** (compiti) in cima a un **sistema ad eventi**, senza l'overhead di uno stack per ogni thread. Lo **scopo** del modello è quello di implementare un **flusso sequenziale di controllo** senza utilizzare complesse macchine a stati finiti ed evitando l'overhead di un multi-threading completo, cioè quello dotato anche del **modo preemptive**.  L'asynchronous I/O scheduler fornisce la sola **modalità cooperativa** e il **rilascio anticipato** delle risorse è realizzato **senza l'utilizzo di interrupt** che generino il **cambio di contesto** dei thread. 

In altre parole, la **gestione dei task** è **ad eventi** all'interno di un **singolo thread reale** ma, tramite delle **macro**, un algoritmo si può programmare in **maniera sequenziale** all'interno di funzioni che girano su dei **thread logici**, cioè emulati. La **concorrenza** dei task è gestita da uno **schedulatore** che, invocato durante il periodo di **sleep** di un task, o tramite il comando **yeld**, assegna la risorsa CPU a quei task che, in quell'intervallo, ne fanno richiesta.

### **Confronto con le altre tecniche**

Anche i **processi** e i **thread** sono flussi di esecuzione indipendenti che procedono in parallelo su una o più CPU, esiste però una **differenza pratica** notevole tra di essi:
- nei **processi** sia input/output, che **area dati globale** che **area dati locale** (stack) sono indipendenti e separate in zone diverse della memoria RAM.
- nei **thread**  input/output e **area dati globale** sono **in comune** nella stessa posizione in RAM mentre soltanto le **aree dati locali** (stack) sono indipendenti e separate in zone diverse della memoria RAM.
- nei **protothread** sia input/output che l'**area dati globale** ma anche le **aree dati locali** (stack) sono **in comune** nella stessa posizione in RAM. Pertanto le **variabili locali** non sono isolate in thread diversi (presenza di possibili ambiguità).

### **Utilizzo in pratica**

**Ogni protothread** realizza un **flusso** di esecuzione **parallelo** a quello degli altri thread, inoltre ognuno possiede un proprio **loop() principale** di esecuzione in cui realizzare le operazioni che tipicamente riguardano le **tre fasi** di lettura degli ingressi, calcolo dello stato del sistema e della sua risposta e la fase finale di scrittura della risposta sulle uscite. Il loop principale è definito sotto forma di **ciclo infinito** come ad esempio:

```python
while(true) {
    // codice del protothread
    .........................
}
```
Le **fasi di lavoro** del loop possono essere **schedulate** (pianificate nel tempo) dal delay() non bloccante ```PT_SLEEP(pt)``` e dal rilascio spontaneo ```PT_YIELD(pt)```che permettono la progettazione **lineare** di un algoritmo nel tempo.

**Ogni protothread** è definito da un **descrittore** che è una variabile di tipo struct, cioè il tipo record del C, che rappresenta il protothread. Il **nome** del descrittore è arbitrario a discrezione del programmatore. Il descrittore deve essere passato come **argomento** ad ogni **comando** (macro) della libreria protothread. Il descrittore può essere definito **esattamente prima** della definizione della funzione del protothread tramite la dichiarazione **```pt ptNome_descr```**;

Il **flusso di esecuzione** di un protothread è **definito** all'interno di una **funzione** e può essere avviato passando allo schedulatore il riferimento a questa funzione sotto la forma di parametro. In sostanza la funzione **serve** al programmatore per definire il protothread e allo schedulatore per poterlo richiamare. All'interno della funzione il protothread deve sempre cominciare con il comando **```PT_BEGIN(pt)```** e deve sempre terminare con il comando  **```PT_END(pt)```**. 

In definitiva la **dichiarazione e definizione** di **descrittore e funzione** del protothread possono assumere la forma:

```python
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
```python
void setup() {
  async_init(&ptMioScopo1);
  async_init(&ptMioScopo2);
}
```

Ogni protothrad viene **schedulato** cioè, valutato periodicamente per stabilire se deve essere eseguito o meno, all'interno del **```loop()```** tramite il comando **```PT_SCHEDULE(mioScopoThread(&ptMioScopo))```** che ha per argomento la funzione che definisce il protothread.

```python
void loop() {
	mioScopo1Thread(&ptMioScopo1);
	mioScopo2Thread(&ptMioScopo2);
}
```

Per quanto riguarda la **definizione** di un protothread va ricordato che ll'interno del loop del protothread ogni **ramo** di esecuzione va reso **non bloccante** inserendo, la funzione **```PT_SLEEP(pt)```** (mai la normale delay()) se il flusso di esecuzione deve essere **bloccato temporaneamente** per un certo tempo fissato, oppure la funzione **```async_yield```** se questo **non deve essere bloccato**. Ciò serve a richiamare lo schedulatore **almeno una volta**, qualunque **direzione** di  esecuzione prenda il codice, in modo da cedere **"spontaneamente"** il controllo ad un altro prothread al termine del loop() del prothread corrente. La **cessione del controllo** dello schedulatore ad ogni ramo di esecuzione **è necessario** altrimenti gli altri protothread non verrebbero mai eseguiti (il sistema **non è preemptive**).
Sia ```PT_YIELD(pt)``` che ```PT_SLEEP(pt)``` cedono il controllo della CPU allo schedulatore che lo assegna agli altri protothread che eventualmente in quel momento hanno scaduto il tempo di attesa di un loro delay.

**Blink sequenziali interagenti**

Di seguito è riportato un esempio di **blink sequenziale** in esecuzione su **due thread** separati su scheda **Arduino Uno**, con **IDE Arduino** e  con la libreria **protothread.h**  (https://gitlab.com/airbornemint/arduino-protothreads). I thread sono senza stack e **non preemptive** (solo collaborativi). La **programmazione sequenziale** del blink del led è **emulata** tramite una funzione delay() **non bloccante** ``` PT_SLEEP(pt, 200) ``` fornita dalla libreria ``` protothreads.h ```.


```python
import uasyncio as asyncio
async def bar(x):
    count = 0
    while True:
        count += 1
        print('Instance: {} count: {}'.format(x, count))
        await asyncio.sleep(1)  # Pause 1s

async def main():
    for x in range(3):
        asyncio.create_task(bar(x))
    await asyncio.sleep(10)

asyncio.run(main())
```
Link simulazione online: https://wokwi.com/projects/369675288427672577

```python
import uasyncio
from machine import Pin

async def blink(led, period_ms):
    while True:
        led.on()
        await uasyncio.sleep_ms(period_ms)
        led.off()
        await uasyncio.sleep_ms(period_ms)

async def main(led1, led2):
    uasyncio.create_task(blink(led1, 1000))
    uasyncio.create_task(blink(led2, 2000))
    await uasyncio.sleep_ms(10000)
    print('Ending all tasks')
    led1.off()
    led2.off()
    
led1 = Pin(12,Pin.OUT)
led2 = Pin(18,Pin.OUT)

uasyncio.run(main(led1, led2))
```

Link simulazione online: https://wokwi.com/projects/369678530188573697

```python
import uasyncio
from machine import Pin

async def blink(led, period_ms):
    while True:
        led.on()
        await uasyncio.sleep_ms(period_ms)
        led.off()
        await uasyncio.sleep_ms(period_ms)

async def main(led1, led2):
    uasyncio.create_task(blink(led1, 1000))
    uasyncio.create_task(blink(led2, 2000))
    while True:
        await uasyncio.sleep_ms(500)

led1 = Pin(12,Pin.OUT)
led2 = Pin(18,Pin.OUT)

uasyncio.run(main(led1, led2))
```

Link simulazione online: https://wokwi.com/projects/369680006454647809


**Pulsante responsivo + blink**

Di seguito è riportato un esempio di un blink sequenziale in esecuzione su un task e di gestione del pulsante sul loop principale. I ritardi sleep agiscono sul task secondario ma non bloccano la lettura dello stato del pulsante che rimane responsivo nell'accendere il secondo led durante entrambe le fasi del blink del primo led. 

```python
import uasyncio
from machine import Pin

async def blink(led, period_ms):
    while True:
        led.on()
        await uasyncio.sleep_ms(period_ms)
        led.off()
        await uasyncio.sleep_ms(period_ms)

async def main(btn, led1, led2):
    uasyncio.create_task(blink(led2, 1000))
    while True:
        if btn.value():
            led1.on()
        else:
            led1.off()
        
        await uasyncio.sleep_ms(50)

btn = Pin(12,Pin.IN)
led1 = Pin(13,Pin.OUT)
led2 = Pin(18,Pin.OUT)

uasyncio.run(main(btn, led1, led2))
```
Link simulazione online: https://wokwi.com/projects/369680948206974977

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

### **Sitografia:**
- https://docs.micropython.org/en/v1.15/library/uasyncio.html
- https://github.com/peterhinch/micropython-async/blob/master/v3/docs/TUTORIAL.md

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  >[Versione in Python](async_await_py.md)
