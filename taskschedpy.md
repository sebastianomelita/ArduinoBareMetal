>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in C++](tasksched.md)

## **SCHEDULATORE COMPITI BASATO SUL CONTEGGIO DEI TICK**

Di seguito è riportato un esempio di schedulatore che pianifica nel tempo **l’esecuzione periodica** di una serie di **compiti** (task) da eseguire con **cadenza diversa**.

Il **tempo base** è la base dei tempi di tutte le schedulazioni, viene calcolato **periodicamente** all'accadere di un **evento** (superamento di una soglia di tempo) che viene detto **tick** (analogia con il metronomo per la musica).

Le varie schedulazioni **sono calcolate** a partire da un **multiplo intero** del tempo base, ne segue che il tempo base dovrebbe essere calcolato come il massimo comune divisore (**MCD**) di tutti i tempi che devono essere generati.

Il conteggio dei multipli del tempo base è tenuto da un **contatore circolare** (step) che deve essere **ruotato** dopo aver effettuato un numero di conteggi superiori al **massimo dei multipli** del tempo base necessari.

Se ci sono **pulsanti** da gestire insieme ad altri task il tempo base può essere impostato tra 50 e 200 mSec in maniera da poterlo utilizzare per effettuare un **polling degli ingressi** immune dal fenomeno dei rimbalzi (**antibounce SW**).
```Python
from gpio import *
from time import *

def main():
	pulsante = 0
	led1 = 1
	led2 = 2
	pinMode(led1, OUT)
	pinMode(led2, OUT)
	ledstate1 = LOW
	ledstate2 = LOW
	precm = 0
	precval = 0
	step = 0
	tbase = 1
	nstep = 100
	
	while True:
		# il codice eseguito al tempo massimo della CPU va qui	
		# .........
		if (uptime() - precm) >= tbase:  	   # schedulatore (e anche antirimbalzo)
			precm = uptime()  			   # preparo il tic successivo	
			step = (step + 1) % nstep      # conteggio circolare arriva al massimo a nstep-1
			# il codice eseguito al tempo base va quì	
			# ..........
			
			# task 1
			if not(step % 2):      # schedulo eventi al multiplo del tempo stabilito (2 sec)
				ledstate1 = HIGH - ledstate1
				digitalWrite(led1, ledstate1)  # stato alto: led blink
								
			# task 2
			if not(step % 3):      # schedulo eventi al multiplo del tempo stabilito (3 sec)
				ledstate2 = HIGH - ledstate2
				digitalWrite(led2, ledstate2)  # stato alto: led blink
								
			# il codice eseguito al tempo base va quì	
			# ..........
				
		# il codice eseguito al tempo massimo della CPU va qui	
		# ........
		
if __name__ == "__main__":
	main()
```
E’ buona norma evitare l’esecuzione frequente di operazioni lente quando queste non sono strettamente necessarie in modo da lasciare spazio ad altre operazioni, quali ad esempio gestione degli eventi di input, che richiedono una velocità maggiore per usufruirne in modo più interattivo.

**In particolare**, all'interno di un task, andrebbe accuratamente evitata l'introduzione di ritardi prolungati tramite **delay()**. Possono esserci per **breve tempo** (qualche mSec) ma, in ogni caso, non dovrebbero mai far parte della maniera (**algoritmo**) con cui la logica di comando **pianifica i suoi eventi**.

Utilizzando la tecnica della **schedulazione esplicita dei task** nel loop(), la **realizzazione di un algoritmo** non dovrebbe essere pensata in **maniera sequenziale (o lineare)** ma, piuttosto, come una **successione di eventi** a cui corrispondono dei **compiti** (task) che **comunicano** con le periferiche di input/output ed, eventualmente, dialogano tra loro in qualche modo (ad es. variabili globali, buffer, ecc.).

### **SCHEDULATORE DI COMPITI BASATO SU TIMER PERIODICI**

Uno schedulatore di compiti (task) si può realizzare anche utilizzando **più timers** basati sul polling della funzione millis(). 

