>[Torna all'indice](timerbase.md) >[versione in C++](polledtimer_seq.md)

## **POLLED TIMERS SEQUENZIALI**

E’ possibile realizzare dei timers, con cui programmare **nel futuro** lo stesso evento o una catena di eventi diversi, eseguendo il **tempo di conteggio**. Il polling serve per verificare, ad ogni loop(), che il tempo trascorso (**elpased**) abbia raggiunto il **valore finale** (timeout) stabilito come obiettivo (target). Ciò si può ottenere utilizzando il **test** periodico di una **condizione** sulla funzione ```get()``` che, all'interno del ```loop()```, restituisce il **tempo di conteggio** (elapsed). Se la **condizione** è vera allora il **blocco** di istruzioni associato all'evento di timeout viene **eseguito**.

I **timer sequenziali** sono più **intuitivi** e **semplici** da usare rispetto ad i **timer ad eventi** perchè possono essere adoperati usando la stessa logica **sequenziale** e **lineare** che si usa in un normale algoritmo sincrono. Un'**algoritmo sincrono** pianifica le azioni in base alla **posizione** della istruzioni che le determinano nel codice e in base ai tempi stabiliti da eventuali **ritardi bloccanti** (l'esecuzione non va avanti) posti tra un'azione e l'altra o dal **polling di un timer** (test di avvenuto timeout).

Si tratta di un **pattern** (tipo di API) per la realizzazione di timers **molto comune** nella programmazione di **bracci robotici** per uso industriale (vedi bracci Universal Robots) che si adatta bene ad essere impiegato sia in un contesto in cui la logica dell'algoritmo è realizzata in maniera sequenziale sia in quello in cui gli input sono gestiti con un modello ad eventi.

**In** **generale**, possiamo individuare alcune **componenti del timer**:

- **reset** del timer. Accade al verificarsi di una certa **condizione**. Determina sia l'azzeramento del **tempo di conteggio** (```elapsed = 0```) che l'azzeramento della misura del ritardo dall'ultimo riavvio (```last = millis()```). L'azzeramento **non** è mai automatico ed è necessario impostarlo prima del riutilizzo del timer dopo uno o più comandi ``stop()`` (tipicamente al momento della valutazione della condizione di timeout) altrimenti è sufficiente richiamare la funzione ```start()```.  
- **start**. Avvia o riavvia il timer quando si avvera la **condizione di attivazione** posta in punto qualsiasi del loop. Imposta il flag di stato ```timerState```. 
- **stop** del timer. Accade al verificarsi di una certa **condizione** di sospensione del timer posta in punto qualsiasi del loop. In questa occasione il timer **campiona** il ritardo accumulato dall'ultimo riavvio e lo somma al **tempo di conteggio** cumulato all'ultima sospensione tramite ```elapsed += millis() - last```. In questo modo il tempo strascorso è memorizzato e può essere valutato in un momento successivo nel codice tramite la funzione ```get()```.
- **get** del tempo **tempo di conteggio** (elapsed) dal momento del **primo avvio** del timer. Viene fatto ad ogni ciclo di **loop** (meno se filtrato) e serve a realizzare il confronto con un tempo di timeout mediante un operatore relazionale (```var.get() > timeout```). Il valore restituito dipende dallo stato del timer:
     - se **```timerState == false```** allora restituisce l'ultimo **tempo di conteggio** misurato e memorizzato al momento della chiamata a stop() tramite ```elapsed += millis() - last```.
     - se **```timerState == true```** allora calcola il **tempo di conteggio** corrente campionando il ritardo attuale dall'ultimo riavvio e sommandolo ai ritardo cumulato prima dell'ultima sospensione (```millis() - last + elapsed```).
- **istruzioni triggerate** (scatenate) dal timer. Vengono eseguite in base al **tempo di conteggio misurato** dal timer. Vengono eseguite in maniera sequenziale in un punto stabilito del loop (istruzioni sincrone) in corrispondenza della **verità** di una certa **condizione** sul tempo di conteggio che coinvolge la funzione ```get()``` come operando. In corrispondenza si potrebbe azzerare il timer con ```reset()``` per prepararlo per il prossimo utilizzo.


