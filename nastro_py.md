

>[Torna all'indice](indexpulsanti.md) >[versione in C++](nastro.md)

##  **NASTRO TRASPORTATORE**

###  **Gestione di un singolo nastro**

Programma per la gestione di un nastro trasportatore realizzato con un **timer** basato sul polling della funzione ```millis()``` e su **rilevatori di transito** toggle basati su ```delay()```:

Scrivere un programma che realizzi la gestione di un nastro traportatore attraverso la lettura di tre sensori di transito (barriere ad infrarossi) e il comando di un motore. 

I sensori permangono al livello alto finchè un oggetto ingombra l'area del sensore, dopodichè vanno a livello basso. I sensori sono due all'inizio del nastro (uno per i pezzi bassi ed un'altro per quelli alti) ed uno alla fine del nastro che rileva il pezzo pronto per essere prelevato. 

Esiste un timer di volo, così chiamato perchè dura esattamente per il tempo che impiegherebbe il pezzo più lontano dall'uscita, cioè quello sulla barriera di ingresso, per arrivare sulla barriera di uscita. 

All'attivazione di un qualsiasi sensore di ingresso parte il motore e si resetta e pure si blocca il timer di volo. All'attivazione del sensore di uscita si blocca il nastro, alla sua disattivazione riparte il nastro e parte il timer di volo. Allo scadere del timer di volo si spegne il motore.


L'algoritmo proposto per la gestione di un nastro trasportatore fa uso: 
- della **primitiva** ```waitUntilInputLow()``` per la realizzazione della logica di barriera (pulsante toggle)
- delle **funzioni** ```stop()```, ```reset()``` e ```get()``` per la gestione di un timer
- della **variabile globale** ```ready``` per segnalare agli altri thread (o al loop principale) la presenza di un **pezzo pronto** per essere prelevato.

L'algoritmo gestisce **tre sensori** di barriera (rilevatori di transito):
- uno in barriera di ingresso per i **pezzi alti** appena inseriti sul nastro
- uno in barriera di ingresso per i **pezzi bassi** appena inseriti sul nastro
- uno in barriera di uscita per i **pezzi pronti** per essere prelevati

L'algoritmo si divide in due **fasi** in cui vengono svolti due compiti (**task**) differenti:
- la **lettura degli ingressi**, ovvero il controllo dello stato dei **sensori di transito**, sia quello dei **pezzi in ingresso** che quello dei **pezzi in uscita**
- il polling dello **stato del timer** per stabilire se non ci sono più pezzi sul nastro e quindi **spegnere il motore**.

```python
#Alla pressione del pulsante si attiva o disattiva il lampeggo di un led
import time
from machine import Pin

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
	if self.timerState:
	        self.timerState = False
	        self.elapsed = self.elapsed + time.ticks_ms() - self.last
    def start(self):
	if not self.timerState:
	        self.timerState = True
	        self.last = time.ticks_ms()
    def get(self):
        if self.timerState:
            return time.ticks_ms() - self.last + self.elapsed
        return self.elapsed
    def set(self, e):
        reset()
        self.elapsed = e

#attesa evento con tempo minimo di attesa
def waitUntilInLow(btn,t):
    while btn.value():
	    time.sleep_ms(t)

startSensorHigh = Pin(14,Pin.IN)
startSensorLow = Pin(12,Pin.IN)
stopSensor = Pin(13,Pin.IN)
engineLed = Pin(33,Pin.OUT)
lowStartLed = Pin(26,Pin.OUT)
highStartLed = Pin(25,Pin.OUT)
stopLed = Pin(27,Pin.OUT)
flyTime = 4000
volo = DiffTimer()
engineon = False

while True:
    if startSensorLow.value():
        engineon = True
        engineLed.on()
        lowStartLed.on()
        volo.stop()
        volo.reset()
        print("Pezzo basso in ingresso")
        print("Timer di volo disattivato")
        waitUntilInLow(startSensorLow,50)
        print("Pezzo basso transitato in ingresso")
        lowStartLed.off()
    elif startSensorHigh.value():
        engineon = True
        engineLed.on()
        highStartLed.on()
        volo.stop()
        volo.reset()
        print("Pezzo alto in ingresso")
        print("Timer di volo disattivato")
        waitUntilInLow(startSensorHigh,50)
        print("Pezzo alto transitato in ingresso")
        highStartLed.off()
    elif stopSensor.value():
	volo.stop()
        engineon = False
        engineLed.off()
        stopLed.on()
        print("Pezzo in uscita")
        waitUntilInLow(startSensorHigh,50)
        print("Pezzo prelevato dall'uscita")
        engineon = True
        stopLed.off()
        engineLed.on()
        volo.start()
	volo.reset()
        print("Timer di volo attivato")
    elif volo.get() > flyTime:
        volo.stop()
        volo.reset()
        engineon = False
        engineLed.off()
        print("Timer di volo scaduto")
```
Simulazione su ESP32 con Wokwi: https://wokwi.com/projects/370497523363061761

###  **Gestione di due nastri**

Programma per la gestione di **due** nastri trasportatori realizzato con un **timer HW** gestito dalla libreria ```Ticker``` e con **rilevatori di transito** toggle basati su istruzioni ```delay()```. Il **timer** di sistema lavora con segnali di **interrupt** che attivano **callback** invocate **in sequenza**, per cui al loro interno sarebbe opportuno perdere poco tempo evitando di usare istruzioni lente (**no delay()**). I **rilevatori di transito** riguardano due nastri e la loro definizione è **indipendente** per ciascuno di essi perchè è realizzata all'interno di due **thread** separati.

Nel ```loop()``` principale è gestito lo **switch** di un **pulsante generale** di sicurezza che **disabilità** la marcia dei motori di entrambi i nastri. La gestione è **non bloccante** e inibisce l'attivazione dei motori anche se i thread ancora non hanno completato il loro flusso di esecuzione arrivando fino all'ultima istruzione. Infatti, l'istruzione ```digitalWrite(n->engineLed, HIGH && isrun)``` accende il motore solo se la variabile globale ```isrun``` è asserita dallo switch nel **loop** principale del programma.

```python
#Alla pressione del pulsante si attiva o disattiva il lampeggo di un led
import time
from machine import Pin
import _thread as th

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
	if self.timerState:
	        self.timerState = False
	        self.elapsed = self.elapsed + time.ticks_ms() - self.last
    def start(self):
	if not self.timerState:
	        self.timerState = True
	        self.last = time.ticks_ms()
    def get(self):
        if self.timerState:
            return time.ticks_ms() - self.last + self.elapsed
        return self.elapsed
    def set(self, e):
        reset()
        self.elapsed = e

class Nastro(object):
    def __init__(self, id, startSensorHigh, startSensorLow, stopSensor, engineLed, lowStartLed, highStartLed, stopLed, flyTime):
        self.id = id
        self.startSensorHigh = startSensorHigh
        self.startSensorLow = startSensorLow
        self.stopSensor = stopSensor
        self.engineLed = engineLed
        self.lowStartLed = lowStartLed
        self.highStartLed = highStartLed
        self.stopLed = stopLed
        self.flyTime = flyTime
        self.volo = DiffTimer()
        self.engineon = False   
        self.isRun = False
    def waitUntilInLow(self,btn,t):
        while btn.value():
            time.sleep_ms(t)
    def block(self):
        self.isRun = False
        self.engineon = False
        self.engineLed.off()
        self.volo.stop()
        self.volo.reset()
        #print("Nastro", self.id, "Blocco di sicurezza")
    def unBlock(self):
        self.isRun = True
        #print("Nastro", self.id, "Sblocco")
    def checkInput(self):
        if self.isRun:
            if self.startSensorLow.value():
                self.engineon = self.isRun
                self.engineLed.value(self.isRun)
                self.lowStartLed.on()
                self.volo.stop()
                self.volo.reset()
                print("Nastro", self.id, "Pezzo fermo in ingresso")
                print("Nastro", self.id, "Timer di volo disattivato")
                self.waitUntilInLow(self.startSensorLow,50)
                print("Nastro", self.id, "Pezzo basso transitato in ingresso")
                self.lowStartLed.off()
            elif self.startSensorHigh.value():
                self.engineon = self.isRun
                self.engineLed.value(self.isRun)
                self.highStartLed.on()
                self.volo.stop()
                self.volo.reset()
                print("Nastro", self.id, "Pezzo alto in ingresso")
                print("Nastro", self.id, "Timer di volo disattivato")
                self.waitUntilInLow(self.startSensorHigh,50)
                print("Nastro", self.id, "Pezzo alto transitato in ingresso")
                self.highStartLed.off()
            elif self.stopSensor.value():
		self.volo.stop()
                self.engineon = False
                self.engineLed.off()
                self.stopLed.on()
                print("Nastro", self.id, "Pezzo in uscita")
                self.waitUntilInLow(self.startSensorHigh,50)
                print("Nastro", self.id, "Pezzo prelevato dall'uscita")
                self.engineon = self.isRun
                self.stopLed.off()
                self.engineLed.value(self.isRun)
                self.volo.start()
		volo.reset()
                print("Nastro", self.id, "Timer di volo attivato")
    def checkTimer(self):
        if self.isRun and self.volo.get() > self.flyTime:
            self.volo.stop()
            self.volo.reset()
            self.engineon = False
            self.engineLed.off()
            print("Nastro", self.id, "Timer di volo scaduto")

def beltThread(nstr):
    while True:   
        nstr.checkInput()   
        nstr.checkTimer()
        time.sleep_ms(10)

safetystop = Pin(32,Pin.IN)
# Nastro1
startSensorHigh1 = Pin(14,Pin.IN)
startSensorLow1 = Pin(12,Pin.IN)
stopSensor1 = Pin(13,Pin.IN)
engineLed1 = Pin(33,Pin.OUT)
lowStartLed1 = Pin(26,Pin.OUT)
highStartLed1 = Pin(25,Pin.OUT)
stopLed1 = Pin(27,Pin.OUT)
flyTime1 = 4000
# Nastro2
startSensorHigh2 = Pin(4,Pin.IN)
startSensorLow2 = Pin(2,Pin.IN)
stopSensor2 = Pin(15,Pin.IN)
engineLed2 = Pin(21,Pin.OUT)
lowStartLed2 = Pin(18,Pin.OUT)
highStartLed2 = Pin(19,Pin.OUT)
stopLed2 = Pin(5,Pin.OUT)
flyTime2 = 8000
nastro1 = Nastro(0,startSensorHigh1,startSensorLow1,stopSensor1,engineLed1,lowStartLed1,highStartLed1,stopLed1,flyTime1)
nastro2 = Nastro(1,startSensorHigh2,startSensorLow2,stopSensor2,engineLed2,lowStartLed2,highStartLed2,stopLed2,flyTime2)
th.start_new_thread(beltThread, (nastro1,))
th.start_new_thread(beltThread, (nastro2,))

while True:
    if safetystop.value():
        nastro1.block()
        nastro2.block()
    else:
        nastro1.unBlock()
        nastro2.unBlock()
    
    time.sleep_ms(10)		
								
```
Simulazione su Esp32 con Wowki: https://wokwi.com/projects/370504281602332673

###  **Gestione di due nastri e tasto emergenza con interrupt**

E' normalmente la soluzione più affidabile per la realizzazione di un pulsante di emergenza dato che il **disarmo** del sistema avviene in un flusso di esecuzione **diretto** ed **indipendente** (parallelo) al flusso di esecuzione principale del programma.

Il **riarmo** del pulsante di arresto, essendo meno problematico ai fini della sicurezza, invece avviene tramite una **funzione** nel **loop principale** che esegue il **debouncing SW** del tasto e la **selezione** del **fronte di discesa** dello stesso. 

```python
#Alla pressione del pulsante si attiva o disattiva il lampeggo di un led
import time
import machine
from machine import Pin
import _thread as th

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
	if self.timerState:
	        self.timerState = False
	        self.elapsed = self.elapsed + time.ticks_ms() - self.last
    def start(self):
	if not self.timerState:
	        self.timerState = True
	        self.last = time.ticks_ms()
    def get(self):
        if self.timerState:
            return time.ticks_ms() - self.last + self.elapsed
        return self.elapsed
    def set(self, e):
        reset()
        self.elapsed = e

class Nastro(object):
    def __init__(self, id, startSensorHigh, startSensorLow, stopSensor, engineLed, lowStartLed, highStartLed, stopLed, flyTime):
        self.id = id
        self.startSensorHigh = startSensorHigh
        self.startSensorLow = startSensorLow
        self.stopSensor = stopSensor
        self.engineLed = engineLed
        self.lowStartLed = lowStartLed
        self.highStartLed = highStartLed
        self.stopLed = stopLed
        self.flyTime = flyTime
        self.volo = DiffTimer()
        self.engineon = False   
        self.isRun = False
    def waitUntilInLow(self,btn,t):
        while btn.value():
            time.sleep_ms(t)
    def block(self):
        self.isRun = False
        self.engineon = False
        self.engineLed.off()
        self.volo.stop()
        self.volo.reset()
        #print("Nastro", self.id, "Blocco di sicurezza")
    def unBlock(self):
        self.isRun = True
        #print("Nastro", self.id, "Sblocco")
    def checkInput(self):
        if self.isRun:
            if self.startSensorLow.value():
                self.engineon = self.isRun
                self.engineLed.value(self.isRun)
                self.lowStartLed.on()
                self.volo.stop()
                self.volo.reset()
                print("Nastro", self.id, "Pezzo fermo in ingresso")
                print("Nastro", self.id, "Timer di volo disattivato")
                self.waitUntilInLow(self.startSensorLow,50)
                print("Nastro", self.id, "Pezzo basso transitato in ingresso")
                self.lowStartLed.off()
            elif self.startSensorHigh.value():
                self.engineon = self.isRun
                self.engineLed.value(self.isRun)
                self.highStartLed.on()
                self.volo.stop()
                self.volo.reset()
                print("Nastro", self.id, "Pezzo alto in ingresso")
                print("Nastro", self.id, "Timer di volo disattivato")
                self.waitUntilInLow(self.startSensorHigh,50)
                print("Nastro", self.id, "Pezzo alto transitato in ingresso")
                self.highStartLed.off()
            elif self.stopSensor.value():
		self.volo.stop()
                self.engineon = False
                self.engineLed.off()
                self.stopLed.on()
                print("Nastro", self.id, "Pezzo in uscita")
                self.waitUntilInLow(self.startSensorHigh,50)
                print("Nastro", self.id, "Pezzo prelevato dall'uscita")
                self.engineon = self.isRun
                self.stopLed.off()
                self.engineLed.value(self.isRun)
                self.volo.start()
		self.volo.reset()
                print("Nastro", self.id, "Timer di volo attivato")
    def checkTimer(self):
        if self.isRun and self.volo.get() > self.flyTime:
            self.volo.stop()
            self.volo.reset()
            self.engineon = False
            self.engineLed.off()
            print("Nastro", self.id, "Timer di volo scaduto")

def beltThread(nstr):
    while True:   
        nstr.checkInput()   
        nstr.checkTimer()
        time.sleep_ms(10)

# Interrupt Service Routine (ISR)
def safety_pressed(pin):
    global previousMillis
    global numberOfButtonInterrupts
    global interrupt_pin
    if numberOfButtonInterrupts == 0:
    # intervento immediato sul fronte di salita
        nastro1.block()
        nastro2.block()
    interrupt_pin = pin 
    numberOfButtonInterrupts += 1      # contatore rimbalzi
    previousMillis = time.ticks_ms()   # tempo evento

def waitUntilInputChange():
    global previousMillis
    global numberOfButtonInterrupts
    # sezione critica
    # protegge previousMillis che, essendo a 16it, potrebbe essere danneggiata se interrotta da un interrupt
    # numberOfButtonInterrupts è 8 bit e non è danneggiabile ne in lettura ne in scrittura
    irq_state = machine.disable_irq() # Start of critical section
    # il valore lastintTime potrà essere in seguito letto interrotto ma non danneggiato
    lastintTime = previousMillis
    machine.enable_irq(irq_state) # End of critical section

    c = (numberOfButtonInterrupts != 0 and time.ticks_ms() - lastintTime > debtime and safetystop.value() == 0)
    if c:
        print("HIT: ")
        print(numberOfButtonInterrupts)
        numberOfButtonInterrupts = 0   # reset del flag (riarmo differito sul fronte di discesa)
        print(" in DISCESA debounced")
        nastro1.unBlock()
        nastro2.unBlock()


safetystop = Pin(32,Pin.IN)
# Nastro1
startSensorHigh1 = Pin(14,Pin.IN)
startSensorLow1 = Pin(12,Pin.IN)
stopSensor1 = Pin(13,Pin.IN)
engineLed1 = Pin(33,Pin.OUT)
lowStartLed1 = Pin(26,Pin.OUT)
highStartLed1 = Pin(25,Pin.OUT)
stopLed1 = Pin(27,Pin.OUT)
flyTime1 = 4000
# Nastro2
startSensorHigh2 = Pin(4,Pin.IN)
startSensorLow2 = Pin(2,Pin.IN)
stopSensor2 = Pin(15,Pin.IN)
engineLed2 = Pin(21,Pin.OUT)
lowStartLed2 = Pin(18,Pin.OUT)
highStartLed2 = Pin(19,Pin.OUT)
stopLed2 = Pin(5,Pin.OUT)
flyTime2 = 8000
nastro1 = Nastro(0,startSensorHigh1,startSensorLow1,stopSensor1,engineLed1,lowStartLed1,highStartLed1,stopLed1,flyTime1)
nastro2 = Nastro(1,startSensorHigh2,startSensorLow2,stopSensor2,engineLed2,lowStartLed2,highStartLed2,stopLed2,flyTime2)
th.start_new_thread(beltThread, (nastro1,))
th.start_new_thread(beltThread, (nastro2,))
safetystop.irq(handler=safety_pressed, trigger=Pin.IRQ_RISING)
previousMillis = 0
numberOfButtonInterrupts = 0
debtime = 50 
interrupt_pin = 0
nastro1.unBlock()
nastro2.unBlock()

while True:
    waitUntilInputChange()   
    time.sleep_ms(10)														
```

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/370533168086483969

>[Torna all'indice](indexpulsanti.md) >[versione in C++](nastro.md)