```python
import time
from machine import Pin

def blink(led):
     led.value(not led.value())

led = [0, 0]
led[0] = Pin(12, Pin.OUT)
led[1] = Pin(18, Pin.OUT)
period = [500, 1000]
precs = [0, 0]
currTime = 0;

while True:
     currTime = time.ticks_ms()
     #task1
     if currTime - precs[0] >= period[0]:
          precs[0] += period[0]
          blink(led[0])
     #task2
     if currTime - precs[1] >= period[1]:
          precs[1] += period[1]
          blink(led[1])

     # il codice eseguito al tempo massimo della CPU va quì 
```

Si noti che:
- il timer SW con il polling viene eseguito ad ogni ciclo di ```loop()```
- i timer SW sono N (uno per ogni tempo futuro)
- il calcolo degli N tempi futuri è eseguito N volte (una per ogni tempo) **ad ogni ciclo** di ```loop()```
- il calcolo può essere reso estremamente **preciso** e realizzato in maniera indipendente tra un tempo e l'altro.

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/371606123442169857

**Recupero dei tick persi in un task lento**

In questo caso il **ritardo** di un **task** maggiore di un **tempo base** è compensato. Infatti se dopo un tempo ```t``` pari a ```x``` multipli di ```tbase``` lo scedulatore ricampiona il tempo con la funzione ```millis()```, allora la condizione ```if ((current_millis - precs[0]) >= period[0])``` sarà valutata alla massima velocità del clock e sarà vera per ```x``` volte, ciò causerà un rapido recupero di ```precs[0]``` fino a che la diferenza non sarà minore di ```tbase```. Da questo momento in poi i tick procederanno senza ritardo fino allo scatto della condizione dei vari task. 

**Riordinamento dei task**

Se il primo task veloce è affetto da ritardi casuali può accadere che questi possono ritardare anche i task che occorrono a seguire nello stesso ```loop()```. Questo accade se più task con **periodicità diversa** occorrono nello stesso tempo (tick).

Se più task con **periodicità diversa** occorrono nello stesso tempo (tick), conviene dare **priorità maggiore** a quelli **con periodicità più lunga** perchè un eventuale **ritardo** di un **task veloce** determinerebbe un **errore di tempo** che coinvolgerebbe solo il **primo task breve** a seguire (rimanendo confinato nel tick corrente) e non avrebbe effetto  sui **tick lenti** (di periodicità più grande che agiscono su più tick) dato che questi sarebbero sempre **serviti prima**. In altre parole, si cerca, in questo modo, di **limitare** l'effetto di eventuali ritardi di un task sul minor numero possibile di tick consecutivi.

**Ricampionamento ad ogni task**

Un'**alternativa** praticabile in questa categoria di schedulatori  potrebbe essere **ricampionare** il tempo **corrente** sia ad ogni ```loop()``` e, all'interno di questo, tra ogni task. Il ricampionamente avrebbe due obiettivi:
- recuperare il ritardo di un task che potrebbe ripercuotersi sui task dei **tick successivi**
- recuperare il ritardo di un task facendo in modo che quello successivo, in esecuzione sullo **stesso tick**, parta al tempo giusto

Una soluzione basata sul **ricampionamento** tra un task e l'altro potrebbe farsi così:

```python
import time
from machine import Pin
import random

def blink(led):
     led.value(not led.value())

led = [0, 0]
led[0] = Pin(12, Pin.OUT)
led[1] = Pin(18, Pin.OUT)
period = [500, 2000]
precs = [0, 0]
currTime = 0;
prevMillis = 0

while True:
     currTime = time.ticks_ms()
     #task1
     if currTime - precs[0] >= period[0]:
          precs[0] += period[0]
          randomDelay = random.randint(1,200)
          print("delay: ", randomDelay)
          time.sleep_ms(randomDelay)
          blink(led[0])
     #task2
     currTime = time.ticks_ms()
     if currTime - precs[1] >= period[1]:
          precs[1] += period[1]
          now = time.ticks_ms()
          diff = now-prevMillis
          print("ontwosec: ", diff);
          blink(led[1])
          prevMillis = now

     # il codice eseguito al tempo massimo della CPU va quì  
```
Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/371607502633180161

### **SCHEDULATORE DI COMPITI BASATO SU FILTRAGGIO DEI TIME TICK**