```python
class DiffTimer(object):
    def __init__(self,elapsed):
        self.elapsed = elapsed
        self.timerState = False

    def __init__(self):
        self.elapsed = 0
        self.timerState = False

    def reset(self): # transizione di un pulsante
        self.elapsed = 0
        self.last = time.time()

    def stop(self):
        self.timerState = False
        self.elapsed = self.elapsed + time.time() - last

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

```C++
#Scrivere un programma che realizzi l'accensione di un led tramite un pulsante temporizzato che spegne il led 
#dopo un numero di ms impostati da setup. La logica del tasto deve essere senza stato e deve essere sensibile 
#al fronte di salita del segnale. Immaginare un collegamento pull down del tasto.
import time
from machine import Pin

# attesa evento con tempo minimo di attesa
def waitUntilInLow(btn,t):
    while btn.value():
	 time.sleep(t)

class DiffTimer(object):
    def __init__(self,elapsed):
        self.elapsed = elapsed
        self.timerState = False

    def __init__(self):
        self.elapsed = 0
        self.timerState = False

    def reset(self): # transizione di un pulsante
        self.elapsed = 0
        self.last = time.time()

    def stop(self):
        self.timerState = False
        self.elapsed = self.elapsed + time.time() - last

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
	
tasto = Pin(12,Pin.IN)
led = Pin(13,Pin.OUT)
acceso = DiffTimer()

while True:
    if tasto.value() == 1:
        print("on")
        led.on()
        waitUntilInLow(tasto, .005); # attendi finchè non c'è fronte di discesa
        print("off")
        acceso.start()
    elif acceso.get() > 5:
        #non necessario il reset() perchè mai eseguito uno stop()
        led.off()
    time.sleep(0.01)

```
Simulazione su Arduino con Wokwi: https://wokwi.com/projects/370308771487427585

### **Selezione luci**

```C++
/*
Scrivere un programma che realizzi l'accensione di un led, due led o tre led tramite la pressione consecutiva di un pulsante 
una, due o tre volte all'interno di un intervallo temporale di un secondo.
Col la pressione prolungata dello stesso pulsante si spengono tutti e tre contemporaneamente.
(Realizzazione del timer senza schedulatore)
*/

// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	    delay(t);
    }
}

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

DiffTimer conteggio, spegnimento;
int led1=13;
int led2=12;
int led3=11;
int tasto=2;
int count=0;

void setup(){
    pinMode(tasto,INPUT);
    pinMode(led1,OUTPUT);
    pinMode(led2,OUTPUT);
    pinMode(led3,OUTPUT);
    digitalWrite(led1,LOW);
    digitalWrite(led2,LOW);
    digitalWrite(led3,LOW);
    Serial.begin(115200);
}

void loop(){	
    //legge valore attuale dell'ingresso
    if(digitalRead(tasto)==HIGH){
	//fronte di salita
        spegnimento.start();
        waitUntilInputLow(tasto,50);			// attendi finchè non c'è fronte di discesa
        //fronte di discesa
        spegnimento.stop();
	//parte alla prima pressione
	count++;
	if(count == 1)
		conteggio.start();
    }else if(conteggio.get() > 1000){
        conteggio.stop();
	conteggio.reset();
	if(count>0){
		digitalWrite(14-count,HIGH);
		count=0;
	}
    }else if(spegnimento.get() > 3000){
        spegnimento.reset();
        digitalWrite(led1,LOW);
        digitalWrite(led2,LOW);
        digitalWrite(led3,LOW);
        count=0;
    }
    delay(10);
}
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/things/1EIcEp5BkZt-copy-of-selezione-luci-e-spegnimento-con-un-solo-tasto/editel?tenant=circuits

### **Scegli chi ON/OFF un tasto**

