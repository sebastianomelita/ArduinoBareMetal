

>[Torna all'indice](indexpulsanti.md) >[versione in C++](nastro.md)

##  **NASTRO TRASPORTATORE**

###  **Gestione di un singolo nastro**

Programma per la gestione di un nastro trasportatore realizzato con un **timer** basato sul polling della funzione ```millis()``` e su **rilevatori di transito** toggle basati su ```delay()```.

IL timer **timer** è realizzato non ad eventi ma in **logica sequenziale** più intuitiva e più **in armonia** con la **logica sequenziale** delle **altre parti** dell'algoritmo risolutivo:

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

```C++
/*
Scrivere un programma che realizzi la gestione di un nastro traportatore attraverso la lettura di tre sensori 
di transito e il comando di un motore.
I sensori permangono al livello alto finchè un oggetto ingombra l'area del sensore, dopodichè vanno a livello basso. 
I sensori sono due all'inizio del nastro (uno per i pezzi bassi ed un'altro per quelli alti) ed uno alla fine del nastro 
che rileva il pezzo pronto per essere prelevato.
All'attivazione di un qualsiasi sensore di ingresso parte il motore e si resetta (blocca) il timer di volo.
All'attivazione del sensore di uscita si blocca il nastro, alla sua disattivazione riparte il nastro e parte il timer di volo.
Allo scadere del timer di volo si spegne il motore.
*/
#include <pthread.h> //libreria di tipo preemptive
#define DEBOUNCETIME 50
pthread_t t1;
pthread_t t2;
bool engineon;  // variabile globale che memorizza lo stato del motore
uint8_t safetystop;
bool isrun;
//gestione interrupt
volatile unsigned long previousMillis = 0;
volatile unsigned short numberOfButtonInterrupts = 0;
//fine gestione interrupt

typedef struct
{
	uint8_t id;
	uint8_t startSensorHigh;
	uint8_t startSensorLow;
	uint8_t stopSensor;
	uint8_t engineLed;
	uint8_t lowStartLed;
	uint8_t highStartLed;
	uint8_t stopLed;
	unsigned flyTime;
	bool engineon;
} Nastro;

typedef struct 
{
	unsigned long elapsed, last;
	bool timerState=false;
	void reset(){
		elapsed = 0;
		last = millis();
	}
	void stop(){
		timerState = false;
		elapsed += millis() - last;
	}
	void start(){
		timerState = true;
		last = millis();
	}
	unsigned long get(){
		if(timerState){
			return millis() - last + elapsed;
		}
		return elapsed;
	}
	void set(unsigned long e){
		reset();
		elapsed = e;
	}
} DiffTimer;

Nastro nastro1, nastro2;
DiffTimer fly[2];

void initNastri(){
	// porte nastro 1
	nastro1.id = 0;
	nastro1.startSensorHigh=14;
	nastro1.startSensorLow=12;
	nastro1.stopSensor=13;
	nastro1.engineLed=33;
	nastro1.lowStartLed=26;
	nastro1.highStartLed=25;
	nastro1.stopLed=27;
	nastro1.flyTime=4000;
	// porte nastro 2
	nastro2.id = 1;
	nastro2.startSensorHigh=4;
	nastro2.startSensorLow=2;
	nastro2.stopSensor=15;
	nastro2.engineLed=21;
	nastro2.lowStartLed=18;
	nastro2.highStartLed=19;
	nastro2.stopLed=5;
	nastro2.flyTime=8000;
	// set porte nastro 1
	pinMode(nastro1.engineLed, OUTPUT);
	pinMode(nastro1.lowStartLed, OUTPUT);
	pinMode(nastro1.highStartLed, OUTPUT);
	pinMode(nastro1.stopLed, OUTPUT);
	//pinMode(nastro1.motorCmd, OUTPUT);
	pinMode(nastro1.startSensorHigh, INPUT);
	pinMode(nastro1.startSensorLow, INPUT);
	pinMode(nastro1.stopSensor, INPUT); 
	// set porte nastro 2
	pinMode(nastro2.engineLed, OUTPUT);
	pinMode(nastro2.lowStartLed, OUTPUT);
	pinMode(nastro2.highStartLed, OUTPUT);
	pinMode(nastro2.stopLed, OUTPUT);
	//pinMode(nastro2.motorCmd, OUTPUT);
	pinMode(nastro2.startSensorHigh, INPUT);
	pinMode(nastro2.startSensorLow, INPUT);
	pinMode(nastro2.stopSensor, INPUT); 
}

// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	    delay(t);
    }
}

void * beltThread(void * d)
{
		Nastro *n;
    n = (Nastro *) d;
    while(true){    	
	String id = "Nastro "+String(n->id) + ": ";			
	if(digitalRead(n->startSensorLow)==HIGH){				// se è alto c'è stato un fronte di salita
		n->engineon = true && isrun; 	
		digitalWrite(n->engineLed, HIGH && isrun);
		digitalWrite(n->lowStartLed, HIGH);
		fly[n->id].stop();						// c'è almeno un pezzo in transito
		Serial.println(id+"Pezzo basso in ingresso");
		Serial.println(id+"Timer di volo disattivato");
		waitUntilInputLow(n->startSensorLow,50);			// attendi finchè non c'è fronte di discesa
		Serial.println(id+"Pezzo basso transitato in ingresso");
		digitalWrite(n->lowStartLed, LOW);
	}else if(digitalRead(n->startSensorHigh)==HIGH){				// se è alto c'è stato un fronte di salita
		n->engineon = true && isrun; 	
		digitalWrite(n->engineLed, HIGH && isrun);
		digitalWrite(n->highStartLed, HIGH);
		fly[n->id].stop();						 // c'è almeno un pezzo in transito
		Serial.println(id+"Pezzo alto in ingresso");
		Serial.println(id+"Timer di volo disattivato");
		waitUntilInputLow(n->startSensorHigh,50);			// attendi finchè non c'è fronte di discesa
		Serial.println(id+"Pezzo alto transitato in ingresso");
		digitalWrite(n->highStartLed, LOW);
	}else if(digitalRead(n->stopSensor)==HIGH) {
		n->engineon = false; 		
		digitalWrite(n->engineLed, LOW);
		digitalWrite(n->stopLed, HIGH);
		Serial.println(id+"Pezzo in uscita");
		waitUntilInputLow(n->stopSensor,50);
		Serial.println(id+"Pezzo prelevato dall'uscita");
		n->engineon = true && isrun; 
		digitalWrite(n->stopLed, LOW);
		digitalWrite(n->engineLed, HIGH && isrun);
		fly[n->id].start();
		Serial.println(id+"Timer di volo attivato");
	}else if(fly[n->id].get() > n->flyTime){
		String id = "Nastro "+String(n->id) + ": ";	 
		n->engineon = false; 
		digitalWrite(n->engineLed, LOW);
		Serial.println(id+"Timer di volo scaduto");
		fly[n->id].stop();
		fly[n->id].reset();
	}else if(!isrun){
		fly[n->id].stop();
		fly[n->id].reset();
	}
	delay(10);
    }
    return NULL;
}

void setup() {
  Serial.begin(115200);
  safetystop = 32;
  pinMode(safetystop, INPUT);
  attachInterrupt(digitalPinToInterrupt(safetystop), switchPressed, CHANGE );  
  engineon= false;
  isrun = true;
  initNastri();
  if (pthread_create(&t1, NULL, beltThread, (void *)&nastro1)) {
         Serial.println("Errore creazione btnThread");
  }
  if (pthread_create(&t2, NULL, beltThread, (void *)&nastro2)) {
         Serial.println("Errore creazione blinkThread");
  } 
}

// Interrupt Service Routine (ISR)
void switchPressed ()
{
  numberOfButtonInterrupts++; // contatore rimbalzi
  byte val = digitalRead(safetystop); // lettura stato pulsante
  previousMillis = millis(); // tempo evento
  if(val==HIGH){ // fronte di salita
		isrun = false; 				// impostazione dello stato dei nastri
		digitalWrite(nastro1.engineLed, LOW);	               
		digitalWrite(nastro2.engineLed, LOW);
  }
}  // end of switchPressed

void waitUntilInputLow2()
{
   // sezione critica
   // protegge previousMillis che, essendo a 16it, potrebbe essere danneggiata se interrotta da un interrupt
   // numberOfButtonInterrupts è 8bit e non è danneggiabile ne in lettura ne in scrittura
   noInterrupts();
   // il valore lastintTime potrà essere in seguito letto interrotto ma non danneggiato
   unsigned long lastintTime = previousMillis;
   interrupts();
   if ((numberOfButtonInterrupts != 0) //flag interrupt! Rimbalzo o valore sicuro? 
        && (millis() - lastintTime > DEBOUNCETIME )//se è passato il transitorio 
        && digitalRead(safetystop) == LOW)//se coincide con il valore di un polling
    { 
		Serial.print("HIT: "); Serial.print(numberOfButtonInterrupts);
		numberOfButtonInterrupts = 0; // reset del flag
	
		Serial.println(" in DISCESA");
		Serial.println(" Riattivo il nastro dopo blocco di sicurezza");
		isrun = true;
    }
}

void loop() {
	waitUntilInputLow2();
	delay(10); 							// equivale a yeld() (10 per le simulazioni 0 in HW)
}
```

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/349645533881565780

>[Torna all'indice](indexpulsanti.md) >[versione in C++](nastro.md)