E' possibile realizzare uno schedulatore di **più task** che agiscono con periodicità **diverse** in tempi diversi a partire da un timer che esegue periodicamente un **task comune** si sincronizzazione che agisce su un **tempo** base comune **sottomultiplo** del tempo di tutti gli altri task. In altre parole, il **periodo** di ciascun task è un **multiplo intero** del periodo base di sincronizzazione comune:
- il **tick** del **tempo base** viene ottenuto mediante il polling della funzione ```millis()``` eseguito ad **ogni ```loop()```**
- ad ogni **tick** del **tempo base** viene aggiornato il momento del prossimo tick mediante l'espressione ```precm += tbase```
- Il **tempo base** comune a tutti i task, si può calcolare col **massimo comune divisore** (M.C.D. o G.C.D) di tutti i tempi dei singoli task.
- **ad ogni tick** del tempo base viene misurato se è maturato il tick del tempo di ciascun task valutando il valore del tempo base fino a quel momento mediante ```(precm - precs[i]) >= period[i]```
- se la **condizione di scatto** del tick di un task è verificata allora viene calcolato il **tempo** del tick **successivo** sommando al tempo del tick corrente la periodicità del task: ```precs[i] += period[i]```


```python
import time
from machine import Pin
import random

def blink(led):
     led.value(not led.value())

led = [0, 0]
led[0] = Pin(12, Pin.OUT)
led[1] = Pin(18, Pin.OUT)
period = [500, 1000]
precs = [0, 0]
currTime = 0;
prevMillis = 0
precm = 0
tbase = 500
#inizializzazione dei task
for i in range(2):
     precs[i] = precm -period[i]

while True:
     if time.ticks_ms() - precm >= tbase:
          precm += tbase
          #task1
          if currTime - precs[0] >= period[0]:
               precs[0] += period[0]
               blink(led[0])
          #task2
          currTime = time.ticks_ms()
          if currTime - precs[1] >= period[1]:
               precs[1] += period[1]
               blink(led[1])

     # il codice eseguito al tempo massimo della CPU va quì 
```
Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/371608560910636033

**Inizializzazione dei task**

Per ottenere la partenza sincronizzata di tutti i task al primo tick del sistema bisogna prima inizializzare il tempo trascorso (```precs[i]```) di ogni task al valore del suo intervallo di esecuzione (``period[i]``):

```python
#inizializzazione dei task
for i in range(2):
     precs[i] = precm -period[i]
```

**Recupero dei tick persi in un task lento**

In questo caso il **ritardo** di un **task** maggiore di un **tempo base** è compensato. Infatti se dopo un tempo ```t``` pari a ```x``` multipli di ```tbase``` lo scedulatore ricampiona il tempo con la funzione ```millis()```, allora la condizione ```if(millis()-precm >= tbase``` sarà valutata alla massima velocità del clock e sarà vera per ```x``` volte, ciò causerà un rapido recupero di ```precm``` fino a che la diferenza non sarà minore di ```tbase```. Da questo momento in poi i tick procederanno senza ritardo fino allo scatto della condizione dei vari task.

**Riordinamento dei task**

Se il primo task veloce è affetto da ritardi casuali può accadere che questi possono ritardare anche i task che occorrono a seguire nello stesso ```loop()```. Questo accade se più task con **periodicità diversa** occorrono nello stesso tempo (tick).

Se più task con **periodicità diversa** occorrono nello stesso tempo (tick), conviene dare **priorità maggiore** a quelli **con periodicità più lunga** perchè un eventuale **ritardo** di un **task veloce** determinerebbe un **errore di tempo** che coinvolgerebbe solo il **primo task breve** a seguire (rimanendo confinato nel tick corrente) e non avrebbe effetto  sui **tick lenti** (di periodicità più grande che agiscono su più tick) dato che questi sarebbero sempre **serviti prima**. In altre parole, si cerca, in questo modo, di **limitare** l'effetto di eventuali ritardi di un task sul minor numero possibile di tick consecutivi.

In questo caso **non è possibile ricampionare** i task in maniera indipendente l'uno dall'altro perchè quelli con **tempo uguale** devono avvenire nello **stesso tick**.

