>[Torna all'indice](timerbase.md) >[versione in C++](polledtimer_seq.md)

## **TIMER BASATO SUL POLLING DEL TEMPO CORRENTE GET()**

E’ possibile realizzare dei timers, con cui programmare **nel futuro** lo stesso evento o una sequenza di eventi diversi, **senza attese**, cioè senza ```delay()```, eseguendo un polling del **tempo di conteggio** di un **timer**. 

Un **polling** è l'interrogazione periodica di una **variabile** o di un **ingresso** per leggerne il **valore**. Se il valore è maggiore di un **tempo massimo** allora si considera avvenuto un **timeout** e si esegue il **blocco** di codice associato ad esso.

Il polling serve per verificare, ad ogni loop(), che il tempo trascorso (**elapsed**) abbia raggiunto il **valore finale** (timeout) stabilito come obiettivo (target). Ciò si può ottenere eseguendo il **test** periodico di una **condizione** su una funzione di **misura** del tempo, nello specifico una ```get()```, che, ad ogni ```loop()```, restituisce il **tempo di conteggio**. Se la **condizione di uscita** dal conteggio è **vera** allora il **blocco** di istruzioni associato al **timeout del timer** viene **eseguito**.

Un **timeout** si controlla, valutando la **condizione di scadenza** del timer sulla funzione ```get()``` mediante una istruzione di selezione **if**. Ad esempio, eseguendo periodicamente (**polling**) nel ```loop()``` il controllo ```if(t.get() > 10000) {....}```, si può stabilire se, dal momento dell'attivazione del timer, sono pasasti 10 secondi e, in caso affermativo, eseguire le istruzioni nel blocco then che nel codice segue la condizione di scadenza.

La funzione ```get()```  **non è bloccante** (non causa alcun ritardo) e  **non interferisce** con nessun delay del loop corrente, sia esso il loop principale o quello secondario di un thread. Essa rappresenta, quindi, un **task** che si può tranquillamente adoperare in **sequenza** ad altri task di uno stesso loop, anche se questi contengono dei ritardi ```delay()```.

Il **polling** della funzione ```get()``` può essere effettuato in qualunque punto del loop, anche più volte se necessario, in genere si esegue per:
- se il timer **è attivo**, controllare una **cadenza**, cioè vedere se è trascorso il tempo **giusto** (periodo) per compiere una certa **azione** regolare (modificare una variabile o chiamare una funzione). La **periodicità** del timer può essere ottenuta invocando in sequenza due funzioni ```get() e reset()```. La **prima** serve a rilevare lo scadere del periodo e la **seconda** per far ripartire il timer da questo momento. 
- se il timer **è attivo**, controllare una **scadenza**, cioè vedere se è trascorso il tempo **massimo** (timeout) per compiere una certa **azione** prima che questa venga inibita. Il blocco del timer dopo una operazione una tantum (non periodica) può essere ottenuto invocando due funzioni ```get() e stop()``` consecutive. La **prima** serve a rilevare lo scadere del timeout e la **seconda** per far disattivare (disarmare) il timer da questo momento. 
- se il timer **non è attivo**, memorizzare una **misura di tempo**, cioè quell'intervallo temporale che è stato **fissato** su un certo tempo con uno ```stop()```. Dopo si potrebbe utilizzare quel valore di tempo misurato, ad esempio, per impostare un **ritardo** variabile con una istruzione ```delay(t.get())```, per trasformarlo in una grandezza fisica **proporzionale** a quel tempo mediante la funzione ```map()```, oppure per confrontarlo con altri tempi che si stanno **accumulando** all'interno del loop.  

Si tratta di un **pattern** (tipo di API) per la realizzazione di timers **molto comune** nella programmazione di **bracci robotici** per uso industriale (vedi bracci Universal Robots) che si adatta bene ad essere impiegato sia in un contesto in cui la logica dell'algoritmo è realizzata in maniera sequenziale sia in quello in cui gli input sono gestiti con un modello ad eventi.

**In** **generale**, possiamo individuare alcune **componenti del timer**:

- **reset** del timer. Accade al verificarsi di una certa **condizione**. Determina sia l'azzeramento del **tempo di conteggio** che l'azzeramento della misura del ritardo dall'ultimo riavvio. L'azzeramento **non** è mai automatico ed è necessario impostarlo prima del riutilizzo del timer dopo uno o più comandi ```stop()``` (tipicamente al momento della valutazione della condizione di timeout) altrimenti è sufficiente richiamare la funzione ```start()```.  
- **start**. Avvia (arma) o riavvia (riarma) il timer quando si avvera la **condizione di attivazione** posta in punto qualsiasi del loop. Il suo scopo è anche quello di memorizzare il **tempo iniziale** di riferimento a partire dal quale la ```get()``` misurerà il **tempo trascorso** (o elapsed). Chiamate successive alla funzione ```start()``` non modificano questo **tempo iniziale** perchè questo viene marcato come già misurato da un **flag singleton** che si assicura che la chiamata alla sottostante funzione ```millis()``` non avvenga più di **una volta**. 
- **stop** del timer (disarmo). Accade al verificarsi di una certa **condizione** di sospensione del timer posta in punto qualsiasi del loop. In questa occasione il timer **campiona** il ritardo accumulato dall'ultimo riavvio e lo somma al **tempo di conteggio** cumulato in corrispondenza della sospensione precedente. In questo modo il tempo trascorso è memorizzato e può essere valutato in un momento successivo nel codice tramite la funzione ```get()```.
- **get** del tempo **tempo di conteggio** (elapsed) dal momento del **primo avvio** del timer. E' una funzione **non bloccante**, cioè non genera attese dato che restituisce immediatamente il suo risultato. Viene **interrogata** periodicamente (polling) ad ogni ciclo di **loop** (meno se decimato ogni tot loop) e serve a realizzare una espressione di confronto con un tempo di **timeout** (```var.get() > timeout```). Il **valore restituito** dipende dallo stato del timer:
     - se **```timerState == false```** allora restituisce l'ultimo **tempo di conteggio** misurato e memorizzato al momento della chiamata a stop() .
     - se **```timerState == true```** allora calcola il **tempo di conteggio** corrente campionando il ritardo attuale dall'ultimo riavvio e sommandolo ai ritardo cumulato prima dell'ultima sospensione.
- **istruzioni triggerate** (scatenate) dal timer. Vengono eseguite in base al **tempo di conteggio misurato** dal timer. Vengono eseguite in maniera sequenziale in un punto stabilito del loop (istruzioni sincrone) in corrispondenza della **verità** di una certa **condizione** sul tempo di conteggio che coinvolge la funzione ```get()``` come operando. Contestualmente a queste si potrebbero eseguire :
  -    **reset()** per azzerare il timer.
  -    **stop()** per bloccare (disarmare) il timer.

Per consultare dettagli sulla sua implementazione vedi [timer sequenziali](polledtimer_seq.md).



```python
class DiffTimer(object):
    def __init__(self,elapsed):
        self.elapsed = elapsed
        self.timerState = False
	self.last = 0
    def __init__(self):
        self.elapsed = 0
        self.timerState = False
	self.last = 0
    def reset(self): # transizione di un pulsante
        self.elapsed = 0
        self.last = time.time()
    def stop(self):
        self.timerState = False
        self.elapsed = self.elapsed + time.time() - self.last
    def start(self):
        self.timerState = True
        self.last = time.time()
    def get(self):
        if self.timerState:
            return time.time() - self.last + self.elapsed
        return self.elapsed
    def set(self, e):
        reset()
        self.elapsed = e
```

### **Esempi**

```python
#Scrivere un programma che realizzi l'accensione di un led tramite un pulsante temporizzato che spegne il led 
#dopo un numero di ms impostati da setup. La logica del tasto deve essere senza stato e deve essere sensibile 
#al fronte di salita del segnale. Immaginare un collegamento pull down del tasto.
import time
from machine import Pin

# attesa evento con tempo minimo di attesa
def waitUntilInLow(btn,t):
    while btn.value():
	 time.sleep_ms(t)

class DiffTimer(object):
    def __init__(self,elapsed):
        self.elapsed = elapsed
        self.timerState = False
        self.last = 0
    def __init__(self):
        self.elapsed = 0
        self.timerState = False
        self.last = 0
    def reset(self): # transizione di un pulsante
        self.elapsed = 0
        self.last = time.ticks_ms()
    def stop(self):
        self.timerState = False
        self.elapsed = self.elapsed + time.ticks_ms() - self.last
    def start(self):
        self.timerState = True
        self.last = time.ticks_ms()
    def get(self):
        if self.timerState:
            return time.ticks_ms() - self.last + self.elapsed
        return self.elapsed
    def set(self, e):
        reset()
        self.elapsed = e
	
tasto = Pin(12,Pin.IN)
led = Pin(13,Pin.OUT)
acceso = DiffTimer()

while True:
    if tasto.value() == 1:
        print("on")
        led.on()
        waitUntilInLow(tasto, 50); # attendi finchè non c'è fronte di discesa
        print("off")
        acceso.start()
    elif acceso.get() > 5000:
        #non necessario il reset() perchè mai eseguito uno stop()
        led.off()
    time.sleep_ms(10)
```
Simulazione su Arduino con Wokwi: https://wokwi.com/projects/370308771487427585
### **Selezione luci**

```python
#Scrivere un programma che realizzi l'accensione di un led, due led o tre led tramite la pressione consecutiva di un pulsante 
#una, due o tre volte all'interno di un intervallo temporale di un secondo.
#Col la pressione prolungata dello stesso pulsante si spengono tutti e tre contemporaneamente.
import time
from machine import Pin

# attesa evento con tempo minimo di attesa
def waitUntilInLow(btn,t):
    while btn.value():
	 time.sleep_ms(t)

class DiffTimer(object):
    def __init__(self,elapsed):
        self.elapsed = elapsed
        self.timerState = False
        self.last = 0
    def __init__(self):
        self.elapsed = 0
        self.timerState = False
        self.last = 0
    def reset(self): # transizione di un pulsante
        self.elapsed = 0
        self.last = time.ticks_ms()
    def stop(self):
        self.timerState = False
        self.elapsed = self.elapsed + time.ticks_ms() - self.last
    def start(self):
        self.timerState = True
        self.last = time.ticks_ms()
    def get(self):
        if self.timerState:
            return time.ticks_ms() - self.last + self.elapsed
        return self.elapsed
    def set(self, e):
        reset()
        self.elapsed = e
	
tasto = Pin(12,Pin.IN)
led1 = Pin(13,Pin.OUT)
led2 = Pin(14,Pin.OUT)
led3 = Pin(18,Pin.OUT)
conteggio = DiffTimer()
spegnimento = DiffTimer()
count = 0
leds = [led1,led2,led3]

while True:
    if tasto.value() == 1:
        print("salita")
        spegnimento.start()
        waitUntilInLow(tasto, 50); # attendi finchè non c'è fronte di discesa
        print("discesa")
        spegnimento.stop()
        # parte alla prima pressione
        count = count + 1
        if count == 1:
            conteggio.start()
    elif conteggio.get() > 1000:
        conteggio.stop()
        conteggio.reset()
        if(count>0 and count < 4):
            leds[count-1].on()
            count=0
    elif spegnimento.get() > 3000:
        spegnimento.reset()
        count=0
        leds[0].off()
        leds[1].off()
        leds[2].off()
    time.sleep_ms(10)
```
Simulazione su Arduino con Wokwi: https://wokwi.com/projects/370317625079361537

### **Scegli chi ON/OFF un tasto**

```python
#Scrivere un programma che realizzi l'accensione del led1, oppure del led2 oppure del led3 led 
#tramite la pressione consecutiva di un pulsante una, due o tre volte all'interno di 
#un intervallo temporale di un secondo. Col la pressione per almeno un secondo, ma meno di tre, dello stesso pulsante si 
#accendono tutti i led, con la pressione dello stesso tasto per più di tre secondi si spengono tutti i led.
import time
from machine import Pin

# attesa evento con tempo minimo di attesa
def waitUntilInLow(btn,t):
    while btn.value():
	 time.sleep_ms(t)

class DiffTimer(object):
    def __init__(self,elapsed):
        self.elapsed = elapsed
        self.timerState = False
        self.last = 0
    def __init__(self):
        self.elapsed = 0
        self.timerState = False
        self.last = 0
    def reset(self): # transizione di un pulsante
        self.elapsed = 0
        self.last = time.ticks_ms()
    def stop(self):
        self.timerState = False
        self.elapsed = self.elapsed + time.ticks_ms() - self.last
    def start(self):
        self.timerState = True
        self.last = time.ticks_ms()
    def get(self):
        if self.timerState:
            return time.ticks_ms() - self.last + self.elapsed
        return self.elapsed
    def set(self, e):
        reset()
        self.elapsed = e
	
tasto = Pin(12,Pin.IN)
led1 = Pin(13,Pin.OUT)
led2 = Pin(14,Pin.OUT)
led3 = Pin(18,Pin.OUT)
conteggio = DiffTimer()
spegnimento = DiffTimer()
count = 0
leds = [led1,led2,led3]

while True:
    if tasto.value() == 1:
        print("salita")
        spegnimento.start()
        waitUntilInLow(tasto, 50); # attendi finchè non c'è fronte di discesa
        print("discesa")
        spegnimento.stop()
        # parte alla prima pressione
        count = count + 1
        if count == 1:
            conteggio.start()
    elif conteggio.get() > 1000:
        if(count>0 and count < 4):
            leds[count-1].on()
            count=0
    if spegnimento.get() > 1000 and spegnimento.get() < 3000:
        spegnimento.reset()
        count=0
        leds[0].on()
        leds[1].on()
        leds[2].on()
    elif spegnimento.get() > 3000:
        spegnimento.reset()
        count=0
        leds[0].off()
        leds[1].off()
        leds[2].off()
    time.sleep_ms(10)
```

Simulazione su Arduino con Wokwi: https://wokwi.com/projects/370321961849316353

### **Scegli chi ON/OFF due tasti**

In questo esempio vengono gestiti due input con i delay in un unico loop(). Questo comporta che possono essere utilizzati entrambi i pulsanti ma in momenti diversi essendo il loop() per 50 msec monopolizzato dal task dell'antirimbalzo di ciascun pulsante. Nel tempo di guardia dell'antirimbalzo nessun task in più può essere svolto in parallelo.  

```python
#Scrivere un programma che realizzi l'accensione del led1, oppure del led2 oppure del led3 led 
#tramite la pressione consecutiva di un pulsante una, due o tre volte all'interno di 
#un intervallo temporale di un secondo. Col la pressione di un altro pulsante si fa una cosa analoga per spegnerli.
from machine import Pin
import time

def waitUntilInLow(btn,t):
    while btn.value():
	 time.sleep_ms(t)

class DiffTimer(object):
    def __init__(self,elapsed):
        self.elapsed = elapsed
        self.timerState = False
        self.last = 0
    def __init__(self):
        self.elapsed = 0
        self.timerState = False
        self.last = 0
    def reset(self): # transizione di un pulsante
        self.elapsed = 0
        self.last = time.ticks_ms()
    def stop(self):
        self.timerState = False
        self.elapsed = self.elapsed + time.ticks_ms() - self.last
    def start(self):
        self.timerState = True
        self.last = time.ticks_ms()
    def get(self):
        if self.timerState:
            return time.ticks_ms() - self.last + self.elapsed
        return self.elapsed
    def set(self, e):
        reset()
        self.elapsed = e

tasto1 = Pin(12,Pin.IN)	
tasto2 = Pin(15,Pin.IN)
led1 = Pin(13,Pin.OUT)
led2 = Pin(14,Pin.OUT)
led3 = Pin(18,Pin.OUT)
conteggio1 = DiffTimer()
conteggio2 = DiffTimer()
count1 = 0
count2 = 0
leds = [led1,led2,led3]

while True:
    if tasto1.value() == 1:
        print("salita")
        waitUntilInLow(tasto1, 50); # attendi finchè non c'è fronte di discesa
        print("discesa")
        # parte alla prima pressione
        count1 = count1 + 1
        if count1 == 1:
            conteggio1.start()
    elif tasto2.value() == 1:
        print("salita2")
        waitUntilInLow(tasto2, 50); # attendi finchè non c'è fronte di discesa
        print("discesa2")
        # parte alla prima pressione
        count2 = count2 + 1
        if count2 == 1:
            conteggio2.start() 
    if conteggio1.get() > 1000:
        if(count1>0 and count1 < 4):
            print(count1)
            leds[count1-1].on()
            count1=0
    if conteggio2.get() > 1000:
        if(count2>0 and count2 < 4):
            print(count2)
            leds[count2-1].off()
            count2=0
    time.sleep_ms(10)
```
Simulazione su Arduino con Tinkercad: https://wokwi.com/projects/370335687978017793
>[Torna all'indice](timerbase.md) >[versione in C++](polledtimer_seq.md)