```C++
/*
 Scrivere un programma che realizzi l'accensione del led1, oppure del led2 oppure del led3 led 
 tramite la pressione consecutiva di un pulsante una, due o tre volte all'interno di 
 un intervallo temporale di un secondo. Col la pressione per almeno un secondo, ma meno di due, dello stesso pulsante si 
 accendono tutti i led, con la pressione dello stesso tasto per più di due secondi si spengono tutti i led.
*/
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

// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	    delay(t);
    }
}

DiffTimer conteggio, spegnimento;

int led1=13;
int led2=12;
int led3=11;
int tasto=2;
int count=0;
int in, out;

void setup(){
	pinMode(tasto,INPUT);
	pinMode(led1,OUTPUT);
	pinMode(led2,OUTPUT);
	pinMode(led3,OUTPUT);
	digitalWrite(led1,LOW);
	digitalWrite(led2,LOW);
	digitalWrite(led3,LOW);
}

void loop(){			
	//ad ogni pressione del tasto entro il tempo prefissato aggiorna il contatore di quel tasto (pressione)
	if(digitalRead(tasto)==HIGH){
		//fronte di salita
		spegnimento.start();
		waitUntilInputLow(tasto,50);			// attendi finchè non c'è fronte di discesa
		//fronte di discesa
		spegnimento.stop();
		//parte alla prima pressione
		count++;
		if(count == 1)
			conteggio.start();
	}else if(conteggio.get() > 1000){
                conteggio.stop();
		conteggio.reset();
		if(count>0){
			digitalWrite(14-count,HIGH);
			count=0;
		}
	}else if(spegnimento.get() > 1000 && spegnimento.get() < 2000){
		spegnimento.reset();
		digitalWrite(led1,HIGH);
		digitalWrite(led2,HIGH);
		digitalWrite(led3,HIGH);
		count=0;
	}else if(spegnimento.get() > 2000){
		spegnimento.reset();
		digitalWrite(led1,LOW);
		digitalWrite(led2,LOW);
		digitalWrite(led3,LOW);
		count=0;
	}
}
```

Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/things/92WnWXH0OvB-copy-of-scegli-chi-onoff-con-un-tasto/editel?tenant=circuits

### **Scegli chi ON/OFF due tasti**

In questo esempio vengono gestiti due input con i delay in un unico loop(). Questo comporta che possono essere utilizzati entrambi i pulsanti ma in momenti diversi essendo il loop() per 50 msec monopolizzato dal task dell'antirimbalzo di ciascun pulsante. Nel tempo di guardia dell'antirimbalzo nessun task in più può essere svolto in parallelo.  

```C++
/*
 Scrivere un programma che realizzi l'accensione del led1, oppure del led2 oppure del led3 led 
 tramite la pressione consecutiva di un pulsante una, due o tre volte all'interno di 
 un intervallo temporale di un secondo. Col la pressione di un altro pulsante si fa una cosa analoga per spegnerli.
*/
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

// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	    delay(t);
    }
}

DiffTimer conteggio1, conteggio2;
//fine variabili timer
int led1=13;
int led2=12;
int led3=11;
int tasto1=2;
int tasto2=3;
int count1=0;
int count2=0;

//azione da compiere allo scadere del timer	
void onElapse(){
	//se c'è un conteggio di accensione attivo accendi il led corrispondente al numero raggiunto
	if(count1>0){
		digitalWrite(14-count1,HIGH);
		count1=0;
	}
	//se c'è un conteggio di spegnimento attivo spegni il led corrispondente al numero raggiunto
	if(count2>0){
		digitalWrite(14-count2,LOW);
		count2=0;
	}
}

void setup(){
	pinMode(tasto1,INPUT);
	pinMode(tasto2,INPUT);
	pinMode(led1,OUTPUT);
	pinMode(led2,OUTPUT);
	pinMode(led3,OUTPUT);
	digitalWrite(led1,LOW);
	digitalWrite(led2,LOW);
	digitalWrite(led3,LOW);
	//startTimer(1000);
	count1=0;
	count2=0;
}

void loop(){
	if(digitalRead(tasto1)==HIGH){
		//fronte di salita
		waitUntilInputLow(tasto1,50);			// attendi finchè non c'è fronte di discesa
		//fronte di discesa
		//parte alla prima pressione
		count1++;
		if(count1 == 1)
			conteggio1.start();
	}else if(digitalRead(tasto2)==HIGH){
		//fronte di salita
		waitUntilInputLow(tasto2,50);			// attendi finchè non c'è fronte di discesa
		//fronte di discesa
		//parte alla prima pressione
		count2++;
		if(count2 == 1)
			conteggio2.start();
	}else if(conteggio1.get() > 1000){
        	conteggio1.stop();
		conteggio1.reset();
		if(count1>0){
			digitalWrite(14-count1,HIGH);
			count1=0;
		}
	}else if(conteggio2.get() > 1000){
        	conteggio2.stop();
		conteggio2.reset();
		if(count2>0){
			digitalWrite(14-count2,LOW);
			count2=0;
		}
	}
	delay(10);
}
```
Simulazione su Arduino con Tinkercad: [tenant=circuits](https://www.tinkercad.com/things/1eJwTOD7t8K-copy-of-scegli-chi-onoff/editel?tenant=circuits)https://www.tinkercad.com/things/1eJwTOD7t8K-copy-of-scegli-chi-onoff/editel?tenant=circuits

>[Torna all'indice](timerbase.md) >[versione in C++](polledtimer_seq.md)