Una soluzione in base a quanto descritto sopra potrebbe essere:

```python
import time
from machine import Pin
import random

def blink(led):
     led.value(not led.value())

led = [0, 0]
led[0] = Pin(12, Pin.OUT)
led[1] = Pin(18, Pin.OUT)
period = [500, 2000]
precs = [0, 0]
currTime = 0;
prevMillis = 0
precm = 0
tbase = 500
#inizializzazione dei task
for i in range(2):
     precs[i] = precm -period[i];

while True:
     if time.ticks_ms() - precm >= tbase:
          precm += tbase
          #task2 (con periodicità maggiore prima)
          currTime = time.ticks_ms()
          if currTime - precs[1] >= period[1]:
               precs[1] += period[1]
               now = time.ticks_ms()
               diff = now-prevMillis
               print("ontwosec: ", diff);
               blink(led[1])
               prevMillis = now
          #task1
          if currTime - precs[0] >= period[0]:
               precs[0] += period[0]
               randomDelay = random.randint(1,200)
               print("delay: ", randomDelay)
               time.sleep_ms(randomDelay)
               blink(led[0])

     # il codice eseguito al tempo massimo della CPU va quì 
```

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/371659893418077185

### **SCHEDULATORE DI COMPITI GENERICO SENZA MILLIS**

E' possibile realizzare uno schedulatore di task che non necessita di alcuna funzione di misura del tempo (delay() o millis()). L'unico **prerequisito** è che avvenga l'**invocazione periodica** di una funzione ad un certo **tempo base** comune a tutti i task, calcolato col **massimo comune divisore** (M.C.D. o G.C.D) di tutti i tempi dei singoli task. Un **contatore** di tempi base determina, per ogni task, il momento buono in cui questo deve essere eseguito **resettando** il proprio contatore subito dopo.

La **base dei tempi** comune può essere realizzata mediante qualunque tecnica di **generazione di tempi periodici**, ma la particolarità dell'assenza di misure di **tempo corrente** (millis()) rendono la realizzazione particolarmente adatta per essere adoperata su una base dei tempi generata mediante **interrupt**. 

La **versione originale** più completa dello schedulatore insieme ad una dettagliata discussione teorica si trova in: https://www.ics.uci.edu/~givargis/pubs/C50.pdf e in https://www.cs.ucr.edu/~vahid/rios/.

```python
import time
from machine import Pin, Timer

def blink(led):
     led.value(not led.value())

def scheduleAll(timer):
     global processingRdyTasks
     global elapsedTime
     if processingRdyTasks:
          print("Timer ticked before task processing done")
     else:
          processingRdyTasks = True
          #task1
          if elapsedTime[0] >= period[0]:
               blink(led[0])
               elapsedTime[0] = 0
          elapsedTime[0] += tbase
          #task2
          if elapsedTime[1] >= period[1]:
               blink(led[1])
               elapsedTime[1] = 0
          elapsedTime[1] += tbase
          processingRdyTasks = False

led = [0, 0]
led[0] = Pin(12, Pin.OUT)
led[1] = Pin(18, Pin.OUT)
period = [500, 1000]
elapsedTime = [0, 0]
tbase = 500
myPerTimer = Timer(3)
myPerTimer.init(period=tbase, mode=Timer.PERIODIC, callback=scheduleAll)
processingRdyTasks = False
#inizializzazione dei task
for i in range(2):
     elapsedTime[i] = period[i]

while True:
     time.sleep_ms(1)
     # il codice eseguito al tempo massimo della CPU va quì 
```
Il **flag** ```processingRdyTasks``` servirebbe ad evitare l'interruzione della ISR sopra un'altra ISR dovuta ad un eventuale ritardo di completamento di un task precedente. Questa circostanza nei microcontrollori come Arduino o ESP32 in genere non accade perchè le **interruzioni annidate** sono **di base** (default) **disabilitate**.

Anche in questo caso **non è possibile ricampionare** i task in maniera indipendente l'uno dall'altro perchè quelli con **tempo uguale** devono avvenire nello **stesso tick**.

**Inizializzazione dei task**

Per ottenere la partenza sincronizzata di tutti i task al primo tick del sistema bisogna prima inizializzare il tempo trascorso (elapsed) di ogni task al valore del suo intervallo di esecuzione (periodo):

```python
#inizializzazione dei task
for i in range(2):
     precs[i] = precm - period[i]
```
**Recupero dei tick persi in un task lento**

In questo caso il **ritardo** di un **task** maggiore di un **tempo base** non potrebbe essere compensato dato che **da un lato** la funzione ```scheduleAll()``` non è interrompibile per cui ritardo di un task ritarda anche il task successivo, **dall'altro** ```elapsedTime[0] += tbase``` viene incrementata ad ogni tick sempre una sola volta, per cui se il ritardo di un task ha superato più tick, il task successivo **non** è in grado di recuperarli nel suo conteggio del tempo trascorso.

**Riordinamento dei task**

Se il primo task veloce è affetto da ritardi casuali può accadere che questi possono ritardare anche i task che occorrono a seguire nello stesso ```loop()```. Questo accade se più task con **periodicità diversa** occorrono nello stesso tempo (tick).

Se più task con **periodicità diversa** occorrono nello stesso tempo (tick), conviene dare **priorità maggiore** a quelli **con periodicità più lunga** perchè un eventuale **ritardo** di un **task veloce** determinerebbe un **errore di tempo** che coinvolgerebbe solo il **primo task breve** a seguire (rimanendo confinato nel tick corrente) e non avrebbe effetto  sui **tick lenti** (di periodicità più grande che agiscono su più tick) dato che questi sarebbero sempre **serviti prima**. In altre parole, si cerca, in questo modo, di **limitare** l'effetto di eventuali ritardi di un task sul minor numero possibile di tick consecutivi.

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/371622016301018113

Una soluzione parziale in base a quanto descritto sopra potrebbe essere:

```python
import time
from machine import Pin, Timer
import random

def blink(led):
     led.value(not led.value())

def scheduleAll(timer):
     global processingRdyTasks
     global elapsedTime
     global prevMillis
     if processingRdyTasks:
          print("Timer ticked before task processing done")
     else:
          processingRdyTasks = True
          #task2 (task con periodicità maggiore prima)
          if elapsedTime[1] >= period[1]:
               now = time.ticks_ms()
               diff = now-prevMillis
               print("ontwosec: ", diff);
               blink(led[1])
               prevMillis = now
               elapsedTime[1] = 0
          elapsedTime[1] += tbase
          #task1
          if elapsedTime[0] >= period[0]:
               randomDelay = random.randint(1,200)
               print("delay: ", randomDelay)
               time.sleep_ms(randomDelay)
               blink(led[0])
               elapsedTime[0] = 0
          elapsedTime[0] += tbase
          processingRdyTasks = False
          

led = [0, 0]
led[0] = Pin(12, Pin.OUT)
led[1] = Pin(18, Pin.OUT)
period = [500, 2000]
elapsedTime = [0, 0]
tbase = 500
myPerTimer = Timer(3)
myPerTimer.init(period=tbase, mode=Timer.PERIODIC, callback=scheduleAll)
processingRdyTasks = False
prevMillis = 0
#inizializzazione dei task
for i in range(2):
     elapsedTime[i] = period[i]

while True:
     time.sleep_ms(1)
     # il codice eseguito al tempo massimo della CPU va quì 
```

Per compensare i **ritardi** di un task, potrebbe essere sfruttata la proprietà di **prerilascio** forzato di un task che possiedono gli interrupt. Se i tick sono **interrupt based** allora essi accadono sempre e comunque nel tempo esatto a loro deputato anche in presenza di un task che ritarda la sua esecuzione. Questo perchè un tick che occorresse ripetutamente su uno stesso task molto lento causa comunque la sua **interruzione** e l'esecuzione di una **ISR** che, pur **non** potendo servire un **nuovo task**, può far partire un algoritmo che **compensi il conteggio** dei tick dei task successivi a quello che si prolunga troppo in maniera da farli accadere al **tempo giusto**. 

Anche se task di tick successivi non si interferiscono, rimane sempre l'influenza che un task **lento a terminarsi** può avere sui task del suo stesso tick che potrebbe essere ridotta grazie all'espediente del **riordinamento** per tempi decrescenti dei task.

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/371634262353906689

```python
import time
from machine import Pin, Timer

class Toggle(object):
    def __init__(self, btn, state = False):
        self.btn = btn
        self.state = state
        self.precval = 0
    def toggle(self):
        changed = False
        val = self.btn.value()
        if self.precval == 0 and val == 1: 
            changed = True
            self.state = not self.state
            print(self.state)
        self.precval = val 
        return changed
    def getState(self):
        return self.state
    def setState(self,state):
        self.state = state

def blink(led):
    led.value(not led.value())

def press(led):
    global pulsante
    pulsante.toggle()
    led.value(pulsante.getState())
            
def  timerISR(timer):
    global timerFlag
    global count
    if timerFlag:
        # recupero dei tick saltati a causa di 
        # un eventuale ritardo di un task
        for i in range(taskNum):
            elapsedTime[i] += tbase
    else:
        timerFlag = True
       
def scheduleAll():
    global elapsedTime
    global tickFct
    global elapsedTime
    global taskNum
    for i in range(taskNum):
        if elapsedTime[i] >= period[i]:
            tickFct[i](pin[i])
            elapsedTime[i] = 0			
        elapsedTime[i] += tbase

btn1 = Pin(12,Pin.IN)
led1 = Pin(13,Pin.OUT)
led2 = Pin(2,Pin.OUT)
pulsante = Toggle(btn1)
pin = [led2, led1]
timerFlag = False
tickFct = [blink, press]
period = [500, 50]
elapsedTime = [0, 0]
taskNum = len(period)
tbase = 50
count = [0, 0]
myPerTimer = Timer(3)
myPerTimer.init(period=tbase, mode=Timer.PERIODIC, callback=timerISR)
#inizializzazione dei task
for i in range(taskNum):
     elapsedTime[i] = period[i]

while True:
    if timerFlag:
        scheduleAll()
        #time.sleep_ms(200)
        timerFlag = False
    time.sleep_ms(1)
    # il codice eseguito al tempo massimo della CPU va quì 
```

Il **flag** ```timerFlag``` serve a:
- **segnalare** nel ```loop()``` il momento buono per eseguire lo schedulatore, cioè lo scadere di un tempo base
- **evitare l'interruzione** dello schedulatore ```scheduleAll()``` sopra un'altra chiamata non terminata della stessa funzione dovuta ad un eventuale ritardo di completamento di un task precedente. Questa circostanza, che in una ISR normalmente non accade, è invece sempre possibile nel loop dove le funzioni sono eseguite in maniera **non atomica** e quindi sono tutte potenzialmente **interrompibili**. Il controllo si effettua **testando** il flag di schedulazione all'interno della **ISR**. 

La **versione originale** più completa dello schedulatore insieme ad una dettagliata discussione teorica si trova in: https://www.ics.uci.edu/~givargis/pubs/C50.pdf e in https://www.cs.ucr.edu/~vahid/rios/.

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/371675699190786049

## **Esempi**

### **Blink a fasi**

```python
#Realizzzare un programma che fa blinkare un led per 5 sec poi lo fa stare
#sempre spento per altri 5 sec, poi lo fa blinkare di nuovo per altri 5 sec e così via.
import time
from machine import Pin

led1 = Pin(13, Pin.OUT)
pulsante = 0
stato = True
precm = 0
precval = 0
step = 0
tbase = 500
nstep = 1000
	
while True:
    # il codice eseguito al tempo massimo della CPU va qui	
    # .........
    if time.ticks_ms() - precm >= tbase:  	   # schedulatore (e anche antirimbalzo)
        precm = time.ticks_ms()  			   # preparo il tic successivo	
        step = (step + 1) % nstep      # conteggio circolare arriva al massimo a nstep-1
        # il codice eseguito al tempo base va quì	
        # ..........
        
        # task 1
        if not(step % 1):      # schedulo eventi al multiplo del tempo stabilito (2 sec)
            if stato:
                led1.value(not led1.value())  # stato alto: led blink
            else:
                led1.off()
                            
        # task 2
        if not(step % 10):      # schedulo eventi al multiplo del tempo stabilito (3 sec)
            stato = not stato                           
        # il codice eseguito al tempo base va quì	
        # ..........
            
    # il codice eseguito al tempo massimo della CPU va qui	
    # ........
```

Di seguito il link della simulazione online con Wokwi: https://wokwi.com/projects/371688251759396865

### **Pulsante toggle con antirimbalzo insieme a blink**

In questo esempio si utilizza un unico **timer HW** come **base dei tempi** per uno **schedulatore SW** che gestisce la tempistica di **due task**: 
- uno per la relizzazione di un **tasto toggle** con proprietà di antirimbalzo
- un'altra per la realizzazione del **blink periodico** di un led

Le operazioni benchè semplici vengono considerate come prototipi di task più complessi e magari soggetti a **ritardi** considerevoli. In questa circostanza la loro esecuzione all'interno di una ISR è **sconsigliata** per cui essi vengono eseguiti nel ```loop()``` principale su **segnalazione** di un **flag** asserito dentro la ISR del timer.

```python
import time
from machine import Pin, Timer

class Toggle(object):
    def __init__(self, btn, state = False):
        self.btn = btn
        self.state = state
        self.precval = 0
    def toggle(self):
        changed = False
        val = self.btn.value()
        if self.precval == 0 and val == 1: 
            changed = True
            self.state = not self.state
            print(self.state)
        self.precval = val 
        return changed
    def getState(self):
        return self.state
    def setState(self,state):
        self.state = state

def blink(led):
    led.value(not led.value())

def press(p):
    p.toggle()
    time.sleep_ms(200)# emulazione ritardo del task

def toggleLogic(led):
    global pulsante
    if pulsante.getState():
        blink(led)
        print("Stato ",pulsante.getState())
        time.sleep_ms(100)# emulazione ritardo del task
    else:
        led.off()
         
def  timerISR(timer):
    global timerFlag
    global count
    if timerFlag:
        for i in range(taskNum):
            elapsedTime[i] += tbase
            count[i] +=1
    else:
        timerFlag = True
        for i in range(taskNum):
            if count[i] > 0:
                print("Recuperati ", count[i], " ticks del task ", i)
        for i in range(taskNum):
             count[i] = 0

def scheduleAll():
    global elapsedTime
    global tickFct
    global elapsedTime
    global taskNum
    for i in range(taskNum):
        if elapsedTime[i] >= period[i]:
            tickFct[i](pin[i])
            elapsedTime[i] = 0			
        elapsedTime[i] += tbase

btn1 = Pin(12,Pin.IN)
led1 = Pin(13,Pin.OUT)
led2 = Pin(2,Pin.OUT)
pulsante = Toggle(btn1)
pin = [led1, led2, pulsante]
timerFlag = False
tickFct = [toggleLogic, blink, press]
period = [1000, 500, 50]
elapsedTime = [0, 0, 0]
taskNum = len(period)
tbase = 50
count = [0, 0, 0]
myPerTimer = Timer(3)
myPerTimer.init(period=tbase, mode=Timer.PERIODIC, callback=timerISR)
#inizializzazione dei task
for i in range(taskNum):
     elapsedTime[i] = period[i]

while True:
    if timerFlag:
        scheduleAll()
        #time.sleep_ms(200)
        timerFlag = False
    time.sleep_ms(1)
    # il codice eseguito al tempo massimo della CPU va quì 
```

Di seguito il link della simulazione online con Tinkercad su Arduino: https://wokwi.com/projects/371662961899688961


### **Sitografia:**

- https://github.com/Koepel/Fun_with_millis#fun_with_millis
- http://arduino.cc/forum/index.php/topic,124048.msg932592.html#msg932592
- https://github.com/marcelloromani/Arduino-SimpleTimer/tree/master/SimpleTimer
- https://github.com/sebastianomelita/time-tick-scheduler
- https://www.ics.uci.edu/~givargis/pubs/C50.pdf
- https://www.cs.ucr.edu/~vahid/rios/
- https://wokwi.com/projects/352691213474403329
- https://wokwi.com/projects/352976196236642305
 
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in C++](tasksched.md)

